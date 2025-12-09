// archive_guids.cpp - Define missing archive interface GUIDs
// Most GUIDs are defined in codec_guids_manual.cpp and password_guids.cpp
// This file only defines GUIDs that are missing

#include <guiddef.h>

// Base 7-Zip GUID: {23170F69-40C1-278A-0000-000600XXXXXX00}
#define k_7zip_GUID_Data1 0x23170F69
#define k_7zip_GUID_Data2 0x40C1
#define k_7zip_GUID_Data3_Common 0x278A

// IArchiveExtractCallback - group=6, sub=0x20
// Defined as: Z7_IFACE_CONSTR_ARCHIVE_SUB(IArchiveExtractCallback, IProgress, 0x20)
extern "C" const GUID IID_IArchiveExtractCallback = {
    k_7zip_GUID_Data1, k_7zip_GUID_Data2, k_7zip_GUID_Data3_Common, {0, 0, 0, 6, 0, 0x20, 0, 0}};

// IArchiveUpdateCallback - group=6, sub=0x80
// Defined as: Z7_IFACE_CONSTR_ARCHIVE_SUB(IArchiveUpdateCallback, IProgress, 0x80)
extern "C" const GUID IID_IArchiveUpdateCallback = {
    k_7zip_GUID_Data1, k_7zip_GUID_Data2, k_7zip_GUID_Data3_Common, {0, 0, 0, 6, 0, 0x80, 0, 0}};
