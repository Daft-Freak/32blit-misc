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

// game/space list
struct StorageEntry {
  uint16_t start_block;
  RawMetadata *metadata; // nullptr == empty
};

std::list<StorageEntry> storage_usage;

static uint16_t calc_num_blocks(uint32_t size) {
  return (size - 1) / storage_block_size + 1;
}

void init() {
  blit::set_screen_mode(blit::ScreenMode::hires);

  uint16_t last_end = 0;

  blit::api.list_installed_games([&last_end](const uint8_t *ptr, uint32_t block, uint32_t size) {

    // go back to find the metadata
    uint32_t header_size = ((BlitGameHeader *)ptr)->end & 0x1FFFFFF;
    auto meta_ptr = ptr + header_size;

    RawMetadata *metadata = nullptr;

    if(memcmp(meta_ptr, "BLITMETA", 8) == 0) {
      metadata = (RawMetadata *)(meta_ptr + 10);
    }
    // FIXME: else set placeholder

    uint16_t block16 = block;
    uint16_t end_block = block16 + calc_num_blocks(size);

    StorageEntry new_entry = {block16, metadata};

    if(last_end != block) {
      // insert empty space
      StorageEntry empty_entry = {last_end, nullptr};
      storage_usage.emplace_back(empty_entry);
    }

    storage_usage.emplace_back(new_entry);

    last_end = end_block;
  });

  if(last_end != storage_size_blocks) {
    // insert empty space
      StorageEntry empty_entry = {last_end, nullptr};
      storage_usage.emplace_back(empty_entry);
  }
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

  for(int y = 0; y < num_rows; y++) {
    for(int x = 0; x < num_cols; x++) {

      unsigned block_index = y * num_cols + x;

      // advance to next entry
      if(cur_entry != storage_usage.end() && std::next(cur_entry)->start_block == block_index)
        cur_entry++;

      Pen col = cur_entry->metadata ? Pen{255, 255, 255} : Pen{100, 100, 100};

      screen.pen = col;

      screen.rectangle({
        x_off + x * size_with_border, y_off + y * size_with_border,
        block_tile_size, block_tile_size
      });
    }
  }
}

void update(uint32_t time_ms) {

}
