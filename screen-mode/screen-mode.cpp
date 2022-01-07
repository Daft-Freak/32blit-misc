#include "screen-mode.hpp"

using namespace blit;

static std::tuple<ScreenMode, PixelFormat> screen_modes[]{
  {ScreenMode::lores, PixelFormat::RGB},
  {ScreenMode::lores, PixelFormat::RGB565},
  {ScreenMode::lores, PixelFormat::P},
  {ScreenMode::hires, PixelFormat::RGB},
  {ScreenMode::hires, PixelFormat::RGB565},
  {ScreenMode::hires, PixelFormat::P},
};

static const char *labels[]{
  "lores RGB8",
  "lores RGB565",
  "lores P",
  "hires RGB8",
  "hires RGB565",
  "hires P"
};

int current_mode = 0;
const int num_screen_modes = std::size(screen_modes);
bool auto_mode = true;

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
  screen.text(labels[current_mode], minimal_font, {10, 20});

  // in non-palette modes, this will blend the text a little darker
  screen.pen = Pen(2);
  screen.text(labels[current_mode], minimal_font, {10, 20});

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
      current_mode = current_mode == 0 ? num_screen_modes - 1 : current_mode - 1;

      auto &mode = screen_modes[current_mode];
      if(set_screen_mode(std::get<0>(mode), std::get<1>(mode)))
        break;
    }
  };

  auto next_mode = [](){
    while(true) {
      current_mode = (current_mode + 1) % num_screen_modes;

      auto &mode = screen_modes[current_mode];
      if(set_screen_mode(std::get<0>(mode), std::get<1>(mode)))
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
