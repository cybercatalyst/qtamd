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

bool AMDOverdrive::isPowerControlSupported(int adapterIndex) {
    int isSupported = 0;

    if(_dll) {
        Capabilities caps = capabilities(adapterIndex);
        int returnCode = 0;
        ADL(_dll, ADL_OVERDRIVE5_POWERCONTROL_CAPS, ADL_Overdrive5_PowerControl_Caps)
        ADL(_dll, ADL_OVERDRIVE6_POWERCONTROL_CAPS, ADL_Overdrive6_PowerControl_Caps)

        switch (caps.version) {
        case 5:
            if(ADL_Overdrive5_PowerControl_Caps) {
                returnCode = ADL_Overdrive5_PowerControl_Caps(adapterIndex, &isSupported);
                if(returnCode != ADL_OK) {
                    functionCallFailed("ADL_Overdrive5_PowerControl_Caps", returnCode);
                }
            } else {
                functionNotAvailable("ADL_Overdrive5_PowerControl_Caps");
            }
            break;
        case 6:
            if(ADL_Overdrive6_PowerControl_Caps) {
                returnCode = ADL_Overdrive6_PowerControl_Caps(adapterIndex, &isSupported);
                if(returnCode != ADL_OK) {
                    functionCallFailed("ADL_Overdrive6_PowerControl_Caps", returnCode);
                }
            } else {
                functionNotAvailable("ADL_Overdrive6_PowerControl_Caps");
            }
            break;
        default:
            qDebug() << "Overdrive version" << caps.version << "is not supported.";
            break;
        }
    }

    return (bool)isSupported;
}

ADLPowerControlInfo AMDOverdrive::powerControlInfo(int adapterIndex) {
    ADLPowerControlInfo info = {0, 0, 0};
    ADLOD6PowerControlInfo info6 = {0, 0, 0, 0, 0};

    if(_dll) {
        if(isPowerControlSupported(adapterIndex)) {
            Capabilities caps = capabilities(adapterIndex);
            int returnCode = 0;
            ADL(_dll, ADL_OVERDRIVE5_POWERCONTROLINFO_GET, ADL_Overdrive5_PowerControlInfo_Get)
            ADL(_dll, ADL_OVERDRIVE6_POWERCONTROLINFO_GET, ADL_Overdrive6_PowerControlInfo_Get)

            switch (caps.version) {
            case 5:
                if(ADL_Overdrive5_PowerControlInfo_Get) {
                    returnCode = ADL_Overdrive5_PowerControlInfo_Get(adapterIndex, &info);
                    if(returnCode != ADL_OK) {
                        functionCallFailed("ADL_Overdrive5_PowerControlInfo_Get", returnCode);
                    }
                } else {
                    functionNotAvailable("ADL_Overdrive5_PowerControlInfo_Get");
                }
                break;
            case 6:
                if(ADL_Overdrive6_PowerControlInfo_Get) {
                    returnCode = ADL_Overdrive6_PowerControlInfo_Get(adapterIndex, &info6);
                    info.iMinValue = info6.iMinValue;
                    info.iMaxValue = info6.iMaxValue;
                    info.iStepValue = info6.iStepValue;
                    if(returnCode != ADL_OK) {
                        functionCallFailed("ADL_Overdrive6_PowerControlInfo_Get", returnCode);
                    }
                } else {
                    functionNotAvailable("ADL_Overdrive6_PowerControlInfo_Get");
                }
                break;
            default:
                qDebug() << "Overdrive version" << caps.version << "is not supported.";
                break;
            }
        } else {
            qDebug() << "Cannot retrieve power control info: power control not supported on adapter " << adapterIndex;
        }
    }

    return info;
}

int AMDOverdrive::powerControlGetCurrent(int adapterIndex) {
    int powerControlCurrent, powerControlDefault;

    if(_dll) {
        if(isPowerControlSupported(adapterIndex)) {
            Capabilities caps = capabilities(adapterIndex);
            int returnCode = 0;
            ADL(_dll, ADL_OVERDRIVE5_POWERCONTROL_GET, ADL_Overdrive5_PowerControl_Get)
            ADL(_dll, ADL_OVERDRIVE6_POWERCONTROL_GET, ADL_Overdrive6_PowerControl_Get)

            switch (caps.version) {
            case 5:
                if(ADL_Overdrive5_PowerControl_Get) {
                    returnCode = ADL_Overdrive5_PowerControl_Get(adapterIndex, &powerControlCurrent, &powerControlDefault);
                    if(returnCode != ADL_OK) {
                        functionCallFailed("ADL_Overdrive5_PowerControl_Get", returnCode);
                    }
                } else {
                    functionNotAvailable("ADL_Overdrive5_PowerControl_Get");
                }
                break;
            case 6:
                if(ADL_Overdrive6_PowerControl_Get) {
                    returnCode = ADL_Overdrive6_PowerControl_Get(adapterIndex, &powerControlCurrent, &powerControlDefault);
                    if(returnCode != ADL_OK) {
                        functionCallFailed("ADL_Overdrive6_PowerControl_Get", returnCode);
                    }
                } else {
                    functionNotAvailable("ADL_Overdrive6_PowerControl_Get");
                }
                break;
            default:
                qDebug() << "Overdrive version" << caps.version << "is not supported.";
                break;
            }
        } else {
            qDebug() << "Cannot retrieve power control info: power control not supported on adapter " << adapterIndex;
        }
    }

    return powerControlCurrent;
}

int AMDOverdrive::powerControlGetDefault(int adapterIndex) {
    int powerControlCurrent, powerControlDefault;

    if(_dll) {
        if(isPowerControlSupported(adapterIndex)) {
            Capabilities caps = capabilities(adapterIndex);
            int returnCode = 0;
            ADL(_dll, ADL_OVERDRIVE5_POWERCONTROL_GET, ADL_Overdrive5_PowerControl_Get)
            ADL(_dll, ADL_OVERDRIVE6_POWERCONTROL_GET, ADL_Overdrive6_PowerControl_Get)

            switch (caps.version) {
            case 5:
                if(ADL_Overdrive5_PowerControl_Get) {
                    returnCode = ADL_Overdrive5_PowerControl_Get(adapterIndex, &powerControlCurrent, &powerControlDefault);
                    if(returnCode != ADL_OK) {
                        functionCallFailed("ADL_Overdrive5_PowerControl_Get", returnCode);
                    }
                } else {
                    functionNotAvailable("ADL_Overdrive5_PowerControl_Get");
                }
                break;
            case 6:
                if(ADL_Overdrive6_PowerControl_Get) {
                    returnCode = ADL_Overdrive6_PowerControl_Get(adapterIndex, &powerControlCurrent, &powerControlDefault);
                    if(returnCode != ADL_OK) {
                        functionCallFailed("ADL_Overdrive6_PowerControl_Get", returnCode);
                    }
                } else {
                    functionNotAvailable("ADL_Overdrive6_PowerControl_Get");
                }
                break;
            default:
                qDebug() << "Overdrive version" << caps.version << "is not supported.";
                break;
            }
        } else {
            qDebug() << "Cannot retrieve power control info: power control not supported on adapter " << adapterIndex;
        }
    }

    return powerControlDefault;
}

bool AMDOverdrive::powerControlSet(int adapterIndex, int value) {
    if(_dll) {
        if(isPowerControlSupported(adapterIndex)) {
            Capabilities caps = capabilities(adapterIndex);
            int returnCode = 0;
            ADL(_dll, ADL_OVERDRIVE5_POWERCONTROL_SET, ADL_Overdrive5_PowerControl_Set)
            ADL(_dll, ADL_OVERDRIVE6_POWERCONTROL_SET, ADL_Overdrive6_PowerControl_Set)

            switch (caps.version) {
            case 5:
                if(ADL_Overdrive5_PowerControl_Set) {
                    returnCode = ADL_Overdrive5_PowerControl_Set(adapterIndex, value);
                    if(returnCode == ADL_OK) {
                        return true;
                    } else {
                        functionCallFailed("ADL_Overdrive5_PowerControl_Set", returnCode);
                    }
                } else {
                    functionNotAvailable("ADL_Overdrive5_PowerControl_Set");
                }
                break;
            case 6:
                if(ADL_Overdrive6_PowerControl_Set) {
                    returnCode = ADL_Overdrive6_PowerControl_Set(adapterIndex, value);
                    if(returnCode == ADL_OK) {
                        return true;
                    } else {
                        functionCallFailed("ADL_Overdrive6_PowerControl_Set", returnCode);
                    }
                } else {
                    functionNotAvailable("ADL_Overdrive6_PowerControl_Set");
                }
                break;
            default:
                qDebug() << "Overdrive version" << caps.version << "is not supported.";
                break;
            }
        } else {
            qDebug() << "Cannot set power control value: power control not supported on adapter " << adapterIndex;
        }
    }
    return false;
}

ADLODParameters AMDOverdrive::overdriveParameters(int adapterIndex) {
    ADLODParameters overdriveParameters = {0, 0, 0, 0, 0, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    overdriveParameters.iSize = sizeof(ADLODParameters);

    if(_dll) {
        ADL(_dll, ADL_OVERDRIVE5_ODPARAMETERS_GET, ADL_Overdrive5_ODParameters_Get)
        if(ADL_Overdrive5_ODParameters_Get) {
            int returnCode = ADL_Overdrive5_ODParameters_Get(adapterIndex, &overdriveParameters);
            if(returnCode != ADL_OK) {
                functionCallFailed("ADL_Overdrive5_ODParameters_Get", returnCode);
            }
        } else {
            functionNotAvailable("ADL_Overdrive5_ODParameters_Get");
        }
    }

    return overdriveParameters;
}

ADLPMActivity AMDOverdrive::currentActivity(int adapterIndex) {
    ADLPMActivity activity = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    activity.iSize = sizeof(ADLPMActivity);

    if(_dll) {
        ADL(_dll, ADL_OVERDRIVE5_CURRENTACTIVITY_GET, ADL_Overdrive5_CurrentActivity_Get)
        if(ADL_Overdrive5_CurrentActivity_Get) {
            int returnCode = ADL_Overdrive5_CurrentActivity_Get(adapterIndex, &activity);
            if(returnCode != ADL_OK) {
                functionCallFailed("ADL_Overdrive5_CurrentActivity_Get", returnCode);
            }
        } else {
            functionNotAvailable("ADL_Overdrive5_CurrentActivity_Get");
        }
    }

    return activity;
}

QList<AMDOverdrive::PerformanceLevelInfo> AMDOverdrive::performanceLevels(int adapterIndex) {
    QList<PerformanceLevelInfo> levels;

    if(_dll) {
        ADLODParameters parameters = overdriveParameters(adapterIndex);
        int n = parameters.iNumberOfPerformanceLevels;
        if(n > 0) {
            int size = sizeof(ADLODPerformanceLevels) + sizeof(ADLODPerformanceLevel) * (n - 1);
            void* defaultLevelsBuffer = malloc(size);
            memset(defaultLevelsBuffer, 0, size);
            void* currentLevelsBuffer = malloc(size);
            memset(currentLevelsBuffer, 0, size);

            ADLODPerformanceLevels* pDefaultPerformanceLevels = (ADLODPerformanceLevels*)defaultLevelsBuffer;
            pDefaultPerformanceLevels->iSize = size;
            ADLODPerformanceLevels* pCurrentPerformanceLevels = (ADLODPerformanceLevels*)currentLevelsBuffer;
            pCurrentPerformanceLevels->iSize = size;

            ADL(_dll, ADL_OVERDRIVE5_ODPERFORMANCELEVELS_GET, ADL_Overdrive5_ODPerformanceLevels_Get)
            if(ADL_Overdrive5_ODPerformanceLevels_Get) {
                int returnCodeCurrent = ADL_Overdrive5_ODPerformanceLevels_Get(adapterIndex, 0, pCurrentPerformanceLevels);
                int returnCodeDefault = ADL_Overdrive5_ODPerformanceLevels_Get(adapterIndex, 1, pDefaultPerformanceLevels);
                if(returnCodeDefault == ADL_OK && returnCodeCurrent == ADL_OK) {
                    PerformanceLevelInfo info;
                    for (int i = 0; i < n; i++) {
                        info.stock = pDefaultPerformanceLevels->aLevels[i];
                        info.current = pCurrentPerformanceLevels->aLevels[i];
                        levels.append(info);
                    }
                } else {
                    functionCallFailed("ADL_Overdrive5_ODPerformanceLevels_Get", returnCodeDefault);
                }
            } else {
                functionNotAvailable("ADL_Overdrive5_ODPerformanceLevels_Get");
            }

            free(defaultLevelsBuffer);
            free(currentLevelsBuffer);
        }
    }

    return levels;
}

QList<ADLThermalControllerInfo> AMDOverdrive::thermalControllersInfo(int adapterIndex) {
    QList<ADLThermalControllerInfo> info;

    if(_dll) {
        ADL(_dll, ADL_OVERDRIVE5_THERMALDEVICES_ENUM, ADL_Overdrive5_ThermalDevices_Enum)
        if(ADL_Overdrive5_ThermalDevices_Enum) {
            // We're probing up to ten thermal devices.
            for(int i = 0; i < 10; i++) {
                ADLThermalControllerInfo thermalControllerInfo = {0, 0, 0, 0};
                thermalControllerInfo.iSize = sizeof(ADLThermalControllerInfo);
                int returnCode = ADL_Overdrive5_ThermalDevices_Enum(adapterIndex, i, &thermalControllerInfo);
                // If we don't get any more data, bail out.
                if(returnCode == ADL_WARNING_NO_DATA) {
                    break;
                }
                // Otherwise, collect info.
                if(returnCode == ADL_OK) {
                    info.append(thermalControllerInfo);
                } else {
                    functionCallFailed("ADL_Overdrive5_ThermalDevices_Enum", returnCode);
                }
            }

        } else {
            functionNotAvailable("ADL_Overdrive5_ThermalDevices_Enum");
        }
    }

    return info;
}

int AMDOverdrive::temperatureMillidegreesCelsius(int adapterIndex, int thermalControllerIndex) {
    ADLTemperature temperature = {0, 0};
    temperature.iSize = sizeof(ADLTemperature);

    if(_dll) {
        ADL(_dll, ADL_OVERDRIVE5_TEMPERATURE_GET, ADL_Overdrive5_Temperature_Get)
        if(ADL_Overdrive5_Temperature_Get) {
            int returnCode = ADL_Overdrive5_Temperature_Get(adapterIndex, thermalControllerIndex, &temperature);
            if(returnCode != ADL_OK) {
                functionCallFailed("ADL_Overdrive5_Temperature_Get", returnCode);
            }
        } else {
            functionNotAvailable("ADL_Overdrive5_Temperature_Get");
        }
    }

    return temperature.iTemperature;
}

ADLFanSpeedInfo AMDOverdrive::fanSpeedInfo(int adapterIndex, int thermalControllerIndex) {
    ADLFanSpeedInfo fanSpeedInfo = {0, 0, 0, 0, 0, 0};
    fanSpeedInfo.iSize = sizeof(ADLFanSpeedInfo);

    if(_dll) {
        ADL(_dll, ADL_OVERDRIVE5_FANSPEEDINFO_GET, ADL_Overdrive5_FanSpeedInfo_Get)
        if(ADL_Overdrive5_FanSpeedInfo_Get) {
            int returnCode = ADL_Overdrive5_FanSpeedInfo_Get(adapterIndex, thermalControllerIndex, &fanSpeedInfo);
            if(returnCode != ADL_OK) {
                functionCallFailed("ADL_Overdrive5_FanSpeedInfo_Get", returnCode);
            }
        } else {
            functionNotAvailable("ADL_Overdrive5_FanSpeedInfo_Get");
        }
    }

    return fanSpeedInfo;
}

bool AMDOverdrive::fanSupportsPercentRead(ADLFanSpeedInfo fanSpeedInfo) {
    return fanSpeedInfo.iFlags & ADL_DL_FANCTRL_SUPPORTS_PERCENT_READ;
}

bool AMDOverdrive::fanSupportsRpmRead(ADLFanSpeedInfo fanSpeedInfo) {
    return fanSpeedInfo.iFlags & ADL_DL_FANCTRL_SUPPORTS_RPM_READ;
}

bool AMDOverdrive::fanSupportsPercentWrite(ADLFanSpeedInfo fanSpeedInfo) {
    return fanSpeedInfo.iFlags & ADL_DL_FANCTRL_SUPPORTS_PERCENT_WRITE;
}

bool AMDOverdrive::fanSupportsRpmWrite(ADLFanSpeedInfo fanSpeedInfo) {
    return fanSpeedInfo.iFlags & ADL_DL_FANCTRL_SUPPORTS_RPM_WRITE;
}

ADLFanSpeedValue AMDOverdrive::fanSpeedValue(int adapterIndex, int thermalControllerIndex, FanSpeedValueType type) {
    ADLFanSpeedValue fanSpeedValue = {0, 0, 0, 0};
    fanSpeedValue.iSize = sizeof(ADLFanSpeedValue);
    fanSpeedValue.iSpeedType = (type == Rpm) ? ADL_DL_FANCTRL_SPEED_TYPE_RPM : ADL_DL_FANCTRL_SPEED_TYPE_PERCENT;

    if(_dll) {
        ADL(_dll, ADL_OVERDRIVE5_FANSPEED_GET, ADL_Overdrive5_FanSpeed_Get)
        if(ADL_Overdrive5_FanSpeed_Get) {
            int returnCode = ADL_Overdrive5_FanSpeed_Get(adapterIndex, thermalControllerIndex, &fanSpeedValue);
            if(returnCode != ADL_OK) {
                functionCallFailed("ADL_Overdrive5_FanSpeed_Get", returnCode);
            }
        } else {
            functionNotAvailable("ADL_Overdrive5_FanSpeed_Get");
        }
    }

    return fanSpeedValue;
}

bool AMDOverdrive::setFanSpeedValue(int adapterIndex, int thermalControllerIndex, FanSpeedValueType type, int value) {
    ADLFanSpeedValue fanSpeedValue = {0, 0, 0, 0};
    fanSpeedValue.iSize = sizeof(ADLFanSpeedValue);
    fanSpeedValue.iSpeedType = (type == Rpm) ? ADL_DL_FANCTRL_SPEED_TYPE_RPM : ADL_DL_FANCTRL_SPEED_TYPE_PERCENT;
    fanSpeedValue.iFanSpeed = value;
    bool success = false;

    if(_dll) {
        ADL(_dll, ADL_OVERDRIVE5_FANSPEED_SET, ADL_Overdrive5_FanSpeed_Set)
        if(ADL_Overdrive5_FanSpeed_Set) {
            int returnCode = ADL_Overdrive5_FanSpeed_Set(adapterIndex, thermalControllerIndex, &fanSpeedValue);
            if(returnCode == ADL_OK) {
                success = true;
            } else {
                functionCallFailed("ADL_Overdrive5_FanSpeed_Set", returnCode);
            }
        } else {
            functionNotAvailable("ADL_Overdrive5_FanSpeed_Set");
        }
    }

    return success;
}

bool AMDOverdrive::setFanSpeedToDefault(int adapterIndex, int thermalControllerIndex) {
    bool success = false;

    if(_dll) {
        ADL(_dll, ADL_OVERDRIVE5_FANSPEEDTODEFAULT_SET, ADL_Overdrive5_FanSpeedToDefault_Set)
        if(ADL_Overdrive5_FanSpeedToDefault_Set) {
            int returnCode = ADL_Overdrive5_FanSpeedToDefault_Set(adapterIndex, thermalControllerIndex);
            if(returnCode == ADL_OK) {
                success = true;
            } else {
                functionCallFailed("ADL_Overdrive5_FanSpeedToDefault_Set", returnCode);
            }
        } else {
            functionNotAvailable("ADL_Overdrive5_FanSpeedToDefault_Set");
        }
    }

    return success;
}

