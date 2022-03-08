#include "timing-test.hpp"

using namespace blit;

static uint32_t init_time;
static uint32_t init_time_us;
static uint32_t num_updates = 0, num_renders = 0;
static uint32_t num_timer100 = 0, num_timer10 = 0, num_timer1 = 0;
static uint32_t fake_last_update, real_last_update;

static uint32_t display_current_time;
static uint32_t display_current_time_us;
static uint32_t display_num_updates, display_num_renders;
static uint32_t display_num_timer100, display_num_timer10, display_num_timer1;
static uint32_t display_fake_last_update, display_real_last_update;

static bool paused = false, slow_render = false;

static Timer timer_100ms, timer_10ms, timer_1ms;


static void timer_100_update(blit::Timer &t){
  num_timer100++;
}

static void timer_10_update(blit::Timer &t){
  num_timer10++;
}

static void timer_1_update(blit::Timer &t){
  num_timer1++;
}

void init() {
  set_screen_mode(ScreenMode::hires);

  init_time = now();
  init_time_us = now_us();

  timer_100ms.init(timer_100_update, 100, -1);
  timer_100ms.start();

  timer_10ms.init(timer_10_update, 10, -1);
  timer_10ms.start();

  timer_1ms.init(timer_1_update, 1, -1);
  timer_1ms.start();
}

void render(uint32_t time_ms) {
  num_renders++;
  auto current_time = now();
  auto current_time_us = now_us();

  if(!paused) {
    display_current_time = current_time;
    display_current_time_us = current_time_us;
    display_num_updates = num_updates;
    display_num_renders = num_renders;
    display_num_timer100 = num_timer100;
    display_num_timer10 = num_timer10;
    display_num_timer1 = num_timer1;
    display_fake_last_update = fake_last_update;
    display_real_last_update = real_last_update;
  }

  screen.pen = Pen(20, 30, 40);
  screen.clear();

  screen.pen = Pen(255, 255, 255);

  screen.text("A: slow update B: slow render X: Pause", minimal_font, {4, 4}, false);

  char buf[110];

  // start time
  snprintf(buf, sizeof(buf), "init       %10ums %10uus", init_time, init_time_us);
  screen.text(buf, minimal_font, {4, 20}, false);

  // now
  snprintf(buf, sizeof(buf), "now        %10ums %10uus", display_current_time, display_current_time_us);
  screen.text(buf, minimal_font, {4, 30}, false);

  // time from init
  snprintf(buf, sizeof(buf), "from init  %10ums %10uus", display_current_time - init_time, us_diff(init_time_us, display_current_time_us));
  screen.text(buf, minimal_font, {4, 40}, false);

  // num update/render
  snprintf(buf, sizeof(buf), "update     %10ums (%10u x 10)\nrender     %10ums (%10u x 20)",
           display_num_updates * 10, display_num_updates, display_num_renders * 20, display_num_renders);
  screen.text(buf, minimal_font, {4, 50}, false);

  // value passed into update vs now()
  snprintf(buf, sizeof(buf), "update arg %10ums", display_fake_last_update);
  screen.text(buf, minimal_font, {4, 70}, false);

  snprintf(buf, sizeof(buf), "update now %10ums", display_real_last_update);
  screen.text(buf, minimal_font, {4, 80}, false);

  // timers
  snprintf(buf, sizeof(buf), "timer 100  %10ums (%10u x 100)\ntimer 10   %10ums (%10u x 10)\ntimer 1    %10ums",
           display_num_timer100 * 100, display_num_timer100, display_num_timer10 * 10, display_num_timer10, display_num_timer1);
  screen.text(buf, minimal_font, {4, 90}, false);


  // fake a slow render
  if(slow_render) {
    auto target = now() + 55;
    while(now() != target);
    slow_render = false;
  }
}

void update(uint32_t time) {
  num_updates++;
  fake_last_update = time; //very fake
  real_last_update = now();

  if(buttons.pressed & Button::X)
    paused = !paused;

  // fake a slow update
  if(buttons.pressed & Button::A) {
    auto target = now() + 55;
    while(now() != target);
  }

  if(buttons.pressed & Button::B)
    slow_render = true;
}
