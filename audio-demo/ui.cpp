#include "ui.hpp"

#include "engine/api.hpp"
#include "engine/engine.hpp"
#include "engine/input.hpp"

using namespace blit;

void UIItem::render() {
    if(selected && children.empty()) {
        screen.pen = {255, 255, 255, 90};
        screen.rectangle(display_rect);
    }

    screen.pen = {255, 255, 255};

    switch(type){
        case UIType::None:
            screen.text(text, minimal_font, display_rect, true, TextAlign::center_center);
            break;

        case UIType::Checkbox: {
            bool checked = value != 0;

            auto text_bounds = screen.measure_text(text, minimal_font);
            int check_size = std::min(display_rect.w, display_rect.h - text_bounds.h) - 8;

            Point off;
            off.x = (display_rect.w - check_size) / 2;
            off.y = (display_rect.h - (check_size + text_bounds.h)) / 2;

            screen.text(text, minimal_font, Point(display_rect.x + display_rect.w / 2, display_rect.y + off.y + check_size + 4), true, TextAlign::top_center);

            Rect check_rect(display_rect.tl() + off, Size(check_size, check_size));

            if(!checked)
                screen.pen.a = 127;

            screen.rectangle(check_rect);

            break;
        }

        case UIType::Slider: {
            auto text_bounds = screen.measure_text(text, minimal_font);

            int slider_h = (display_rect.h - (text_bounds.h + 4)) / 2;

            Rect bar_rect(display_rect.tl() + Point(4, slider_h / 2), Size(display_rect.w - 8, slider_h));
            screen.pen = {127, 127, 127};
            screen.rectangle(bar_rect);

            screen.pen = {255, 255, 255};
            bar_rect.w = (value - min) * bar_rect.w / (max - min);
            screen.rectangle(bar_rect);

            screen.text(text, minimal_font, display_rect.bl() + Point(4, -2), true, TextAlign::bottom_left);
            screen.text(std::to_string(value), minimal_font, display_rect.br() + Point(-4, -2), true, TextAlign::bottom_right);
            break;
        }

    }

    for(auto &child : children)
        child.render();
}

void UIItem::update(uint32_t time) {
    // top-level update
    Point navigation;
    if(buttons.released & Button::DPAD_LEFT)
        navigation.x = -1;
    else if(buttons.released & Button::DPAD_RIGHT)
        navigation.x = 1;

     if(buttons.released & Button::DPAD_UP)
        navigation.y = -1;
    else if(buttons.released & Button::DPAD_DOWN)
        navigation.y = 1;

    handle_navigation(navigation);

    do_update(time);
}

void UIItem::set_display_rect(const blit::Rect &rect) {
    if(rect == display_rect)
        return;

    display_rect = rect;
    update_layout();
}

UIItem &UIItem::add_child(UIType type, std::string text, int id) {
    bool empty = children.empty();

    auto &ret = children.emplace_back();

    ret.id = id;
    ret.type = type;
    ret.text = std::move(text);

    if(!empty || !selected)
        ret.selected = false;

    if(!display_rect.empty())
        update_layout();

    return ret;
}

std::list<UIItem> &UIItem::get_children() {
    return children;
}

void UIItem::set_direction(UIDirection dir) {
    if(direction == dir)
        return;

    direction = dir;

    if(!display_rect.empty())
        update_layout();
}

UIItem &UIItem::get_selected_item() {
    if(children.empty())
        return *this; // assume selected

    for(auto &child : children) {
        if(child.selected)
            return child.get_selected_item();
    }

    // shouldn't happen
    return *this;
}

void UIItem::set_range(int min, int max, int step, int value) {
    this->min = min;
    this-> max = max;
    this->step = step;
    this->value = value;
}

void UIItem::update_layout() {
    if(children.empty())
        return;

    auto child_rect = display_rect;
    Point step;

    if(direction == UIDirection::Horizontal)
        child_rect.w = step.x = display_rect.w / children.size();
    else
        child_rect.h = step.y = display_rect.h / children.size();

    for(auto &child : children) {
        child.set_display_rect(child_rect);
        child_rect.x += step.x;
        child_rect.y += step.y;
    }
}

void UIItem::do_update(uint32_t time) {
    for(auto &child : children) {
        child.do_update(time);
    }

    // self update
    if(selected) {
        if(type == UIType::Checkbox) {
            if(buttons.released & Button::A)
                value = !value;
        } else if(type == UIType::Slider) {
            // need to cancel navigation somehow...

            int step = this->step;

            if(buttons & Button::A)
                step *= 10;

            if(buttons.released & Button::DPAD_LEFT && value > min) {
                value -= step;
                if(value < min) value = min;
            } else if(buttons.released & Button::DPAD_RIGHT && value < max) {
                value += step;
                if(value > max) value = max;
            }
        }
    }
}

void UIItem::handle_navigation(Point &navigation) {
    if(children.empty())
        return;

    auto it = children.begin();
    for(; it != children.end(); ++it) {
        if(it->selected) {
            it->handle_navigation(navigation);
            break;
        }
    }

    auto &nav = direction == UIDirection::Horizontal ? navigation.x : navigation.y;
    if(!nav) return;

    auto next = it;
    ++next;

    // currently selected leaf
    Point old_pos = it->get_selected_item().display_rect.center();

    if(nav == 1 && next != children.end()) {
        nav = 0;

        it->set_selected(false);
        next->set_selected(true, old_pos);
    } else if(nav == -1 && it != children.begin()) {
        nav = 0;

        it->set_selected(false);
        --it;
        it->set_selected(true, old_pos);
    }
}

void UIItem::set_selected(bool selected, Point old_selection) {
    if(this->selected == selected)
        return;

    this->selected = selected;

    if(children.empty())
        return;

    if(selected) {
        // select child closest to the old selected item
        auto nearest = children.begin();
        int dist = 0x7FFFFFFF;

        for(auto it = children.begin(); it != children.end(); ++it) {
            Point p = it->display_rect.center() - old_selection;
            int new_dist = p.x * p.x + p.y * p.y;

            if(new_dist < dist) {
                dist = new_dist;
                nearest = it;
            }
        }

        nearest->set_selected(true, old_selection);
    } else {
        for(auto &child : children)
            child.set_selected(false);
    }
}