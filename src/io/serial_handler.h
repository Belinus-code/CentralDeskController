#pragma once
#include <Arduino.h>

namespace Desk
{
    class SystemIO;
    class PC;
    class AC;
    class RGBController;
    class TimeManager;
    class Network;
}

namespace Desk
{
    class SerialHandler
    {
    public:
        void init(SystemIO *io, PC *pc, AC *ac, RGBController *rgb, TimeManager *time, Network *network);

        void update();

    private:
        SystemIO *io_ = nullptr;
        PC *pc_ = nullptr;
        AC *ac_ = nullptr;
        RGBController *rgb_ = nullptr;
        TimeManager *time_ = nullptr;
        Network *network_ = nullptr;

        String input_buffer_ = "";

        void processInput(String input);

        void handlePcCommand(String args);
        void handleAcCommand(String args);
        void handleRgbCommand(String args);
        void executeDump();

        // Extracts a specific word from a space-separated string (index 1 of "new static RED" is "static")
        String getCmdArg(const String &data, int index);
    };
}