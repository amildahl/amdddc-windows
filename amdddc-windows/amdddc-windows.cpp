// amdddc-windows.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <functional>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <sstream>
#include <iomanip>

#define MAX_NUM_DISPLAY_DEVICES             16

using namespace std;

void vSetVcpCommand(unsigned char ucVcp, unsigned int ulVal);

struct Settings {
    bool help{ false };
    bool verbose{ false };
    bool detect{ false };
    unsigned int i2c_subaddress{ 0x51 };
    unsigned int command;
    unsigned int input;
    unsigned int monitor;
    unsigned int display;
};

typedef function<void(Settings&) > ArgHandle;
typedef function<void(Settings&, const unsigned int)> OneArgHandle;
typedef function<void(Settings&, const unsigned int, const unsigned int)> TwoArgHandle;

#define ZERO_ARGS(str, f, v) {str, [](Settings& s) {s.f = v;}}
#define ONE_ARG(str, f, v) {str, [](Settings& s, const unsigned int arg) {s.f = v;}}
#define TWO_ARGS(str, f1, v1, f2, v2) {str, [](Settings& s, const unsigned int arg1, const unsigned int arg2) {s.f1 = v1; s.f2 = v2;}}


const unordered_map<string, ArgHandle> ZeroArgs{
    ZERO_ARGS("--help", help, true),
    ZERO_ARGS("-h",     help, true),

    ZERO_ARGS("--verbose", verbose, true),
    ZERO_ARGS("-v",        verbose, true),

    ZERO_ARGS("detect", detect, true)
};

const unordered_map<string, OneArgHandle> OneArg{
    ONE_ARG("--i2c-source-addr", i2c_subaddress, arg),
    ONE_ARG("-m", monitor, arg),
    ONE_ARG("-d", display, arg),

};

const unordered_map<string, TwoArgHandle> TwoArgs{
    TWO_ARGS("setvcp", command, arg1, input, arg2),  
};

Settings parse_settings(int argc, const char* argv[]) {
    Settings settings;

    for (int i{ 1 }; i < argc; i++) {
        string opt{ argv[i] };

        if (auto j{ ZeroArgs.find(opt) }; j != ZeroArgs.end()) {
            j->second(settings);
        }
        else if (auto k{ OneArg.find(opt) }; k != OneArg.end()) {
            if (++i < argc) {
                istringstream converter(argv[i]);
                unsigned int value;
                converter >> hex >> value;
                k->second(settings, { value });
            }
            else {
                throw runtime_error{ "missing param after " + opt };
            }

            // No, has infile been set yet?
        }
        else if (auto l{ TwoArgs.find(opt) }; l != TwoArgs.end()) {
            if (++i + 1 < argc) {
                istringstream converter1(argv[i]), converter2(argv[++i]);
                unsigned int value1, value2;
                converter1 >> hex >> value1;
                converter2 >> hex >> value2;
                l->second(settings, { value1 }, { value2 });
            }
            else {
                throw runtime_error{ "missing param after " + opt };
            }
        }
        else {
            cerr << "unrecognized command-line option " << opt << endl;
        }
    }

    return settings;
}

int main(int argc, const char* argv[])
{
    Settings settings = parse_settings(argc, argv);

    if (settings.detect) {
        cout << "Detect!";
    }

    if (settings.input != NULL) {
        cout << "Switch! " << settings.command << " " << settings.input;
    }
}
