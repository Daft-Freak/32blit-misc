#include <cstring>
#include <list>

#include "32blit.hpp"

#include "engine/api_private.hpp"

// hardcoded consts...
// 32MB flash - 4MB reserved
static const uint32_t storage_size = (32 - 4) * 1024 * 1024;
static const uint32_t storage_block_size = 64 * 1024;
static const uint32_t storage_size_blocks = storage_size / storage_block_size;

static const int num_cols = 32;
static const int num_rows = storage_size_blocks / num_cols;
static const int block_tile_size = 8;
static const int block_tile_border = 1;

// from launcher-shared
struct BlitGameHeader {
  uint32_t magic;

  uint32_t render;
  uint32_t tick;
  uint32_t init;

  uint32_t end;

  uint8_t device_id;
  uint8_t unused[3];

  // if device_id != 0
  uint16_t api_version_major;
  uint16_t api_version_minor;
};

struct RawMetadata {
  uint32_t crc32;
  char datetime[16];
  char title[25];
  char description[129];
  char version[17];
  char author[17];
};

struct RawTypeMetadata {
  char category[17];
  char url[129];
  uint8_t num_filetypes;
  char filetypes[][5];
};

// game/space list
struct StorageEntry {
  uint16_t start_block;
  const RawMetadata *metadata; // nullptr == empty
  blit::Surface *icon;
};

static std::list<StorageEntry> storage_usage;
static std::list<StorageEntry>::iterator selected_entry;

static const RawMetadata placeholder_meta {
  0,
  "UNKNOWN",
  "UNKNOWN",
  "",
  "v??",
  "UNKNOWN"
};

static uint16_t calc_num_blocks(uint32_t size) {
  return (size - 1) / storage_block_size + 1;
}

static blit::Pen get_entry_colour(StorageEntry &entry) {
  if(!entry.metadata)
    return {100, 100, 100}; // empty

  if(!entry.icon)
    return {255, 255, 255}; // no metadata/icon

  // average the icon pixels (it's only 8x8)
  float r = 0.0f, g = 0.0f, b = 0.0f;
  int c = 0;

  for(int y = 0; y < entry.icon->bounds.h; y++) {
    for(int x = 0; x < entry.icon->bounds.w; x++) {
      auto tmp = entry.icon->get_pixel({x, y});
      if(tmp.a) {
        r += tmp.r;
        g += tmp.g;
        b += tmp.b;
        c++;
      }
    }
  }

  return {uint8_t(r / c), uint8_t(g / c), uint8_t(b / c)};
}

void init() {
  blit::set_screen_mode(blit::ScreenMode::hires);

  uint16_t last_end = 0;

  blit::api.list_installed_games([&last_end](const uint8_t *ptr, uint32_t block, uint32_t size) {

    // go back to find the metadata
    uint32_t header_size = ((BlitGameHeader *)ptr)->end & 0x1FFFFFF;
    auto meta_ptr = ptr + header_size;

    const RawMetadata *metadata = nullptr;
    const blit::packed_image *icon_data = nullptr;

    if(memcmp(meta_ptr, "BLITMETA", 8) == 0) {
      auto meta_size = *(uint16_t *)(meta_ptr + 8);
      meta_ptr += 10;
      metadata = (const RawMetadata *)meta_ptr;

      uint16_t offset = sizeof(RawMetadata);

      // skip type info
      if(offset != meta_size && memcmp(meta_ptr + offset, "BLITTYPE", 8) == 0) {
        auto type_meta = (const RawTypeMetadata *)(meta_ptr + offset + 8);
        offset += sizeof(RawTypeMetadata) + 8 + type_meta->num_filetypes * 5;
      }

      // next is the icon (then the splash)
      if(offset != meta_size)
        icon_data = (const blit:: packed_image *)(meta_ptr + offset);
    }
    else
      metadata = &placeholder_meta; // to distinguish from empty space

    // setup new entry
    uint16_t block16 = block;
    uint16_t end_block = block16 + calc_num_blocks(size);

    auto icon = icon_data ? blit::Surface::load(icon_data) : nullptr;

    StorageEntry new_entry = {block16, metadata, icon};

    if(last_end != block) {
      // insert empty space
      StorageEntry empty_entry = {last_end, nullptr, nullptr};
      storage_usage.emplace_back(empty_entry);
    }

    storage_usage.emplace_back(new_entry);

    last_end = end_block;
  });

  if(last_end != storage_size_blocks) {
    // insert empty space
      StorageEntry empty_entry = {last_end, nullptr, nullptr};
      storage_usage.emplace_back(empty_entry);
  }

  selected_entry = storage_usage.begin();
}

void render(uint32_t time_ms) {
  using namespace blit;

  screen.pen = {20, 30, 40};
  screen.clear();

  // draw grid
  auto size_with_border = block_tile_size + block_tile_border;

  int y_off = 10;
  int x_off = (screen.bounds.w - (num_cols * size_with_border)) / 2;

  auto cur_entry = storage_usage.begin();
  auto next_entry = std::next(cur_entry);
  auto col = get_entry_colour(*cur_entry);

  // tile cur item started in
  int start_x = 0, start_y = 0;

  for(int y = 0; y < num_rows; y++) {
    for(int x = 0; x < num_cols; x++) {

      unsigned block_index = y * num_cols + x;

      // advance to next entry
      if(next_entry != storage_usage.end() && next_entry->start_block == block_index) {
        cur_entry = next_entry;
        next_entry = std::next(cur_entry);
        col = get_entry_colour(*cur_entry);

        start_x = x;
        start_y = y;
      }

      screen.pen = col;

      Rect r{
        x_off + x * size_with_border, y_off + y * size_with_border,
        block_tile_size, block_tile_size
      };

      // join tiles if not first
      if(x && (x != start_x || y != start_y)) {
        r.x -= block_tile_border;
        r.w += block_tile_border;
      }

      // join up if on third+ row or second row after the start
      if(y != start_y && (y - start_y > 1 || x >= start_x)) {
        r.y -= block_tile_border;
        r.h += block_tile_border;
      }

      screen.rectangle(r);

      if(cur_entry->icon && x == start_x && y == start_y) {
        // draw icon
        screen.blit(cur_entry->icon, {0, 0, 8, 8}, {r.x, r.y});
      }

      // draw border around selected
      if(cur_entry == selected_entry) {
        screen.pen = {255, 255, 255};

        // left
        if(x == 0 || (x == start_x && y == start_y))
          screen.v_span({r.x - 1, r.y}, size_with_border);

        // right
        if(x == num_cols - 1 || (next_entry != storage_usage.end() && next_entry->start_block == block_index + 1))
          screen.v_span({r.x + r.w, r.y}, size_with_border);

        // top
        if(y == start_y || (y == start_y + 1 && x < start_x))
          screen.h_span({r.x - 1, r.y - 1}, r.w + 2);

        // bottom
        int end_x = num_cols - 1, end_y = num_rows - 1;
        if(next_entry != storage_usage.end()) {
          end_x = next_entry->start_block % num_cols;
          end_y = next_entry->start_block / num_cols;
        }

        if(y == end_y || (y == end_y - 1 && x >= end_x))
          screen.h_span({r.x - 1, r.y +r.h}, r.w + 2);
      }
    }
  }

  // show game info
  screen.pen = {255, 255, 255};

  int meta_y = y_off + num_rows * size_with_border + 10;

  if(selected_entry->metadata) {
    screen.text(selected_entry->metadata->title, minimal_font, {x_off, meta_y});
    screen.text(selected_entry->metadata->author, minimal_font, {x_off, meta_y + 10});
    screen.text(selected_entry->metadata->version, minimal_font, {x_off, meta_y + 20});
  } else {
    screen.text("Empty", minimal_font, {x_off, meta_y});
  }
}

void update(uint32_t time_ms) {

  if(blit::buttons.released & blit::Button::DPAD_LEFT) {
    if(selected_entry == storage_usage.begin())
      selected_entry = std::prev(storage_usage.end());
    else
      selected_entry--;
  }

  if(blit::buttons.released & blit::Button::DPAD_RIGHT) {
    selected_entry++;
    if(selected_entry == storage_usage.end())
      selected_entry = storage_usage.begin();
  }
}
