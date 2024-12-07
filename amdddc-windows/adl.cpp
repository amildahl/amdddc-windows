#include "adl.h"
#include <iostream>
#include <tchar.h>



LPAdapterInfo		lpAdapterInfo = NULL;
LPADLDisplayInfo	lpAdlDisplayInfo = NULL;
ADLPROCS adlprocs = { 0,0,0,0 };

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
            std::cerr << "Error: ADL initialization failed! This app will NOT work!";
            return false;
        }
        // Initialize ADL with second parameter = 1, which means: Get the info for only currently active adapters!
        ADL_Err = adlprocs.ADL_Main_Control_Create(ADL_Main_Memory_Alloc, 1);

    }
    return (ADL_OK == ADL_Err) ? true : false;

}

void FreeADL()
{

    ADL_Main_Memory_Free((void**)&lpAdapterInfo);
    ADL_Main_Memory_Free((void**)&lpAdlDisplayInfo);

    adlprocs.ADL_Main_Control_Destroy();
    FreeLibrary(adlprocs.hModule);
    adlprocs.hModule = NULL;
}
