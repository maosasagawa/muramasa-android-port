#ifndef MURAMASA_WINDOW_HPP
#define MURAMASA_WINDOW_HPP

#include "Window.hpp"

class MuramasaInterpreter;

class MuramasaWindow : public Window
{
public:
    MuramasaWindow();
    void SetInterpreter(MuramasaInterpreter* pInterpreter);
};

#endif
