#ifndef TIMESCREEN_H
#define TIMESCREEN_H

#include "Screen.h"

// Abstract base class for screens. Each screen must provide a method for
// showing its face, and methods for handling up, down, back, and menu button
// presses. Default handlers set watchy screen to that screen.
class TimeScreen : public Screen
{
public:
    virtual void show(); // display this screen
};
#endif