#pragma once
#include "io.h"

class PC : public IButtonListener
{
public:
    // Constructor
    PC(IO *io) : io_(io) {};

    // Update Method checks everything
    void update();

    // toggles pc (non-blocking)
    void togglePC();

    // hard-resets pc (non-blocking)
    void resetPC();

    // returns status of pc given of by its LED-Light
    bool getPCState() const { return io_.getPcState(); }

    // handels button change isr
    void onButtonChange(bool button_state);

private:
    IO *io_;
    bool button_overwrite_ = false;
    uint32_t doing_start_ = 0;    // millis-time of starting relay action
    uint32_t doing_duration_ = 0; // Duration of relay action. Zero means no active action
};