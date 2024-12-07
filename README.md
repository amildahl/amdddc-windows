### Description

A simple CLI utility to issue the switch input command via DDC to an LG monitor connected to an AMD GPU.

Supports the --i2c-subcommand argument that's missing from alternative Windows DDC utilities, which enables issues input switching on some LG monitors.  Implementing this argument is the entire point of this project.

This was based off the ADL sample here: https://github.com/GPUOpen-LibrariesAndSDKs/display-library/blob/master/Sample/DDCBlockAccess/DDCBlockAccessDlg.cpp

Built using VS2022
