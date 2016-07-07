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

    AMDOverdrive();

    int numberOfAdapters();
    QList<AdapterInfo> adapterInfo();
    bool isAdapterActive(AdapterInfo adapterInfo);
    Capabilities capabilities(int adapterIndex);

    bool isPowerControlSupported5(int adapterIndex);
    ADLPowerControlInfo powerControlInfo5(int adapterIndex);
    int powerControlGetCurrent5(int adapterIndex);
    int powerControlGetDefault5(int adapterIndex);
    bool powerControlSet5(int adapterIndex, int value);

private:
#if defined Q_OS_LINUX
    void *_dll;
#else
    HINSTANCE _dll;
#endif

};
