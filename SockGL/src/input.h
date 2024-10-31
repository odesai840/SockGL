#ifndef INPUT_H
#define INPUT_H

#include <unordered_map>
#include <string>

#include "GLFW/glfw3.h"

enum State {
    NONE,
    PRESSED,
    HELD,
    RELEASED
};

class Input {
public:
    // updates key states based on values passed from key callback
    void UpdateKeyState(int key, int action);

    State GetKeyState(int key);

    // returns true when specified key is pressed
    bool GetKeyPressed(std::string key);

    // returns true while specified key is held
    bool GetKeyHeld(std::string key);

    // returns true when specified key is released
    bool GetKeyReleased(std::string key);

private:
    std::unordered_map<int, State> keyStates;

    std::unordered_map<std::string, int> keyMap = {
    {"Spacebar", GLFW_KEY_SPACE},
    {"'", GLFW_KEY_APOSTROPHE},
    {",", GLFW_KEY_COMMA},
    {"-", GLFW_KEY_MINUS},
    {".", GLFW_KEY_PERIOD},
    {"/", GLFW_KEY_SLASH},
    {"0", GLFW_KEY_0},
    {"1", GLFW_KEY_1},
    {"2", GLFW_KEY_2},
    {"3", GLFW_KEY_3},
    {"4", GLFW_KEY_4},
    {"5", GLFW_KEY_5},
    {"6", GLFW_KEY_6},
    {"7", GLFW_KEY_7},
    {"8", GLFW_KEY_8},
    {"9", GLFW_KEY_9},
    {";", GLFW_KEY_SEMICOLON},
    {"=", GLFW_KEY_EQUAL},
    {"A", GLFW_KEY_A},
    {"B", GLFW_KEY_B},
    {"C", GLFW_KEY_C},
    {"D", GLFW_KEY_D},
    {"E", GLFW_KEY_E},
    {"F", GLFW_KEY_F},
    {"G", GLFW_KEY_G},
    {"H", GLFW_KEY_H},
    {"I", GLFW_KEY_I},
    {"J", GLFW_KEY_J},
    {"K", GLFW_KEY_K},
    {"L", GLFW_KEY_L},
    {"M", GLFW_KEY_M},
    {"N", GLFW_KEY_N},
    {"O", GLFW_KEY_O},
    {"P", GLFW_KEY_P},
    {"Q", GLFW_KEY_Q},
    {"R", GLFW_KEY_R},
    {"S", GLFW_KEY_S},
    {"T", GLFW_KEY_T},
    {"U", GLFW_KEY_U},
    {"V", GLFW_KEY_V},
    {"W", GLFW_KEY_W},
    {"X", GLFW_KEY_X},
    {"Y", GLFW_KEY_Y},
    {"Z", GLFW_KEY_Z},
    {"{", GLFW_KEY_LEFT_BRACKET},
    {"}", GLFW_KEY_RIGHT_BRACKET},
    {"`", GLFW_KEY_GRAVE_ACCENT},
    {"Escape", GLFW_KEY_ESCAPE},
    {"Enter", GLFW_KEY_ENTER},
    {"Tab", GLFW_KEY_TAB},
    {"Backspace", GLFW_KEY_BACKSPACE},
    {"Insert", GLFW_KEY_INSERT},
    {"Delete", GLFW_KEY_DELETE},
    {"RightArrow", GLFW_KEY_RIGHT},
    {"LeftArrow", GLFW_KEY_LEFT},
    {"DownArrow", GLFW_KEY_DOWN},
    {"UpArrow", GLFW_KEY_UP},
    {"PageUp", GLFW_KEY_PAGE_UP},
    {"PageDown", GLFW_KEY_PAGE_DOWN},
    {"Home", GLFW_KEY_HOME},
    {"End", GLFW_KEY_END},
    {"CapsLock", GLFW_KEY_CAPS_LOCK},
    {"ScrollLock", GLFW_KEY_SCROLL_LOCK},
    {"NumLock", GLFW_KEY_NUM_LOCK},
    {"PrintScreen", GLFW_KEY_PRINT_SCREEN},
    {"Pause", GLFW_KEY_PAUSE},
    {"F1", GLFW_KEY_F1},
    {"F2", GLFW_KEY_F2},
    {"F3", GLFW_KEY_F3},
    {"F4", GLFW_KEY_F4},
    {"F5", GLFW_KEY_F5},
    {"F6", GLFW_KEY_F6},
    {"F7", GLFW_KEY_F7},
    {"F8", GLFW_KEY_F8},
    {"F9", GLFW_KEY_F9},
    {"F10", GLFW_KEY_F10},
    {"F11", GLFW_KEY_F11},
    {"F12", GLFW_KEY_F12},
    {"F13", GLFW_KEY_F13},
    {"F14", GLFW_KEY_F14},
    {"F15", GLFW_KEY_F15},
    {"F16", GLFW_KEY_F16},
    {"F17", GLFW_KEY_F17},
    {"F18", GLFW_KEY_F18},
    {"F19", GLFW_KEY_F19},
    {"F20", GLFW_KEY_F20},
    {"F21", GLFW_KEY_F21},
    {"F22", GLFW_KEY_F22},
    {"F23", GLFW_KEY_F23},
    {"F24", GLFW_KEY_F24},
    {"F25", GLFW_KEY_F25},
    {"LeftShift", GLFW_KEY_LEFT_SHIFT},
    {"LeftCtrl", GLFW_KEY_LEFT_CONTROL},
    {"LeftAlt", GLFW_KEY_LEFT_ALT},
    {"LeftSuper", GLFW_KEY_LEFT_SUPER},
    {"RightShift", GLFW_KEY_RIGHT_SHIFT},
    {"RightCtrl", GLFW_KEY_RIGHT_CONTROL},
    {"RightAlt", GLFW_KEY_RIGHT_ALT},
    {"RightSuper", GLFW_KEY_RIGHT_SUPER},
    {"Menu", GLFW_KEY_MENU}
    };
};
#endif