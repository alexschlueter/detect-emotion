#ifndef CONTROLS_H
#define CONTROLS_H

#include <map>
#include <functional>

class Controls
{
public:
    enum {
        KEY_SPACE = ' ',
        KEY_LEFT = 65361,
        KEY_RIGHT = 65363,
        KEY_0 = '0',
        KEY_9 = '9',
        KEY_N = 'n'
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