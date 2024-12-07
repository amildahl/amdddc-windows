> [!IMPORTANT]
> This utility will **_only_** work with an AMD GPU!
> 
> If you're using an Nvidia GPU, see [this project](https://github.com/kaleb422/NVapi-write-value-to-monitor) for using NVapi.
> 
> If you're using Intel, you could watch [this issue](https://github.com/rockowitz/ddcutil/issues/100) to see if someone writes a utility.

> [!Warning]
> This utility sends data over the i2c bus.  Use at your own risk.

### Description

A simple CLI utility to issue the switch input command via DDC to a monitor connected to an AMD GPU.

Supports the --i2c-subcommand argument that's missing from alternative Windows DDC utilities, which enables issues input switching on some LG monitors.  Implementing this argument is the entire point of this project.

### Building

Built using VS2022

I may upload the binary, if requested.

### Usage

```
Usage: amdddc-windows [options] [command]
Options:
  --i2c-source-addr <addr>             Set the I2C source address (Default: 0x51; For LG DualUp, use 0x50, which will then use 0xF4 for the side channel command)
  --verbose, -v                        Enable verbose output
  --help, -h                           Print this help message
Commands:
  detect                               Print the available monitors and displays
  setvcp <monitor> <display> <input>   Set the VCP command (currently only input switching)
                                       <input> for LG DualUp: 0xD0 for DP1, 0xD1 for DP2/USB-C, 0x90 for HDMI, 0x91 for HDMI2
```

Example output from `detect`:
```
> .\amdddc-windows.exe detect
Adapter Index: 0 Adapter Name: AMD Radeon(TM) Graphics
Adapter Index: 1 Adapter Name: AMD Radeon(TM) Graphics
Adapter Index: 2 Adapter Name: AMD Radeon(TM) Graphics
Adapter Index: 3 Adapter Name: AMD Radeon(TM) Graphics
Adapter Index: 4 Adapter Name: AMD Radeon(TM) Graphics
Adapter Index: 5 Adapter Name: AMD Radeon RX 7900 XTX
        Display Index : 0 Display Name : LG SDQHD
Adapter Index: 6 Adapter Name: AMD Radeon RX 7900 XTX
        Display Index : 4 Display Name : LG SDQHD
Adapter Index: 7 Adapter Name: AMD Radeon RX 7900 XTX
        Display Index : 8 Display Name : LG TV SSCR2
Adapter Index: 8 Adapter Name: AMD Radeon RX 7900 XTX
Adapter Index: 9 Adapter Name: AMD Radeon RX 7900 XTX
```

Example command to change input 0 (LG SDQHD) on adapter 5 to the USB-C input:
```
> .\amdddc-windows.exe --i2c-source-addr 0x50 setvcp 5 0 0xD1
```

### Credits

The ADL bits are based off the ADL sample [here](https://github.com/GPUOpen-LibrariesAndSDKs/display-library/blob/master/Sample/DDCBlockAccess/DDCBlockAccessDlg.cpp)

Commands, channel ids, etc were all helpfully sourced from the [ddcutil wiki](https://github.com/rockowitz/ddcutil/wiki/Switching-input-source-on-LG-monitors)

The DDC spec listed [here](https://boichat.ch/nicolas/ddcci/specs.html) was helpful in determing the i2cset command to issue.

### Reminders




