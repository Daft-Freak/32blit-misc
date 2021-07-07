#pragma once
#include <cstdint>
#include <list>
#include <string>

#include "types/point.hpp"
#include "types/rect.hpp"

enum class UIDirection {
    Horizontal = 0,
    Vertical
};

enum class UIType {
    None = 0, // just a container
    Checkbox,
    Slider
};

class UIItem final {
public:
    void render();
    void update(uint32_t time);

    int get_id() const {return id;}

    void set_display_rect(const blit::Rect &rect);

    UIItem &add_child(UIType type = UIType::None, std::string text = "", int id = -1);

    std::list<UIItem> &get_children();

    void set_direction(UIDirection dir);

    UIItem &get_selected_item();

    int get_value() const {return value;}
    void set_value(int value) {this->value = value;}

    void set_range(int min, int max, int step, int value);

private:
    void update_layout();

    void do_update(uint32_t time);

    void handle_navigation(blit::Point &navigation);

    void set_selected(bool selected, blit::Point old_selection = {});

    int id = -1;
    UIType type = UIType::None;

    blit::Rect display_rect;
    std::list<UIItem> children;
    UIDirection direction = UIDirection::Vertical;

    std::string text;

    bool selected = true;

    int value = 0;

    // slider
    int min = 0, max = 100, step = 1;
};