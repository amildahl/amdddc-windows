#pragma once
#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>
#include <unordered_map>

#define ENUM_TO_STRING(value) #value;

enum Command {
    detect,
    setvcp,
    unknown
};

struct Settings {
    bool help{ false };
    bool verbose{ false };
    Command command = unknown;
    unsigned int i2c_subaddress{ 0x51 };
    unsigned int input;
    unsigned int monitor;
    unsigned int display;
};

static const std::unordered_map<Command, const char*> command_to_string{
	{detect, "detect"},
	{setvcp, "setvcp"}
};

Settings parse_settings(int, const char**);
void print_help();

#endif // !SETTINGS_H