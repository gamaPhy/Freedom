#pragma once
#include <vector>
#include <cstdint>
#include <string>
#include "settings_util.h"

class Menu {
private:
    class MenuItem {
    public:
        MenuItem(std::string name): name(name), parent(nullptr) {};
        std::string name;
        MenuItem* parent;

        virtual void key_0_event() = 0;
        virtual void key_1_event() = 0;
        virtual std::string get_selection_line() = 0;
        virtual std::vector<std::string> get_display_strings() = 0;
    };

    class MenuDirectory: public MenuItem {
    public:
        MenuDirectory(std::string name, std::vector<MenuItem*> sub_items): MenuItem(name), sub_items(sub_items), cursor_position(0) {};
        std::vector<MenuItem*> sub_items;
        int cursor_position;

        void key_0_event() {
            if (cursor_position > 0) {
                --cursor_position;
            }
        }

        void key_1_event() {
            if (cursor_position < sub_items.size() - 1) {
                ++cursor_position;
            }
        }

        std::string get_selection_line() {
            return "/" + name;
        }

        std::vector<std::string> get_display_strings() {
            std::vector<std::string> lines;
            lines.push_back(name);
            const auto& items = sub_items;
            for (int i = 0; i < items.size(); ++i) {
                std::string line = items[i]->get_selection_line();
                if (i == cursor_position) {
                    line = "> " + line;
                }
                else {
                    line = "  " + line;
                }
                lines.push_back(line);
            }

            return lines;
        }

    };

    class MenuValue: public MenuItem {
    public:
        MenuValue(std::string name, uint8_t* value, uint8_t lower_bound, uint8_t upper_bound, std::string description, std::string(*value_to_string)(uint8_t))
            : MenuItem(name), value(value), lower_bound(lower_bound), upper_bound(upper_bound), description(description), value_to_string(value_to_string) {};
        uint8_t* value;
        uint8_t lower_bound;
        uint8_t upper_bound;
        std::string description;
        std::string(*value_to_string)(uint8_t); // function : uint8_t value -> string that represents the value to the user

        // update value, which points to a setting so they are updated in real time
        void key_0_event() {
            if (*value > lower_bound) {
                --(*value);
            }
        }

        void key_1_event() {
            if (*value < upper_bound) {
                ++(*value);
            }
        }

        std::string get_selection_line() {
            return name + " : " + value_to_string(*value);
        }

        // settings are shown in real time
        std::vector<std::string> get_display_strings() {
            std::vector<std::string> lines;
            lines.push_back(name);
            lines.push_back(description);
            lines.push_back("");
            std::string value_string = value_to_string(*value);
            std::string value_line(9 - value_string.length() / 2, ' ');

            if (*value != lower_bound) {
                value_line += "< ";
            }
            else {
                value_line += "  ";
            }

            value_line += value_string;

            if (*value != upper_bound) {
                value_line += " >";
            }
            lines.push_back(value_line);

            return lines;
        }
    };

    MenuDirectory main_menu{
        "Main Menu",
        {
            new MenuValue{ "Rapid trigger", &settings.rapid_trigger_toggle, 0, 1, "rt description", [](uint8_t v) {return std::string(v ? "On" : "Off");} },
            new MenuValue{ "Actuation", &settings.actuation_point, 1, 40, "distance (mm)", [](uint8_t v) {return std::to_string(v / 10) + "." + std::to_string(v % 10);} },
            new MenuValue{ "Release", &settings.release_point, 1, 40, "distance (mm)", [](uint8_t v) {return std::to_string(v / 10) + "." + std::to_string(v % 10);} },
            new MenuDirectory{
                "RGB Settings",
                {
                    new MenuValue{ "Mode", &settings.rgb_mode, 0, NUM_RGB_MODES - 1, "what rgb is based on", [](uint8_t v) {return RGB_MODE_TEXT[v];} },
                    new MenuValue{ "Red", &settings.rgb_r, 0, 255, "how red u want", [](uint8_t v) {return std::to_string(v);} },
                    new MenuValue{ "Green", &settings.rgb_g, 0, 255, "how green u want", [](uint8_t v) {return std::to_string(v);} },
                    new MenuValue{ "Blue", &settings.rgb_b, 0, 255, "how blue u want", [](uint8_t v) {return std::to_string(v);} }
                }
            }
        }
    };

    void set_parents(MenuDirectory* directory) {
        for (MenuItem* sub_item : directory->sub_items) {
            sub_item->parent = directory;
            MenuDirectory* sub_dir = dynamic_cast<MenuDirectory*>(sub_item);
            if (sub_dir) {
                set_parents(sub_dir);
            }
        }
    }

    MenuItem* current_menu;

public:
    Menu() {
        reset();
        main_menu.parent = nullptr;
        set_parents(&main_menu);
    }

    void reset() {
        current_menu = &main_menu;
    }

    int select() {
        MenuDirectory* directory = dynamic_cast<MenuDirectory*>(current_menu);
        if (directory) {
            current_menu = directory->sub_items[directory->cursor_position];
            return 1;
        }

        return 0;
    }

    int go_back() {
        save_settings(settings);
        if (current_menu->parent == nullptr) {
            return 0;
        }
        current_menu = current_menu->parent;
        return 1;
    }

    void key_0_event() {
        current_menu->key_0_event();
    }

    void key_1_event() {
        current_menu->key_1_event();
    }

    std::vector<std::string> get_display_strings() {
        return current_menu->get_display_strings();
    }
};
