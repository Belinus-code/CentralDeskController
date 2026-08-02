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

        Serial.println("[SerialHandler] Init started. Copying references.");
        io_ = io;
        pc_ = pc;
        ac_ = ac;
        rgb_ = rgb;
        time_ = time;
        network_ = network;
        Serial.println("[SerialHandler] Init finished.");
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

        // --- SET ---
        if (action == "set")
        {
            String anim_name = getCmdArg(args, 1);
            if (anim_name == "")
            {
                Serial.println("Usage: rgb set ANIMATION");
                return;
            }
            if (rgb_->setUserAnimation(anim_name))
            {
                Serial.println("Switched to " + anim_name);
            }
            else
            {
                Serial.println("Animation not found.");
            }
        }

        // --- NEW ---
        else if (action == "new")
        {
            String type = getCmdArg(args, 1); // "static", "blink", "fade"
            String name = getCmdArg(args, 2);

            if (type == "static")
            {
                if (name == "")
                {
                    Serial.println("Error: Format is 'new static NAME COLOR'");
                    return;
                }
                uint32_t color = (uint32_t)strtoul(getCmdArg(args, 3).c_str(), NULL, 16);
                AnimationSetting *set = anim_man->createSettingsStaticColor(color, 255, name);
                anim_man->createAnimation(set);
                delete set;
                Serial.println("Created static: " + name);
            }
            else if (type == "blink")
            {
                if (name == "")
                {
                    Serial.println("Usage: 'new blink NAME COLOR_ON COLOR_OFF TICKS'");
                    return;
                }
                uint32_t c_on = (uint32_t)strtoul(getCmdArg(args, 3).c_str(), NULL, 16);
                uint32_t c_off = (uint32_t)strtoul(getCmdArg(args, 4).c_str(), NULL, 16);
                uint8_t ticks = (uint8_t)getCmdArg(args, 5).toInt();
                AnimationSetting *set = anim_man->createSettingsBlink(c_on, c_off, ticks, 255, name);
                anim_man->createAnimation(set);
                delete set;
                Serial.println("Created blink: " + name);
            }
            else if (type == "fade")
            {
                if (name == "" || name == "help")
                {
                    Serial.println("'new fade NAME PALETTE SPEED DELTA'\nPalette: 0: Rainbow, 1: Party, 2: Ocean, 3: Forest, 4: Heat, 5: Lava, 6: Matrix\nDelta: Width");
                    return;
                }
                uint8_t palette = (uint8_t)getCmdArg(args, 3).toInt();
                uint8_t speed = (uint8_t)getCmdArg(args, 4).toInt();
                uint8_t delta = (uint8_t)getCmdArg(args, 5).toInt();
                AnimationSetting *set = anim_man->createSettingsPalette(palette, speed, delta, 255, name);
                anim_man->createAnimation(set);
                delete set;
                Serial.println("Created fade: " + name);
            }
            else
            {
                Serial.println("'new' can be used to create an animation. \nUsage:\n'new static NAME COLOR'\n'new blink NAME COLOR_ON COLOR_OFF TICKS'\n'new fade NAME PALETTE SPEED DELTA'");
            }
        }

        // --- SETTING ---
        else if (action == "setting")
        {
            String subCmd = getCmdArg(args, 1);
            String animName = getCmdArg(args, 2);

            if (subCmd == "list")
            {
                if (animName == "")
                {
                    Serial.println("Error: Missing Name. Usage: setting list NAME");
                    return;
                }
                IAnimation *anim = anim_man->getAnimationByName(animName);
                if (anim)
                {
                    Serial.println("Available Settings for " + animName + ":");
                    Serial.println(anim->GetAvailableSettings());
                }
                else
                {
                    Serial.println("Animation '" + animName + "' not found.");
                }
            }
            else if (subCmd == "show")
            {
                int index = getCmdArg(args, 3).toInt();
                IAnimation *anim = anim_man->getAnimationByName(animName);
                if (anim)
                {
                    int value = anim->GetSetting(index);
                    Serial.println("Setting " + String(index) + " is: " + String(value));
                }
                else
                {
                    Serial.println("Animation '" + animName + "' not found.");
                }
            }
            else if (subCmd == "set")
            {
                int index = getCmdArg(args, 3).toInt();
                String dataStr = getCmdArg(args, 4);
                if (dataStr == "")
                {
                    Serial.println("Error: Missing Data. Usage: setting set NAME INDEX DATA");
                    return;
                }

                unsigned long value = strtoul(dataStr.c_str(), NULL, 0);
                IAnimation *anim = anim_man->getAnimationByName(animName);

                if (anim)
                {
                    if (anim->UpdateSetting(index, value))
                    {
                        Serial.println("Setting updated.");
                        anim_man->saveAnimationIndex(anim_man->getAnimationIndex(animName));
                        Serial.println("Saved to storage.");
                    }
                    else
                    {
                        Serial.println("Failed to update setting. Invalid Index or Value?");
                    }
                }
                else
                {
                    Serial.println("Animation '" + animName + "' not found.");
                }
            }
            else
            {
                Serial.println("Usage:\nsetting set NAME INDEX DATA\nsetting show NAME INDEX\nsetting list NAME");
            }
        }

        // --- LIST ---
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

        // --- DELETE ---
        else if (action == "delete")
        {
            String anim_name = getCmdArg(args, 1);
            if (anim_name == "")
            {
                Serial.println("Usage: rgb delete ANIMATION");
                return;
            }

            int index = anim_man->getAnimationIndex(anim_name);
            if (index != -1)
            {
                // Falls die Animation gerade läuft, schalten wir auf OFF um Abstürze zu vermeiden
                if (anim_man->getAnimation(index) == rgb_->getUserAnimation())
                {
                    rgb_->setUserAnimation("OFF");
                }
                anim_man->deleteAnimation(index);
                Serial.println("Deleted " + anim_name);
            }
            else
            {
                Serial.println("Animation not found.");
            }
        }

        // --- TOGGLE ---
        else if (action == "toggle")
        {
            rgb_->toggleUserAnimation();
            Serial.println("Toggled Light.");
        }

        // --- BRIGHTNESS ---
        else if (action == "brightness")
        {
            String dir = getCmdArg(args, 1);
            dir.toUpperCase();

            if (dir == "UP" || dir == "+")
                rgb_->adjustBrightness(10);
            else if (dir == "DOWN" || dir == "-")
                rgb_->adjustBrightness(-10);
            else if (dir == "MIN")
                rgb_->setBrightness(10);
            else if (dir == "MAX")
                rgb_->setBrightness(255);
            else
                Serial.println("Usage: brightness UP/DOWN | +/- | MIN/MAX");
        }

        // --- HELP ---
        else if (action == "help")
        {
            Serial.print("help - list of commands\nset - set an Animation\nnew - create new animation\nlist - list all Animations\ntoggle - Turn light on/off\nsetting - change setting of Animation\ndelete - delete Animation\nbrightness - adjust brightness\n");
        }
        else
        {
            Serial.println("Unknown RGB cmd. Try: help");
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
            Serial.println("Key: " + String(io_->getKey() ? "ON" : "OFF"));
            Serial.println("Switch: " + String(io_->getSwitch() ? "ON" : "OFF"));
            Serial.println("Button: " + String(io_->getButton() ? "ON" : "OFF"));
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