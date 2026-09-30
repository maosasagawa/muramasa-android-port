#include "MuramasaWindow.hpp"
#include "MuramasaInterpreter.hpp"

// Muramasa renders a 1024x576 widescreen game surface
MuramasaWindow::MuramasaWindow() : Window("Full Metal Daemon: Muramasa", 1024, 576)
{
}

void MuramasaWindow::SetInterpreter(MuramasaInterpreter* pInterpreter)
{
    this->pInterpreter = pInterpreter;
}
