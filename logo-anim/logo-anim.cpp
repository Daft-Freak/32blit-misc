#include "logo-anim.hpp"
#include "assets.hpp"

using namespace blit;

const Font sans_bold_italic_font(asset_sans_bold_italic_font);

const std::string_view anim_text = "TEXT HERE";
const auto &anim_font = sans_bold_italic_font; //minimal_font;

const float zoom_scale = 3.75f;
const Vec2 swirl_start(-0.275f, 0.4f); // these are fractions of the screen size
const Vec2 swirl_end(0.0f, -0.005f);   //
const float bounce_dist = 0.0625;      //
const int bounce_count = 3;

const Vec3 colours[] { // HSV
  {300.0f, 0.0f, 1.0f}, // from white
  {300.0f, 1.0f, 1.0f}, // to purple
  {  0.0f, 1.0f, 1.0f}, // start cycle
  {300.0f, 1.0f, 1.0f}, // RAINBOW
  {240.0f, 1.0f, 1.0f}, // back to blue
};

// timing params
const unsigned initial_anim_delay_ms   = 460; // delay before starting
const unsigned anim_per_char_delay_ms  =  90; // delay between chars (excluding spaces)

const unsigned zoom_duration_ms        = 800;
const unsigned swirl_x_duration_ms     = 800;
const unsigned swirl_y_duration_ms     = 530;
const unsigned swirl_slide_duration_ms = 270; // y axis snaps into place at the end
const unsigned bounce_y_duration_ms    = 500;

const unsigned colour_end_ms[] { // when each step of colour transition ends
  120,
  120 + 140,
  120 + 140 + 1750,
  120 + 140 + 1750 + 390,
};


// easings (calculated from cubic-bezier)
// cubic-bezier(0.2, 2.1, 0.75, 1.3)
const float swirl_x_easing[] {
  0.0f,
  0.123338282f,
  0.233013541f,
  0.331530184f,
  0.420709312f,
  0.501924396f,
  0.576241732f,
  0.64450866f,
  0.707412124f,
  0.765517831f,
  0.819298565f,
  0.869154096f,
  0.915426254f,
  0.958409786f,
  0.99836123f,
  1.03550541f,
  1.07004058f,
  1.10214281f,
  1.13196945f,
  1.15966117f,
  1.18534529f,
  1.20913661f,
  1.23113978f,
  1.25144982f,
  1.27015412f,
  1.28733265f,
  1.30305898f,
  1.31740129f,
  1.33042252f,
  1.34218109f,
  1.35273135f,
  1.36212397f,
  1.37040627f,
  1.3776226f,
  1.38381469f,
  1.38902152f,
  1.39327991f,
  1.39662468f,
  1.39908862f,
  1.40070271f,
  1.40149653f,
  1.40149808f,
  1.40073407f,
  1.39922976f,
  1.39700949f,
  1.39409649f,
  1.39051294f,
  1.38628018f,
  1.38141882f,
  1.37594855f,
  1.36988854f,
  1.36325729f,
  1.35607266f,
  1.34835231f,
  1.34011304f,
  1.33137155f,
  1.32214415f,
  1.31244683f,
  1.30229557f,
  1.29170609f,
  1.28069377f,
  1.26927447f,
  1.25746393f,
  1.24527776f,
  1.23273218f,
  1.21984363f,
  1.20662904f,
  1.19310582f,
  1.17929256f,
  1.16520834f,
  1.15087366f,
  1.13631058f,
  1.12154293f,
  1.10659671f,
  1.091501f,
  1.0762881f,
  1.0609951f,
  1.04566455f,
  1.03034651f,
  1.01510084f,
};

// cubic-bezier(0.15, 1.3, 0.42, 1.8)
const float swirl_y_easing[] {
  0.0f,
  0.154287234f,
  0.291811138f,
  0.414790452f,
  0.525061667f,
  0.624154449f,
  0.713351607f,
  0.793736517f,
  0.866230369f,
  0.931621909f,
  0.990590513f,
  1.04372501f,
  1.09153867f,
  1.13448119f,
  1.17294896f,
  1.2072922f,
  1.23782241f,
  1.26481783f,
  1.28852737f,
  1.30917501f,
  1.32696295f,
  1.3420738f,
  1.35467398f,
  1.36491466f,
  1.37293386f,
  1.37885797f,
  1.3828032f,
  1.38487613f,
  1.38517511f,
  1.38379121f,
  1.38080847f,
  1.37630475f,
  1.37035286f,
  1.36301994f,
  1.35436904f,
  1.34445894f,
  1.33334446f,
  1.32107711f,
  1.30770493f,
  1.29327333f,
  1.27782488f,
  1.26139975f,
  1.24403548f,
  1.22576785f,
  1.20663059f,
  1.1866554f,
  1.16587257f,
  1.14431047f,
  1.12199628f,
  1.09895563f,
  1.07521307f,
  1.05079174f,
  1.02571368f,
};

struct AnimChar {
  char c;
  Point target;
  float target_scale;

  // timing
  unsigned anim_time = 0;
  unsigned anim_delay = 0;

  // current values
  Point pos;
  float scale = 0.0f;
  Pen colour;
};

static std::vector<AnimChar> anim_chars;

// big text helper
static void stretch_text(std::string_view text, const Font &font, const Point &pos, float scale, TextAlign align) {
    auto bounds = screen.measure_text(text, font);

    // add padding (avoid clipping italic fonts)
    int padding = 16;
    bounds.w += padding * 2;

    auto buf = new uint8_t[bounds.area()]();
    Pen palette[] {
        {0, 0, 0, 0},
        screen.pen
    };

    Surface surf(buf, PixelFormat::P, bounds);
    surf.palette = palette;
    surf.pen = {1};

    Rect surf_rect({0, 0}, bounds);

    // draw text centered in the padding
    Rect text_rect = surf_rect;
    text_rect.x = padding;
    text_rect.w -= padding * 2;

    surf.text(text, font, text_rect, true, align);

    Rect dest(pos, bounds * scale);

    // (re-)align
    if(align & TextAlign::center_h)
        dest.x -= bounds.w * scale / 2;
    //...right

    if(align & TextAlign::center_v)
        dest.y -= bounds.h * scale / 2;
    //...bottom

    screen.stretch_blit(&surf, surf_rect, dest);

    delete[] buf;
}

void init() {

  set_screen_mode(ScreenMode::hires);

  // create char list
  anim_chars.reserve(anim_text.length());

  for(auto c : anim_text) {
    AnimChar ac;
    ac.c = c;
    anim_chars.emplace_back(ac);
  }

  // measure width
  auto full_size = screen.measure_text(anim_text, anim_font);

  // aim to fill 2/3 of the screen
  int target_w = screen.bounds.w * 2 / 3;
  float scale = float(target_w) / full_size.w;

  // position characters
  float x = (screen.bounds.w - target_w) / 2;
  int anim_delay_ms = initial_anim_delay_ms;

  for(auto &c : anim_chars) {
    auto char_size = screen.measure_text(std::string_view{&c.c, 1}, anim_font);
    int scaled_w = scale * char_size.w;

    c.target.x = x + scaled_w / 2;
    c.target.y = screen.bounds.h * 2 / 5; // TODO: a bit higher?
    c.target_scale = scale;

    c.anim_delay = anim_delay_ms;

    if(c.c != ' ')
      anim_delay_ms += anim_per_char_delay_ms;

    x += scaled_w;
  }
}

void render(uint32_t time_ms) {
  screen.pen = {255, 255, 255};
  screen.clear();

  for(auto &c : anim_chars) {
    screen.pen = c.colour;
    stretch_text(std::string_view{&c.c, 1}, anim_font, c.pos, c.scale, TextAlign::center_center);
  }
}

void update(uint32_t time) {
  // update animation
  for(auto &c : anim_chars) {
    c.anim_time += 10;

    if(c.anim_time < c.anim_delay)
      continue;

    auto anim_time = c.anim_time - c.anim_delay;

    // zoom
    if(anim_time < zoom_duration_ms) {
      float t = float(anim_time) / zoom_duration_ms;

      c.scale = (zoom_scale * (1.0f - t) + t/* end value is 1.0 */) * c.target_scale;
    } else
      c.scale = c.target_scale;

    // swirl
    // X axis
    if(anim_time < swirl_x_duration_ms) {
      float t = float(anim_time) / swirl_x_duration_ms;
      t = swirl_x_easing[int(t * std::size(swirl_x_easing))];

      float v = swirl_start.x * (1.0f - t) + swirl_end.x * t;

      c.pos.x = c.target.x + v * screen.bounds.w;
    } else
      c.pos.x = c.target.x;

    // Y axis
    if(anim_time < swirl_y_duration_ms) {
      float t = float(anim_time) / swirl_y_duration_ms;
      t = swirl_y_easing[int(t * std::size(swirl_y_easing))];

      float v = swirl_start.y * (1.0f - t) + swirl_end.y * t;

      c.pos.y = c.target.y + v * screen.bounds.h;
    } else if(anim_time < swirl_y_duration_ms + swirl_slide_duration_ms) {
      // snap into place
      float t = float(anim_time - swirl_y_duration_ms) / swirl_slide_duration_ms;

      float v = swirl_end.y * (1.0f - t) + 0.0f/*!*/ * t;

      c.pos.y = c.target.y + v * screen.bounds.h;

    } else if(anim_time < swirl_y_duration_ms + swirl_slide_duration_ms + bounce_y_duration_ms) {
      // bouncy
      float t = float(anim_time - (swirl_y_duration_ms + swirl_slide_duration_ms)) / bounce_y_duration_ms;

      int dist = bounce_dist * screen.bounds.h;

      float v = std::abs(std::sin(t * t * bounce_count * pi)) * dist;

      v *= (1.0f - t); // make each bounce smaller

      c.pos.y = c.target.y - v;
    } else
      c.pos.y = c.target.y;

    // colour

    // find which part of the animation we're doing
    unsigned col_index = 0;

    while(col_index < std::size(colour_end_ms) && colour_end_ms[col_index] < anim_time)
      col_index++;

    Vec3 hsv = colours[std::size(colours) - 1];

    if(col_index < std::size(colour_end_ms)) {
      // get colours
      auto from_col = colours[col_index];
      auto to_col = colours[col_index + 1];

      // go the short way
      if(to_col.x < from_col.x && from_col.x - to_col.x > 180.0f) {
        to_col.x += 360.0f;
      }

      // get time
      int start = col_index ? colour_end_ms[col_index - 1] : 0;
      int duration = colour_end_ms[col_index] - start;
      
      float t = float(anim_time - start) / duration;

      hsv = from_col * (1.0f - t) + to_col * t;
    }

    c.colour = hsv_to_rgba(hsv.x / 360.0f, hsv.y, hsv.z);
  }

  // reset on A
  if(buttons.released & Button::A) {
    for(auto &c : anim_chars) {
      c.anim_time = 0;
      c.scale = 0.0f;
    }
  }
}
