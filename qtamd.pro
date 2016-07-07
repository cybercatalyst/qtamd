TEMPLATE = lib

CONFIG += staticlib
TARGET = qtamd

SOURCES += \
    amdoverdrive.cpp
HEADERS += \
    adl/adl_defines.h \
    adl/adl_sdk.h \
    adl/adl_structures.h \
    amdoverdrive.h \
    adlfunctionpointers.h
