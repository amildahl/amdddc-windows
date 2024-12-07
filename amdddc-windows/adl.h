#pragma once
#ifndef ADL_H
#define ADL_H

#include "../adl-sdk/include/adl_defines.h"
#include "../adl-sdk/include/adl_sdk.h"
#include <windows.h>

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
    ADL_MAIN_CONTROL_CREATE				ADL_Main_Control_Create;
    ADL_MAIN_CONTROL_DESTROY			ADL_Main_Control_Destroy;
    ADL_ADAPTER_NUMBEROFADAPTERS_GET	ADL_Adapter_NumberOfAdapters_Get;
    ADL_ADAPTER_ADAPTERINFO_GET			ADL_Adapter_AdapterInfo_Get;
    ADL_DISPLAY_DDCBLOCKACCESSGET       ADL_Display_DDCBlockAccess_Get;
    ADL_DISPLAY_DISPLAYINFO_GET			ADL_Display_DisplayInfo_Get;
    ADL_DISPLAY_EDIDDATA_GET			ADL_Display_EdidData_Get;
} ADLPROCS;

extern LPAdapterInfo	lpAdapterInfo;
extern LPADLDisplayInfo	lpAdlDisplayInfo;

extern ADLPROCS adlprocs;

void* __stdcall ADL_Main_Memory_Alloc(int);
void __stdcall ADL_Main_Memory_Free(void**);
bool InitADL();
void FreeADL();

#endif // !ADL_H