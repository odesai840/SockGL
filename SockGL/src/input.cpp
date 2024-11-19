#include "input.h"

void Input::UpdateKeyState(int key, int action) {
    if (key < 0 || key > GLFW_KEY_LAST) return; // Prevent invalid key access
    switch (action) {
    case GLFW_PRESS: keyStates[key] = PRESSED; break;
    case GLFW_RELEASE: keyStates[key] = RELEASED; break;
    case GLFW_REPEAT: keyStates[key] = HELD; break;
    default: keyStates[key] = NONE; break;
    }
}

bool Input::GetKeyPressed(int key) {
    if (key < 0 || key > GLFW_KEY_LAST) return false;
    if (keyStates[key] == PRESSED) {
        keyStates[key] = HELD;  // Transition to HELD after checking
        return true;
    }
    return false;
}

bool Input::GetKeyHeld(int key) {
    if (key < 0 || key > GLFW_KEY_LAST) return false;
    return keyStates[key] == PRESSED || keyStates[key] == HELD;
}

bool Input::GetKeyReleased(int key) {
    if (key < 0 || key > GLFW_KEY_LAST) return false;
    if (keyStates[key] == RELEASED) {
        keyStates[key] = NONE;  // Clear state after checking
        return true;
    }
    return false;
}