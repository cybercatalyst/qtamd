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
