#include "Keyboard.hpp"

int Keyboard::keys[GLFW_KEY_LAST];
bool Keyboard::key_toggles[GLFW_KEY_LAST];

int Keyboard::getKey(int key) {
    assert(key >= 0 && key <= GLFW_KEY_LAST);
    return Keyboard::keys[key];
}

int Keyboard::getKeyDown(int key) {
    assert(key >= 0 && key <= GLFW_KEY_LAST);
    return Keyboard::keys[key] >= GLFW_PRESS;
}

int Keyboard::getKeyUp(int key) {
    assert(key >= 0 && key <= GLFW_KEY_LAST);
    return Keyboard::keys[key] == GLFW_RELEASE;
}

bool Keyboard::getKeyToggle(int key) {
    assert(key >= 0 && key <= GLFW_KEY_LAST);
    return Keyboard::key_toggles[key];
}

void Keyboard::setKeyStatus(int key, int action) {
    assert(key >= 0 && key <= GLFW_KEY_LAST);

    if (action == GLFW_PRESS) {
        Keyboard::key_toggles[key] = !Keyboard::key_toggles[key];
    }
    Keyboard::keys[key] = action;
}
