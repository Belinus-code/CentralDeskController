#include "pc.h"

void PC::togglePC()
{
    doing_start_ = millis();
    doing_duration_ = PC_TOGGLE_TIME;
    io_->setRelais(true);
}

void PC::resetPC()
{
    doing_start_ = millis();
    doing_duration_ = PC_RESET_TIME;
    io_->setRelais(true);
}

void PC::onButtonChange(bool button_state)
{
    if (button_state)
    {
        button_overwrite_ = true;
        io_->setRelais(true);
    }
    else
    {
        button_overwrite_ = false;
        io_->setRelais(false);
    }
}

void PC::update()
{
    if (doing_duration_ != 0 && !button_overwrite_ && millis() - doing_start_ > doing_duration_)
    {
        doing_duration_ = 0;
        io_->setRelais(false);
    }
}