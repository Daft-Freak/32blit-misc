#include "screen-mode.hpp"

using namespace blit;

static std::tuple<ScreenMode, Size> screen_modes[]{
  {ScreenMode::lores, {}},
  {ScreenMode::hires, {}},

  // gba modes
  {ScreenMode::lores, {240, 160}},
  {ScreenMode::hires, {240, 160}},

  // picovision modes
  {ScreenMode::lores, {640, 480}},
  {ScreenMode::hires, {640, 480}},
  {ScreenMode::lores, {720, 480}},
  {ScreenMode::hires, {720, 480}},
  {ScreenMode::lores, {720, 400}},
  {ScreenMode::hires, {720, 400}},
  {ScreenMode::lores, {720, 576}},
  {ScreenMode::hires, {720, 576}},

  // picovision "wide" modes
  {ScreenMode::lores, {800, 600}},
  {ScreenMode::hires, {800, 600}},
  {ScreenMode::lores, {800, 480}},
  {ScreenMode::hires, {800, 480}},
  {ScreenMode::lores, {800, 450}},
  {ScreenMode::hires, {800, 450}},
  {ScreenMode::lores, {960, 540}},
  {ScreenMode::hires, {960, 540}},
  {ScreenMode::lores, {1280, 720}},
  {ScreenMode::hires, {1280, 720}},

  // silly modes
  {ScreenMode::hires, {212, 160}}, // ~VGA / 3
  {ScreenMode::hires, {128,  96}}, // VGA / 5
  {ScreenMode::hires, {320, 120}}, // w / 2, h / 4
  {ScreenMode::hires, {160, 240}}, // w / 4, h / 2
  {ScreenMode::hires, {160, 480}}, // w / 4, h / 1
  {ScreenMode::hires, {640, 120}}, // w / 1, h / 4
  // silly probably single-buffered modes
  {ScreenMode::hires, {640, 240}},
  {ScreenMode::hires, {320, 480}},
};


static PixelFormat screen_formats[]{
  PixelFormat::RGB,
  PixelFormat::RGB565,
  PixelFormat::BGR555, // picovision
  PixelFormat::P,
};

static const char *mode_labels[]{
  "lores",
  "hires",

  "240x160 lores",
  "240x160 hires",

  "640x480 lores",
  "640x480 hires",
  "720x480 lores",
  "720x480 hires",
  "720x400 lores",
  "720x400 hires",
  "720x576 lores",
  "720x576 hires",

  "800x600 lores",
  "800x600 hires",
  "800x480 lores",
  "800x480 hires",
  "800x450 lores",
  "800x450 hires",
  "960x540 lores",
  "960x540 hires",
  "1280x720 lores",
  "1280x720 hires",

  // silly
  "212x160",
  "128x96",
  "320x120",
  "160x240",
  "160x480",
  "640x120",
  "640x240",
  "320x480",
};

static const char *format_labels[]{
  "RGB8",
  "RGB565",
  "BGR555",
  "P"
};

static int current_mode = 0, current_format = 0;
const int num_screen_modes = std::size(screen_modes);
const int num_screen_formats = std::size(screen_formats);
bool auto_mode = false;//true;

/* setup */
void init() {

  Pen palette[]{
    {  0,   0,   0},
    { 20,  30,  40},
    {255, 255, 255},
    {255,   0,   0},
    {  0, 255,   0},
    {  0,   0, 255}
  };
  set_screen_palette(palette, 6);
}

void render(uint32_t time_ms) {
  screen.pen = Pen(20, 30, 40);
  screen.clear();

  screen.pen = Pen(255, 255, 255);
  screen.rectangle(Rect(0, 0, 320, 14));

  screen.pen = Pen(2);
  screen.rectangle(Rect(0, 0, 320, 14));

  screen.pen = Pen(0, 0, 0);
  screen.text("Screen Mode", minimal_font, Point(5, 4));

  // lores size
  screen.pen = Pen(0xFF, 0, 0);

  screen.line(Point(  0,   0), Point(159,   0));
  screen.line(Point(159,   0), Point(159, 119));
  screen.line(Point(159, 119), Point(  0, 119));
  screen.line(Point(  0, 119), Point(  0,   0));

  // hires size
  screen.pen = Pen(0, 0xFF, 0);

  screen.line(Point(  0,   0), Point(319,   0));
  screen.line(Point(319,   0), Point(319, 239));
  screen.line(Point(319, 239), Point(  0, 239));
  screen.line(Point(  0, 239), Point(  0,   0));

  // paletted
  screen.pen = Pen(5);

  screen.line(Point(  0,   0), Point(319,   0));
  screen.line(Point(319,   0), Point(319, 239));
  screen.line(Point(319, 239), Point(  0, 239));
  screen.line(Point(  0, 239), Point(  0,   0));

  // current mode label
  screen.pen = Pen(0xFF, 0xFF, 0xFF);
  screen.text(mode_labels[current_mode], minimal_font, {10, 20});
  screen.text(format_labels[current_format], minimal_font, {10, 30});

  // in non-palette modes, this will blend the text a little darker
  screen.pen = Pen(2);
  screen.text(mode_labels[current_mode], minimal_font, {10, 20});
  screen.text(format_labels[current_format], minimal_font, {10, 30});

  // show some colour blocks
  // red
  screen.pen = Pen(255, 0, 0);
  screen.rectangle({10, 40, 20, 20});

  screen.pen = Pen(3);
  screen.rectangle({10, 40, 20, 20});

  // green
  screen.pen = Pen(0, 255, 0);
  screen.rectangle({30, 40, 20, 20});

  screen.pen = Pen(4);
  screen.rectangle({30, 40, 20, 20});

  // blue
  screen.pen = Pen(0, 0, 255);
  screen.rectangle({50, 40, 20, 20});

  screen.pen = Pen(5);
  screen.rectangle({50, 40, 20, 20});
}

int mode_switch_counter = 0;
void update(uint32_t time) {

  auto prev_mode = [](){

    while(true) {
      // decrement format
      current_format--;

      // first format, decrment mode
      if(current_format < 0) {
        current_format = num_screen_formats - 1;
        current_mode = current_mode == 0 ? num_screen_modes - 1 : current_mode - 1;
      }

      auto &mode = screen_modes[current_mode];
      auto format = screen_formats[current_format];
      if(set_screen_mode(std::get<0>(mode), format, std::get<1>(mode)))
        break;
    }
  };

  auto next_mode = [](){
    while(true) {
      // increment format
      current_format++;

      // last format, increment mode
      if(current_format == num_screen_formats) {
        current_format = 0;
        current_mode = (current_mode + 1) % num_screen_modes;
      }

      auto &mode = screen_modes[current_mode];
      auto format = screen_formats[current_format];
      if(set_screen_mode(std::get<0>(mode), format, std::get<1>(mode)))
        break;
    }
  };

  if(auto_mode && ++mode_switch_counter == 50) {
    next_mode();
    mode_switch_counter = 0;
  }

  // manual mode change
  if(buttons.released & Button::A) {
    auto_mode = false;
    next_mode();
  } else if(buttons.released & Button::B) {
    auto_mode = false;
    prev_mode();
  } else if(buttons.released & Button::X) {
    auto_mode = true; // re-enable auto mode
  }
}
