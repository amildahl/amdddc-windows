#include "settings.h"
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <cstring>
#include <iomanip>

using namespace std;

void print_help() {
    cout << "Usage: amdddc-windows [options] [command]" << endl;
    cout << "Options:" << endl;
    cout << "  --i2c-source-addr <addr>             Set the I2C source address (Default: 0x51; For LG DualUp, use 0x50, which will then use 0xF4 for the side channel command)" << endl;
    cout << "  --verbose, -v                        Enable verbose output" << endl;
    cout << "  --help, -h                           Print this help message" << endl;
    cout << "Commands:" << endl;
    cout << "  detect                               Print the available monitors and displays" << endl;
    cout << "  setvcp <monitor> <display> <input>   Set the VCP command (currently only input switching)" << endl;
    cout << "                                       <input> for LG DualUp: 0xD0 for DP1, 0xD1 for DP2/USB-C, 0x90 for HDMI, 0x91 for HDMI2" << endl;
}

Settings parse_settings(int argc, const char** argv) {
    Settings settings;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--i2c-source-addr") == 0) {
            if (++i < argc) {
                istringstream converter(argv[i]);
                unsigned int value;
                converter >> hex >> value;
                settings.i2c_subaddress = value;
            }
            else
            {
                throw runtime_error{ "missing param after --i2c-source-addr" };
            }
        }
        else if ((strcmp(argv[i], "--verbose") == 0) || (strcmp(argv[i], "-v") == 0)) {
            settings.verbose = true;
        }
        else if ((strcmp(argv[i], "--help") == 0) || (strcmp(argv[i], "-h") == 0)) {
            settings.help = true;
        }
        else if (strcmp(argv[i], command_to_string.at(detect)) == 0) {
            settings.command = detect;
        }
        else if (strcmp(argv[i], command_to_string.at(setvcp)) == 0) {
            if (i + 3 < argc) {
                istringstream converter1(argv[++i]), converter2(argv[++i]), converter3(argv[++i]);
                unsigned int value1, value2, value3;
                converter1 >> value1;
                converter2 >> value2;
                converter3 >> hex >> value3;
                settings.monitor = value1;
                settings.display = value2;
                settings.input = value3;
                settings.command = setvcp;
            }
            else {
                throw runtime_error{ "missing param after setvcp" };
            }
        }
        else {
            throw runtime_error{ "unrecognized command-line option" };
        }
    }

    if (settings.verbose) {
		cerr << "Settings:" << endl;
		cerr << "  i2c_subaddress: " << hex << settings.i2c_subaddress << endl;
		cerr << "  verbose: " << settings.verbose << endl;
		cerr << "  help: " << settings.help << endl;
		cerr << "  command: " << command_to_string.at(settings.command) << endl;
		cerr << "  input: " << hex << settings.input << endl;
		cerr << "  monitor: " << settings.monitor << endl;
		cerr << "  display: " << settings.display << endl;
    }

    return settings;
}
