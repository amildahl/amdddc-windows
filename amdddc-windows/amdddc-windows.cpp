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
#include <windows.h>
#include "../adl-sdk/include/adl_defines.h"
#include "../adl-sdk/include/adl_sdk.h"
#include <synchapi.h>
#include <tchar.h>


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

#define SETWRITESIZE 8
#define SET_VCPCODE_SUBADDRESS 1
#define SET_VCPCODE_OFFSET 4
#define SET_HIGH_OFFSET 5
#define SET_LOW_OFFSET 6
#define SET_CHK_OFFSET 7
#define VCP_CODE_SWITCH_INPUT 0xF4

typedef int (*ADL_MAIN_CONTROL_CREATE)(ADL_MAIN_MALLOC_CALLBACK, int);
typedef int (*ADL_MAIN_CONTROL_DESTROY)();
typedef int (*ADL_ADAPTER_NUMBEROFADAPTERS_GET) (int*);
typedef int (*ADL_ADAPTER_ADAPTERINFO_GET) (LPAdapterInfo, int);
typedef int (*ADL_DISPLAY_DISPLAYINFO_GET) (int, int*, ADLDisplayInfo**, int);
typedef int (*ADL_DISPLAY_DDCBLOCKACCESSGET) (int iAdapterIndex, int iDisplayIndex, int iOption, int iCommandIndex, int iSendMsgLen, char* lpucSendMsgBuf, int* lpulRecvMsgLen, char* lpucRecvMsgBuf);
typedef int (*ADL_DISPLAY_EDIDDATA_GET) (int iAdapterIndex, int iDisplayIndex, ADLDisplayEDIDData* lpEDIDData);


typedef struct _ADLPROCS
{
    HMODULE hModule;
    ADL_MAIN_CONTROL_CREATE						ADL_Main_Control_Create;
    ADL_MAIN_CONTROL_DESTROY						ADL_Main_Control_Destroy;
    ADL_ADAPTER_NUMBEROFADAPTERS_GET	ADL_Adapter_NumberOfAdapters_Get;
    ADL_ADAPTER_ADAPTERINFO_GET				ADL_Adapter_AdapterInfo_Get;
    ADL_DISPLAY_DDCBLOCKACCESSGET          ADL_Display_DDCBlockAccess_Get;
    ADL_DISPLAY_DISPLAYINFO_GET					ADL_Display_DisplayInfo_Get;
    ADL_DISPLAY_EDIDDATA_GET						ADL_Display_EdidData_Get;
} ADLPROCS;

ADLPROCS adlprocs = { 0,0,0,0 };
unsigned char ucSetCommandWrite[SETWRITESIZE] = { 0x6e,0x51,0x84,0x03,0x00,0x00,0x00,0x00 };

int vWriteI2c(char* lpucSendMsgBuf, int iSendMsgLen, int iAdapterIndex, int iDisplayIndex)
{
    int iRev = 0;
    return adlprocs.ADL_Display_DDCBlockAccess_Get(iAdapterIndex, iDisplayIndex, NULL, NULL, iSendMsgLen, lpucSendMsgBuf, &iRev, NULL);
}

void vSetVcpCommand(unsigned int subaddress, unsigned char ucVcp, unsigned int ulVal, int iAdapterIndex, int iDisplayIndex)
{
    unsigned int i;
    unsigned char chk = 0;
    int ADL_Err = ADL_ERR;
    /*
    * Following DDC/CI Spec defined here: https://boichat.ch/nicolas/ddcci/specs.html
    *
    UCHAR ucSetCommandWrite[8] =
    0: 0x6e - I2C address     : 0x37, writing
    1: 0x51 - I2C sub address : Using 0x50 for input switching on LG
    2: 0x84 - For writes, the last 4 bits indicates the number of following bytes, excluding checksum (so, 0x84 is 0b10000100 or 4)
    3: 0x03
    4: 0x00 - Side Channel Code, so 0xF4
    5: 0x00 - 0x00 -- ?? Guessing if the code is > 255
    6: 0x00 - 0xD2 -- Display Code
    7: 0x00 - Checksum using XOR of all preceding bytes, including the first
    */

    /*
    * Display codes:
    * 0x00 - Auto?
    * 0xD0 - DP1, Confirmed
    * 0xD1 - USB-C, Confirmed (It's the DP-2 Alt, but DualUp doesn't have it)
    * 0x90 - HDMI, Confiremed
    * 0x91 - HDMI2, Confirmed
    */

    ucSetCommandWrite[SET_VCPCODE_SUBADDRESS] = subaddress;

    ucSetCommandWrite[SET_VCPCODE_OFFSET] = ucVcp;
    ucSetCommandWrite[SET_LOW_OFFSET] = (char)(ulVal & 0x0ff);
    ucSetCommandWrite[SET_HIGH_OFFSET] = (char)((ulVal >> 8) & 0x0ff);

    for (i = 0; i < SET_CHK_OFFSET; i++)
        chk = chk ^ ucSetCommandWrite[i];

    ucSetCommandWrite[SET_CHK_OFFSET] = chk;
    ADL_Err = vWriteI2c((char*)&ucSetCommandWrite[0], SETWRITESIZE, iAdapterIndex, iDisplayIndex);
    Sleep(5000);
}

void* __stdcall ADL_Main_Memory_Alloc(int iSize)
{
    void* lpBuffer = malloc(iSize);
    return lpBuffer;
}

void __stdcall ADL_Main_Memory_Free(void** lpBuffer)
{
    if (NULL != *lpBuffer)
    {
        free(*lpBuffer);
        *lpBuffer = NULL;
    }
}

LPAdapterInfo			lpAdapterInfo = NULL;
LPADLDisplayInfo	lpAdlDisplayInfo = NULL;
char 							MonitorNames[MAX_NUM_DISPLAY_DEVICES][128];		// Array of Monitor names

bool InitADL()
{
    int	ADL_Err = ADL_ERR;
    if (!adlprocs.hModule)
    {
        adlprocs.hModule = LoadLibrary(_T("atiadlxx.dll"));
        // A 32 bit calling application on 64 bit OS will fail to LoadLIbrary.
        // Try to load the 32 bit library (atiadlxy.dll) instead
        if (adlprocs.hModule == NULL)
            adlprocs.hModule = LoadLibrary(_T("atiadlxy.dll"));

        if (adlprocs.hModule)
        {
            adlprocs.ADL_Main_Control_Create = (ADL_MAIN_CONTROL_CREATE)GetProcAddress(adlprocs.hModule, "ADL_Main_Control_Create");
            adlprocs.ADL_Main_Control_Destroy = (ADL_MAIN_CONTROL_DESTROY)GetProcAddress(adlprocs.hModule, "ADL_Main_Control_Destroy");
            adlprocs.ADL_Adapter_NumberOfAdapters_Get = (ADL_ADAPTER_NUMBEROFADAPTERS_GET)GetProcAddress(adlprocs.hModule, "ADL_Adapter_NumberOfAdapters_Get");
            adlprocs.ADL_Adapter_AdapterInfo_Get = (ADL_ADAPTER_ADAPTERINFO_GET)GetProcAddress(adlprocs.hModule, "ADL_Adapter_AdapterInfo_Get");
            adlprocs.ADL_Display_DisplayInfo_Get = (ADL_DISPLAY_DISPLAYINFO_GET)GetProcAddress(adlprocs.hModule, "ADL_Display_DisplayInfo_Get");
            adlprocs.ADL_Display_DDCBlockAccess_Get = (ADL_DISPLAY_DDCBLOCKACCESSGET)GetProcAddress(adlprocs.hModule, "ADL_Display_DDCBlockAccess_Get");
            adlprocs.ADL_Display_EdidData_Get = (ADL_DISPLAY_EDIDDATA_GET)GetProcAddress(adlprocs.hModule, "ADL_Display_EdidData_Get");
        }

        if (adlprocs.hModule == NULL ||
            adlprocs.ADL_Main_Control_Create == NULL ||
            adlprocs.ADL_Main_Control_Destroy == NULL ||
            adlprocs.ADL_Adapter_NumberOfAdapters_Get == NULL ||
            adlprocs.ADL_Adapter_AdapterInfo_Get == NULL ||
            adlprocs.ADL_Display_DisplayInfo_Get == NULL ||
            adlprocs.ADL_Display_DDCBlockAccess_Get == NULL ||
            adlprocs.ADL_Display_EdidData_Get == NULL)
        {
            cerr << "Error: ADL initialization failed! This app will NOT work!";
            return false;
        }
        // Initialize ADL with second parameter = 1, which means: Get the info for only currently active adapters!
        ADL_Err = adlprocs.ADL_Main_Control_Create(ADL_Main_Memory_Alloc, 1);

    }
    return (ADL_OK == ADL_Err) ? true : false;

}

// Function:
// void FreeADL
// Purpose:
// free the ADL Module
// Input: NONE
// Output: VOID
void FreeADL()
{

    ADL_Main_Memory_Free((void**)&lpAdapterInfo);
    ADL_Main_Memory_Free((void**)&lpAdlDisplayInfo);

    adlprocs.ADL_Main_Control_Destroy();
    FreeLibrary(adlprocs.hModule);
    adlprocs.hModule = NULL;
}



int main(int argc, const char* argv[])
{
    if (!InitADL())
        exit(1);

    vSetVcpCommand(0x50, 0xF4, 0xD1, 7, 0);

    Settings settings = parse_settings(argc, argv);

    // TODO: Implement detect
    if (settings.detect) {
        cout << "Detect!";
    }
    
    if (settings.input != 0) {
        cout << "Switch! " << settings.command << " " << settings.input;
    }
}
