#include "launcher-test.hpp"
#include "file-browser.hpp"

#include "engine/api_private.hpp"

FileBrowser file_browser;

void launch_game(std::string filename) {
  blit::api.launch(filename.c_str());
}

void init() {
  blit::set_screen_mode(blit::ScreenMode::hires);

  file_browser.set_extensions({".blit"});
  file_browser.set_on_file_open(launch_game);
  file_browser.init();
}

void render(uint32_t time_ms) {
  file_browser.render();
}

void update(uint32_t time_ms) {
  file_browser.update(time_ms);
}
