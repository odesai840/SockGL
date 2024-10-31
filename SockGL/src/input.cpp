#include "input.h"

void Input::UpdateKeyState(int key, int action) {
    if (action == GLFW_PRESS) {
        keyStates[key] = PRESSED;
    }
    else if (action == GLFW_RELEASE) {
        keyStates[key] = RELEASED;
    }
    else if (action == GLFW_REPEAT) {
        keyStates[key] = HELD;
    }
    else {
        keyStates[key] = NONE;
    }
}

State Input::GetKeyState(int key) {
    return keyStates[key];
}

bool Input::GetKeyPressed(std::string key) {
    int glfwKey = keyMap[key];
    if (GetKeyState(glfwKey) == PRESSED) {
        keyStates[glfwKey] = NONE;
        return true;
    }
    else {
        return false;
    }
}

bool Input::GetKeyHeld(std::string key) {
    int glfwKey = keyMap[key];
    if (GetKeyState(glfwKey) == PRESSED) {
        return true;
    }
    else if (GetKeyState(glfwKey) == HELD) {
        return true;
    }
    else {
        return false;
    }
}

bool Input::GetKeyReleased(std::string key) {
    int glfwKey = keyMap[key];
    if (GetKeyState(glfwKey) == RELEASED) {
        keyStates[glfwKey] = NONE;
        return true;
    }
    else {
        return false;
    }
}