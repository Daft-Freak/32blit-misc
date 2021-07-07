#include <list>

#include "audio-demo.hpp"
#include "ui.hpp"

using namespace blit;

enum class ItemID {
    Wave_Noise = 0,
    Wave_Square,
    Wave_Saw,
    Wave_Triangle,
    Wave_Sine,

    Frequency,
    Volume,
    AttackTime,
    DecayTime,
    ReleaseTime,
    SustainVol,
};

UIItem ui_root;

void init() {
    set_screen_mode(ScreenMode::hires);

    auto &waveforms = ui_root.add_child();
    waveforms.set_direction(UIDirection::Horizontal);

    auto id = [](ItemID i) {return static_cast<int>(i);};

    waveforms.add_child(UIType::Checkbox, "Noise", id(ItemID::Wave_Noise));
    waveforms.add_child(UIType::Checkbox, "Square", id(ItemID::Wave_Square));
    waveforms.add_child(UIType::Checkbox, "Saw", id(ItemID::Wave_Saw));
    waveforms.add_child(UIType::Checkbox, "Tri", id(ItemID::Wave_Triangle));
    waveforms.add_child(UIType::Checkbox, "Sine", id(ItemID::Wave_Sine));

    ui_root.add_child(UIType::Slider, "Frequency", id(ItemID::Frequency)).set_range(10, 10000, 10, 660);
    ui_root.add_child(UIType::Slider, "Volume", id(ItemID::Volume)).set_range(0, 0xFFFF, 655, 0xFFFF);

    ui_root.add_child(UIType::Slider, "Attack time", id(ItemID::AttackTime)).set_range(1, 1000, 1, 2);
    ui_root.add_child(UIType::Slider, "Decay time", id(ItemID::DecayTime)).set_range(1, 1000, 1, 6);
    ui_root.add_child(UIType::Slider, "Release time", id(ItemID::ReleaseTime)).set_range(1, 1000, 1, 1);

    ui_root.add_child(UIType::Slider, "Sustain vol", id(ItemID::SustainVol)).set_range(0, 0xFFFF, 655, 0xFFFF);

    ui_root.set_display_rect(Rect(screen.bounds.w / 2, 0, screen.bounds.w / 2, screen.bounds.h));
}

// 100% not stolen from launcher-shared
void button_icon(const Point &pos, int button) {
    Pen active_col(0xff, 0xff, 0xff);
    Pen col(0x50, 0x64, 0x78);
    Rect r(pos.x, pos.y + 3, 2, 2);

    screen.pen = button == Button::Y ? active_col : col;
    screen.rectangle(r);

    screen.pen = button == Button::A ? active_col : col;
    r.x += 6;
    screen.rectangle(r);

    screen.pen = button == Button::X ? active_col : col;
    r.x -= 3; r.y = pos.y;
    screen.rectangle(r);

    screen.pen = button == Button::B ? active_col : col;
    r.y += 6;
    screen.rectangle(r);
}

void render(uint32_t time) {
    screen.pen = Pen(0, 0, 0);
    screen.clear();

    ui_root.render();

    screen.text("    Attack/Release\n\n    Toggle Selected\n\nLEFT/RIGHT Small Step\n\n    + LEFT/RIGHT Big Step", minimal_font, {8, 8});
    button_icon({8, 7}, Button::X);
    button_icon({8, 25}, Button::A);
    button_icon({8, 62}, Button::A);
}

// TODO: value changed callback/event/...
static void update_audio_params(UIItem &item) {
    auto &children = item.get_children();

    if(!children.empty()) {
        for(auto &child : children)
            update_audio_params(child);
        return;
    }

    auto &channel = channels[0];

    auto update_waveform = [&channel](bool val, int waveform) {
        if(val)
            channel.waveforms |= waveform;
        else
            channel.waveforms &= ~waveform;
    };

    switch(static_cast<ItemID>(item.get_id())) {
        case ItemID::Wave_Noise:
            update_waveform(item.get_value(), Waveform::NOISE);
            break;
        case ItemID::Wave_Square:
            update_waveform(item.get_value(), Waveform::SQUARE);
            break;
        case ItemID::Wave_Saw:
            update_waveform(item.get_value(), Waveform::SAW);
            break;
        case ItemID::Wave_Triangle:
            update_waveform(item.get_value(), Waveform::TRIANGLE);
            break;
        case ItemID::Wave_Sine:
            update_waveform(item.get_value(), Waveform::SINE);
            break;

        case ItemID::Frequency:
            channel.frequency = item.get_value();
            break;
        case ItemID::Volume:
            channel.volume = item.get_value();
            break;
        case ItemID::AttackTime:
            channel.attack_ms = item.get_value();
            break;
        case ItemID::DecayTime:
            channel.decay_ms = item.get_value();
            break;
        case ItemID::ReleaseTime:
            channel.release_ms = item.get_value();
            break;
        case ItemID::SustainVol:
            channel.sustain = item.get_value();
            break;

    }
}

void update(uint32_t time) {
    ui_root.update(time);
    update_audio_params(ui_root);

    if(buttons.released & Button::X) {
        if(channels[0].adsr_phase == ADSRPhase::OFF) // || decay?
            channels[0].trigger_attack();
        else
            channels[0].trigger_release();
    }
}