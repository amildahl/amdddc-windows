### Description

A simple CLI utility to issue the switch input command via DDC to an LG monitor connected to an AMD GPU.

Supports the --i2c-subcommand argument that's missing from alternative Windows DDC utilities, which enables issues input switching on some LG monitors.  Implementing this argument is the entire point of this project.

### Building

Built using VS2022

I may upload the binary, if requested.

### Credits

The ADL bits are based off the ADL sample [here](https://github.com/GPUOpen-LibrariesAndSDKs/display-library/blob/master/Sample/DDCBlockAccess/DDCBlockAccessDlg.cpp)

Commands, channel ids, etc were all helpfully sourced from the [ddcutil wiki](https://github.com/rockowitz/ddcutil/wiki/Switching-input-source-on-LG-monitors)

The DDC spec listed [here](https://boichat.ch/nicolas/ddcci/specs.html) was helpful in determing the i2cset command to issue.

### Reminders

- This utility sends data over the i2c bus.  Use at your own risk.

### AMD ONLY

This utility will only work with an AMD GPU.  See [this project](https://github.com/kaleb422/NVapi-write-value-to-monitor) if you're using an nVidia GPU.  If you're using Intel, you could watch [this issue](https://github.com/rockowitz/ddcutil/issues/100) to see if someone writes a utility for you.
