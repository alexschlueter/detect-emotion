#include "controls.h"

Controls::Controls(const std::map<int, std::function<void()>> &keyMap/* = std::map<int, std::function<void()>>()*/)
: keyMap(keyMap)
{
    // nothing
}

bool Controls::HandleInput(int keyCode)
{
    auto iterator = keyMap.find(keyCode);
    if(iterator == keyMap.end())
        return false;

    iterator->second();
    return true;
}

void Controls::Bind(int keyCode, std::function<void()> func)
{
    keyMap[keyCode] = func;
}

bool Controls::Unbind(int keyCode)
{
    return keyMap.erase(keyCode);
}