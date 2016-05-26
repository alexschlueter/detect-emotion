#ifndef CONTROLS_H
#define CONTROLS_H

#include <map>
#include <functional>

class Controls
{
public:
    enum {
        KEY_SPACE = 32,
        KEY_LEFT = 65361,
        KEY_RIGHT = 65363,
        KEY_0 = 48,
        KEY_9 = 57,
        KEY_N = 110
    };

    Controls(const std::map<int, std::function<void()>> &keyMap = std::map<int, std::function<void()>>());
    bool HandleInput(int keyCode);
    void Bind(int keyCode, std::function<void()> func);
    bool Unbind(int keyCode);
protected:
private:
    std::map<int, std::function<void()>> keyMap;
};

#endif