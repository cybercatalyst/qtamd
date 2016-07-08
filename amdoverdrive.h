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

#pragma once

#include <Qt>
#include <QString>
#include <QList>

#if defined Q_OS_LINUX
#   include <dlfcn.h>
#   include <stdlib.h>
#   include <string.h>
#   include <unistd.h>
#   ifndef LINUX
#       define LINUX
#   endif
#else
#   include <windows.h>
#   include <tchar.h>
#endif

// ADL SDK includes
#include "adl/adl_sdk.h"
#include "adl/adl_structures.h"

class AMDOverdrive {
public:
    struct Capabilities {
        int supported;
        int enabled;
        int version;
    };

    struct PerformanceLevelInfo {
        ADLODPerformanceLevel stock;
        ADLODPerformanceLevel current;
    };

    enum FanSpeedValueType {
        Rpm,
        Percent
    };

    AMDOverdrive();

    // General parameters
    int numberOfAdapters();
    QList<AdapterInfo> adapterInfo();
    bool isAdapterActive(AdapterInfo adapterInfo);
    Capabilities capabilities(int adapterIndex);

    // Clocks and activity
    bool isPowerControlSupported(int adapterIndex);
    ADLPowerControlInfo powerControlInfo(int adapterIndex);
    int powerControlGetCurrent(int adapterIndex);
    int powerControlGetDefault(int adapterIndex);
    bool powerControlSet(int adapterIndex, int value);
    ADLODParameters overdriveParameters(int adapterIndex);
    ADLPMActivity currentActivity(int adapterIndex);
    QList<PerformanceLevelInfo> performanceLevels(int adapterIndex);
    bool setCoreClock(int adapterIndex, int performanceLevel, int clockMHz);
    bool setMemoryClock(int adapterIndex, int performanceLevel, int clockMHz);
    bool setVoltage(int adapterIndex, int performanceLevel, int voltagemV);

    // Thermal control
    QList<ADLThermalControllerInfo> thermalControllersInfo(int adapterIndex);
    int temperatureMillidegreesCelsius(int adapterIndex, int thermalControllerIndex);
    ADLFanSpeedInfo fanSpeedInfo(int adapterIndex, int thermalControllerIndex);
    bool fanSupportsPercentRead(ADLFanSpeedInfo fanSpeedInfo);
    bool fanSupportsRpmRead(ADLFanSpeedInfo fanSpeedInfo);
    bool fanSupportsPercentWrite(ADLFanSpeedInfo fanSpeedInfo);
    bool fanSupportsRpmWrite(ADLFanSpeedInfo fanSpeedInfo);
    int fanSpeedValue(int adapterIndex, int thermalControllerIndex, FanSpeedValueType type);
    bool setFanSpeedValue(int adapterIndex, int thermalControllerIndex, FanSpeedValueType type, int value);
    bool setFanSpeedToDefault(int adapterIndex, int thermalControllerIndex);

private:
    enum PerformanceLevelField {
        CoreClock,
        MemoryClock,
        Voltage
    };

    bool writePerformanceLevel(int adapterIndex, int performanceLevel, PerformanceLevelField field, int value);

#if defined Q_OS_LINUX
    void *_dll;
#else
    HINSTANCE _dll;
#endif

};
