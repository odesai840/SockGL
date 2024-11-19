#ifndef INPUT_H
#define INPUT_H

#include <vector>

#include "GLFW/glfw3.h"

enum State {
    NONE,
    PRESSED,
    HELD,
    RELEASED
};

class Input {
public:
    void UpdateKeyState(int key, int action);
    bool GetKeyPressed(int key);
    bool GetKeyHeld(int key);
    bool GetKeyReleased(int key);
private:
    std::vector<State> keyStates = std::vector<State>(GLFW_KEY_LAST + 1, NONE);
};
#endif