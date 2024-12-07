#include "settings.h"
#include "adl.h"
#include <iostream>

using namespace std;

#pragma region setvcp command
#define SETWRITESIZE 8
#define SET_VCPCODE_SUBADDRESS 1
#define SET_VCPCODE_OFFSET 4
#define SET_HIGH_OFFSET 5
#define SET_LOW_OFFSET 6
#define SET_CHK_OFFSET 7
#define VCP_CODE_SWITCH_INPUT 0xF4

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
#pragma endregion

#pragma region detect commnad

void print_devices() {
    int iNumberAdapters = 0;
    int iAdapterIndex = 0;
    int iNumberDisplays = 0;
	int iDisplayIndex = 0;
    int ADL_Err = ADL_ERR;

    adlprocs.ADL_Adapter_NumberOfAdapters_Get(&iNumberAdapters);

    if (iNumberAdapters <= 0)
    {
        cerr << "No AMD display devices found!" << endl;
        return;
    }

    lpAdapterInfo = (LPAdapterInfo)malloc(sizeof(AdapterInfo) * iNumberAdapters);
    memset(lpAdapterInfo, '\0', sizeof(AdapterInfo) * iNumberAdapters);

    // Get the AdapterInfo structure for all adapters in the system
    adlprocs.ADL_Adapter_AdapterInfo_Get(lpAdapterInfo, sizeof(AdapterInfo) * iNumberAdapters);

    // Repeat for all available adapters in the system
    for (int i = 0; i < iNumberAdapters; i++) {
        iAdapterIndex = lpAdapterInfo[i].iAdapterIndex;
        ADL_Main_Memory_Free((void**)&lpAdlDisplayInfo);

        ADL_Err = adlprocs.ADL_Display_DisplayInfo_Get(lpAdapterInfo[i].iAdapterIndex, &iNumberDisplays, &lpAdlDisplayInfo, 0);

        cout << "Adapter Index: " << iAdapterIndex << " Adapter Name: " << lpAdapterInfo[i].strAdapterName << endl;

        for (int j = 0; j < iNumberDisplays; j++)
        {
            // For each display, check its status. Use the display only if it's connected AND mapped (iDisplayInfoValue: bit 0 and 1 )
            if ((ADL_DISPLAY_DISPLAYINFO_DISPLAYCONNECTED | ADL_DISPLAY_DISPLAYINFO_DISPLAYMAPPED) !=
                (ADL_DISPLAY_DISPLAYINFO_DISPLAYCONNECTED | ADL_DISPLAY_DISPLAYINFO_DISPLAYMAPPED & lpAdlDisplayInfo[j].iDisplayInfoValue))
                continue;   // Skip the not connected or non-active displays

            // Is the display mapped to this adapter?
            if ( iAdapterIndex != lpAdlDisplayInfo[ j ].displayID.iDisplayLogicalAdapterIndex )
                continue;

            // Preserve the Connected displays in PDL style :-)
            iDisplayIndex = lpAdlDisplayInfo[j].displayID.iDisplayLogicalIndex;

            cout << "\tDisplay Index : " << iDisplayIndex << " Display Name : " << lpAdlDisplayInfo[j].strDisplayName << endl;
        }
    }
}
#pragma endregion

int main(int argc, const char* argv[])
{
    if (!InitADL())
        exit(1);

    Settings settings;

    try {
        settings = parse_settings(argc, argv);
    }
    catch (const runtime_error& e) {
        cerr << "Error: " << e.what() << endl << endl;
        print_help();
        return 1;
    }

    if (settings.help) {
        print_help();
        return 0;
    }

    switch (settings.command) {
    case detect:
        print_devices();
        break;
    case setvcp:
        vSetVcpCommand(settings.i2c_subaddress, VCP_CODE_SWITCH_INPUT, settings.input, settings.monitor, settings.display);
        break;
    default:
        print_help();
    }
}
