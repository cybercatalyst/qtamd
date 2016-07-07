///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//    This file is part of QtAMD.                                            //
//    Copyright (C) 2015-2016 Jacob Dawid, jacob@omg-it.works                //
//                                                                           //
//    QtAMD is free software: you can redistribute it and/or modify          //
//    it under the terms of the GNU Affero General Public License as         //
//    published by the Free Software Foundation, either version 3 of the     //
//    License, or (at your option) any later version.                        //
//                                                                           //
//    QtAMD is distributed in the hope that it will be useful,               //
//    but WITHOUT ANY WARRANTY; without even the implied warranty of         //
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          //
//    GNU Affero General Public License for more details.                    //
//                                                                           //
//    You should have received a copy of the GNU General Public License      //
//    along with QtAMD. If not, see <http://www.gnu.org/licenses/>.          //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "amdoverdrive.h"

#include <stdio.h>

#define AMDVENDORID             (1002)
#define ADL_WARNING_NO_DATA      -100

#include "adlfunctionpointers.h"

void* __stdcall ADL_Main_Memory_Alloc(int iSize) {
    void* lpBuffer = malloc(iSize);
    return lpBuffer;
}

void __stdcall ADL_Main_Memory_Free(void** lpBuffer) {
    if (NULL != *lpBuffer) {
        free(*lpBuffer);
        *lpBuffer = NULL;
    }
}

AMDOverdrive::AMDOverdrive() {
#if defined Q_OS_LINUX
    _dll = dlopen("libatiadlxx.so", RTLD_LAZY | RTLD_GLOBAL);
#else
    _dll = LoadLibrary("atiadlxx.dll");
    if(_dll == NULL)
        // A 32 bit calling application on 64 bit OS will fail to LoadLibrary.
        // Try to load the 32 bit library (atiadlxy.dll) instead
        _dll = LoadLibrary("atiadlxy.dll");
#endif

    if(_dll) {
        ADL(_dll, ADL_MAIN_CONTROL_CREATE, ADL_Main_Control_Create)
        if(ADL_Main_Control_Create) {
            int returnCode = ADL_Main_Control_Create(ADL_Main_Memory_Alloc, 1);
            if(returnCode == ADL_OK) {

            } else {
                functionCallFailed("ADL_Main_Control_Create", returnCode);
            }
        } else {
            functionNotAvailable("ADL_Main_Control_Create");
        }
    } else {
        qDebug() << "AMDOverdrive: libatiadlxx.so/atiadlxx.dll/atiadlxy.dll not found.";
    }
}

int AMDOverdrive::numberOfAdapters() {
    if(!_dll) { return -1; }

    ADL(_dll, ADL_ADAPTER_NUMBEROFADAPTERS_GET, ADL_Adapter_NumberOfAdapters_Get)

    if(ADL_Adapter_NumberOfAdapters_Get) {
        int n, returnCode;
        returnCode = ADL_Adapter_NumberOfAdapters_Get(&n);
        if(returnCode == ADL_OK) {
            return n;
        } else {
            functionCallFailed("ADL_Adapter_NumberOfAdapters_Get", returnCode);
        }
    } else {
        functionNotAvailable("ADL_Adapter_NumberOfAdapters_Get");
    }

    return -1;
}

QList<AdapterInfo> AMDOverdrive::adapterInfo() {
    QList<AdapterInfo> infoList;

    if(_dll) {
        ADL(_dll, ADL_ADAPTER_ADAPTERINFO_GET, ADL_Adapter_AdapterInfo_Get)
        if(ADL_Adapter_AdapterInfo_Get) {
            int n = numberOfAdapters();
            if(n > 0) {
                int lpAdapterInfoSize = sizeof(AdapterInfo) * n;

                LPAdapterInfo lpAdapterInfo = (LPAdapterInfo)malloc(lpAdapterInfoSize);
                memset(lpAdapterInfo,'\0', lpAdapterInfoSize);

                int returnCode = ADL_Adapter_AdapterInfo_Get(lpAdapterInfo, lpAdapterInfoSize);
                if(returnCode == ADL_OK) {
                    for(int i = 0; i < n; i++) {
                        infoList.append(lpAdapterInfo[i]);
                    }
                } else {
                    functionCallFailed("ADL_Adapter_AdapterInfo_Get", returnCode);
                }

                free(lpAdapterInfo);
            }
        } else {
            functionNotAvailable("ADL_Adapter_AdapterInfo_Get");
        }
    }

    return infoList;
}

bool AMDOverdrive::isAdapterActive(AdapterInfo adapterInfo) {
    bool isActive = false;

    if(_dll) {
        ADL(_dll, ADL_ADAPTER_ACTIVE_GET, ADL_Adapter_Active_Get)
        if(ADL_Adapter_Active_Get) {
            int adapterActive = 0;
            int returnCode = ADL_Adapter_Active_Get(adapterInfo.iAdapterIndex, &adapterActive);
            if(returnCode == ADL_OK) {
                isActive = adapterActive && adapterInfo.iVendorID == AMDVENDORID;
            } else {
                functionCallFailed("ADL_Adapter_Active_Get", returnCode);
            }
        } else {
            functionNotAvailable("ADL_Adapter_Active_Get");
        }
    }

    return isActive;
}

AMDOverdrive::Capabilities AMDOverdrive::capabilities(int adapterIndex) {
    Capabilities caps;

    if(_dll) {
        ADL(_dll, ADL_OVERDRIVE_CAPS, ADL_Overdrive_Caps)
        if(ADL_Overdrive_Caps) {
            int returnCode = ADL_Overdrive_Caps(adapterIndex, &caps.supported, &caps.enabled, &caps.version);
            if(returnCode != ADL_OK) {
                functionCallFailed("ADL_Overdrive_Caps", returnCode);
            }
        } else {
            functionNotAvailable("ADL_Overdrive_Caps");
        }
    }

    return caps;
}

bool AMDOverdrive::isPowerControlSupported5(int adapterIndex) {
    int isSupported = 0;

    if(_dll) {
        ADL(_dll, ADL_OVERDRIVE5_POWERCONTROL_CAPS, ADL_Overdrive5_PowerControl_Caps)
        if(ADL_Overdrive5_PowerControl_Caps) {
            int returnCode = ADL_Overdrive5_PowerControl_Caps(adapterIndex, &isSupported);
            if(returnCode != ADL_OK) {
                functionCallFailed("ADL_Overdrive5_PowerControl_Caps", returnCode);
            }
        } else {
            functionNotAvailable("ADL_Overdrive5_PowerControl_Caps");
        }
    }

    return (bool)isSupported;
}

ADLPowerControlInfo AMDOverdrive::powerControlInfo5(int adapterIndex) {
    ADLPowerControlInfo info = {0, 0, 0};

    if(_dll) {
        if(isPowerControlSupported5(adapterIndex)) {
            ADL(_dll, ADL_OVERDRIVE5_POWERCONTROLINFO_GET, ADL_Overdrive5_PowerControlInfo_Get)
            if(ADL_Overdrive5_PowerControlInfo_Get) {
                int returnCode = ADL_Overdrive5_PowerControlInfo_Get(adapterIndex, &info);
                if(returnCode != ADL_OK) {
                    functionCallFailed("ADL_Overdrive5_PowerControlInfo_Get", returnCode);
                }
            } else {
                functionNotAvailable("ADL_Overdrive5_PowerControlInfo_Get");
            }
        } else {
            qDebug() << "Cannot retrieve power control info: power control not supported on adapter " << adapterIndex;
        }
    }

    return info;
}

int AMDOverdrive::powerControlGetCurrent5(int adapterIndex) {
    int powerControlCurrent, powerControlDefault;

    if(_dll) {
        if(isPowerControlSupported5(adapterIndex)) {
            ADL(_dll, ADL_OVERDRIVE5_POWERCONTROL_GET, ADL_Overdrive5_PowerControl_Get)
            if(ADL_Overdrive5_PowerControl_Get) {
                int returnCode = ADL_Overdrive5_PowerControl_Get(adapterIndex, &powerControlCurrent, &powerControlDefault);
                if(returnCode != ADL_OK) {
                    functionCallFailed("ADL_Overdrive5_PowerControl_Get", returnCode);
                }
            } else {
                functionNotAvailable("ADL_Overdrive5_PowerControl_Get");
            }
        } else {
            qDebug() << "Cannot retrieve power control info: power control not supported on adapter " << adapterIndex;
        }
    }

    return powerControlCurrent;
}

int AMDOverdrive::powerControlGetDefault5(int adapterIndex) {
    int powerControlCurrent, powerControlDefault;

    if(_dll) {
        if(isPowerControlSupported5(adapterIndex)) {
            ADL(_dll, ADL_OVERDRIVE5_POWERCONTROL_GET, ADL_Overdrive5_PowerControl_Get)
            if(ADL_Overdrive5_PowerControl_Get) {
                int returnCode = ADL_Overdrive5_PowerControl_Get(adapterIndex, &powerControlCurrent, &powerControlDefault);
                if(returnCode != ADL_OK) {
                    functionCallFailed("ADL_Overdrive5_PowerControl_Get", returnCode);
                }
            } else {
                functionNotAvailable("ADL_Overdrive5_PowerControl_Get");
            }
        } else {
            qDebug() << "Cannot retrieve power control info: power control not supported on adapter " << adapterIndex;
        }
    }

    return powerControlDefault;
}

bool AMDOverdrive::powerControlSet5(int adapterIndex, int value) {
    if(_dll) {
        if(isPowerControlSupported5(adapterIndex)) {
            ADL(_dll, ADL_OVERDRIVE5_POWERCONTROL_SET, ADL_Overdrive5_PowerControl_Set)
            if(ADL_Overdrive5_PowerControl_Set) {
                int returnCode = ADL_Overdrive5_PowerControl_Set(adapterIndex, value);
                if(returnCode == ADL_OK) {
                    return true;
                } else {
                    functionCallFailed("ADL_Overdrive5_PowerControl_Set", returnCode);
                }
            } else {
                functionNotAvailable("ADL_Overdrive5_PowerControl_Set");
            }
        } else {
            qDebug() << "Cannot set power control value: power control not supported on adapter " << adapterIndex;
        }
    }
    return false;
}


