#ifndef MURAMASA_INTERPRETER_HPP
#define MURAMASA_INTERPRETER_HPP

#include "NSBInterpreter.hpp"

class MuramasaWindow;

class MuramasaInterpreter : public NSBInterpreter
{
public:
    MuramasaInterpreter(MuramasaWindow* pWindow);
};

#endif
