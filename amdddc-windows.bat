@echo off
:: Simple batch script to run command with parameters for use with an Elgato Streamdeck
%~dp0\x64\Release\amdddc-windows.exe --i2c-source-addr 0x50 setvcp 5 0 0xD1