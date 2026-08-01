#include "serial_handler.h"
#include "system_io.h"
#include "pc.h"
#include "ac.h"
#include "rgb.h"
#include "time_manager.h"
#include "network.h"
#include "animations.h"

namespace Desk
{

    void SerialHandler::init(SystemIO *io, PC *pc, AC *ac, RGBController *rgb, TimeManager *time, Network *network)
    {
        io_ = io;
        pc_ = pc;
        ac_ = ac;
        rgb_ = rgb;
        time_ = time;
        network_ = network;
    }

    void SerialHandler::update()
    {
        // Read async without blocking the main loop
        while (Serial.available() > 0)
        {
            char c = Serial.read();
            if (c == '\n' || c == '\r')
            {
                if (input_buffer_.length() > 0)
                {
                    processInput(input_buffer_);
                    input_buffer_ = ""; // Clear buffer after execution
                }
            }
            else
            {
                input_buffer_ += c;
            }
        }
    }

    void SerialHandler::processInput(String input)
    {
        input.trim();
        String main_cmd = getCmdArg(input, 0);            // e.g., "rgb", "dump", "pc"
        String args = input.substring(main_cmd.length()); // The rest of the string
        args.trim();

        if (main_cmd == "rgb")
        {
            handleRgbCommand(args);
        }
        else if (main_cmd == "pc")
        {
            handlePcCommand(args);
        }
        else if (main_cmd == "ac")
        {
            handleAcCommand(args);
        }
        else if (main_cmd == "dump")
        {
            executeDump();
        }
        else if (main_cmd == "help")
        {
            Serial.println("Commands: help, dump, pc [cmd], ac [cmd], rgb [cmd]");
        }
        else
        {
            Serial.println("Unknown Command. Type 'help'.");
        }
    }

    void SerialHandler::handlePcCommand(String command)
    {
        command.toUpperCase();
        if (command == "TOGGLE")
        {
            if (pc_)
                pc_->togglePC();
            Serial.println("PC Toggled.");
        }
        else if (command == "RESET")
        {
            if (pc_)
                pc_->resetPC();
            Serial.println("PC Reset.");
        }
        else
        {
            Serial.println("Unknown PC command. Use: TOGGLE, RESET");
        }
    }

    void SerialHandler::handleAcCommand(String command)
    {
        // Route the command directly to the AC class logic you already built
        if (ac_)
        {
            ac_->processCommand(command);
            Serial.println("AC Command executed: " + command);
        }
    }

    void SerialHandler::handleRgbCommand(String args)
    {
        if (!rgb_ || !rgb_->getAnimMan())
            return;

        AnimationManager *anim_man = rgb_->getAnimMan();
        String action = getCmdArg(args, 0); // "set", "new", "setting", "list", "delete", etc.

        if (action == "set")
        {
            String anim_name = getCmdArg(args, 1);
            if (anim_name == "")
            {
                Serial.println("Usage: rgb set ANIMATION");
                return;
            }
            // HINWEIS: Ersetze setUserAnimation durch die Methode, die du in LedController nutzt
            if (rgb_->setUserAnimation(anim_name))
            {
                Serial.println("Switched to " + anim_name);
            }
            else
            {
                Serial.println("Animation not found.");
            }
        }
        else if (action == "list")
        {
            int amount = anim_man->getAnimationCount();
            for (int i = 0; i < amount && i < 100; i++)
            {
                IAnimation *ani = anim_man->getAnimation(i);
                if (ani)
                    Serial.println(ani->GetName());
            }
        }
        else if (action == "delete")
        {
            String anim_name = getCmdArg(args, 1);
            int index = anim_man->getAnimationIndex(anim_name);
            if (index != -1)
            {
                anim_man->deleteAnimation(index);
                Serial.println("Deleted " + anim_name);
            }
            else
            {
                Serial.println("Animation not found.");
            }
        }
        else if (action == "new")
        {
            String type = getCmdArg(args, 1); // "static", "blink", "fade"
            String name = getCmdArg(args, 2);

            if (type == "static")
            {
                uint32_t color = (uint32_t)strtoul(getCmdArg(args, 3).c_str(), NULL, 16);
                AnimationSetting *set = anim_man->createSettingsStaticColor(color, 255, name);
                anim_man->createAnimation(set);
                delete set;
                Serial.println("Created static: " + name);
            }
            else if (type == "blink")
            {
                uint32_t c_on = (uint32_t)strtoul(getCmdArg(args, 3).c_str(), NULL, 16);
                uint32_t c_off = (uint32_t)strtoul(getCmdArg(args, 4).c_str(), NULL, 16);
                uint8_t ticks = (uint8_t)getCmdArg(args, 5).toInt();

                AnimationSetting *set = anim_man->createSettingsBlink(c_on, c_off, ticks, 255, name);
                anim_man->createAnimation(set);
                delete set;
                Serial.println("Created blink: " + name);
            }
        }
        else
        {
            Serial.println("Unknown RGBController cmd. Try: set, list, new, delete");
        }
    }

    void SerialHandler::executeDump()
    {
        Serial.println("=== SYSTEM DUMP ===");

        if (time_)
        {
            uint32_t uptime = millis() / 1000;
            Serial.println("Uptime: " + String(uptime / 3600) + ":" + String((uptime % 3600) / 60) + ":" + String(uptime % 60));
            Serial.println("NTP Time: " + time_->getTimeString());
        }

        if (network_)
        {
            Serial.println("IP-Address: " + network_->getIpString());
        }

        if (pc_ && io_)
        {
            Serial.println("PC State: " + String(pc_->getPCState() ? "ON" : "OFF"));
            Serial.println("PC Raw: " + String(io_->getPcRaw()));
        }

        if (ac_)
        {
            Serial.println("Incase Temp: " + String(ac_->getIncaseTemp()));
            Serial.println("Incase Humid: " + String(ac_->getIncaseHumid()));
            Serial.println("AC1 Temp: " + String(ac_->getBeforeAcTemp()));
            Serial.println("AC2 Temp: " + String(ac_->getAfterAcTemp()));
            Serial.println("AC1 Humid: " + String(ac_->getBeforeAcHumid()));
            Serial.println("AC2 Humid: " + String(ac_->getAfterAcHumid()));
        }
        if (io_)
        {
            Serial.println("Water Full: " + String(io_->isWaterFull() ? "YES" : "NO"));
        }

        Serial.println("===================");
    }

    String SerialHandler::getCmdArg(const String &data, int index)
    {
        int found = 0;
        int strIndex[] = {0, -1};
        int maxIndex = data.length() - 1;

        for (int i = 0; i <= maxIndex && found <= index; i++)
        {
            if (data.charAt(i) == ' ' || i == maxIndex)
            {
                found++;
                strIndex[0] = strIndex[1] + 1;
                strIndex[1] = (i == maxIndex) ? i + 1 : i;
            }
        }
        return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
    }
}