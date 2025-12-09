// archive_factory.cpp - GUID definitions for 7-Zip archive interfaces

#include <guiddef.h>
#include <initguid.h>

// 定义 IID_IInArchive GUID
// {23170F69-40C1-278A-0000-000600600000}
DEFINE_GUID(IID_IInArchive, 0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x06, 0x00, 0x60, 0x00,
            0x00);

// 定义 IID_IOutArchive GUID
// {23170F69-40C1-278A-0000-000600A00000}
DEFINE_GUID(IID_IOutArchive, 0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x06, 0x00, 0xA0, 0x00,
            0x00);

// 定义 CLSID_CArchiveHandler (基础 CLSID)
// {23170F69-40C1-278A-1000-000110000000}
DEFINE_GUID(CLSID_CArchiveHandler, 0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x00,
            0x00, 0x00);

// CreateArchiver 函数由 7zip_archive 库提供（ArchiveExports.cpp）
// 这里只定义 GUID，不重新实现 CreateArchiver
