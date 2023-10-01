#pragma once

#include <functional>

namespace core {
////////////////////////////////////////////////////////////////////////////////

template <typename EnterFuncT = std::function<void(void)>, typename ExitFuncT = std::function<void(void)>>
struct Scoped {
private:
    ExitFuncT _onExit;

public:
    Scoped(ExitFuncT onExit)
        : _onExit(onExit)
    {
    }
    Scoped(EnterFuncT onEnter, ExitFuncT onExit)
        : _onExit(onExit)
    {
        onEnter();
    }
    ~Scoped() { _onExit(); }
};

////////////////////////////////////////////////////////////////////////////////
}   // namespace core {
