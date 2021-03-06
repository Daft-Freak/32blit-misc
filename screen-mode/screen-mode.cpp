#include "screen-mode.hpp"

using namespace blit;

ScreenMode screen_modes[]{
  ScreenMode::lores,
  ScreenMode::hires,
  ScreenMode::hires_palette
};
int current_mode = 0;
const int num_screen_modes = 3;

/* setup */
void init() {
  //set_screen_mode(ScreenMode::hires_palette);
  Pen palette[]{
    {  0,   0,   0},
    { 20,  30,  40},
    {255, 255, 255},
    {  0,   0, 255}
  };
  set_screen_palette(palette, 4);
}

void render(uint32_t time_ms) {
  screen.pen = Pen(20, 30, 40);
  screen.clear();

  screen.alpha = 255;
  screen.pen = Pen(255, 255, 255);
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
  screen.pen = Pen(3);

  screen.line(Point(  0,   0), Point(319,   0));
  screen.line(Point(319,   0), Point(319, 239));
  screen.line(Point(319, 239), Point(  0, 239));
  screen.line(Point(  0, 239), Point(  0,   0));
}

int mode_switch_counter = 0;
void update(uint32_t time) {

  if(++mode_switch_counter == 50) {
    current_mode = (current_mode + 1) % num_screen_modes;
    set_screen_mode(screen_modes[current_mode]);

    mode_switch_counter = 0;
  }
}