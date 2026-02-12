#pragma once
#define NULL ((void*)0)
typedef unsigned long long UINTN;
typedef unsigned char UINT8;
typedef unsigned short UINT16;
typedef unsigned short CHAR16;
typedef unsigned int UINT32;
typedef unsigned long long UINT64;
typedef unsigned long long EFI_STATUS;
typedef void *EFI_HANDLE;

typedef struct {
    UINT32 Type;
    void *PhysicalStart;
    void *VirtualStart;
    UINT64 NumberOfPages;
    UINT64 Attribute;
} EFI_MEMORY_DESCRIPTOR;

typedef EFI_STATUS (*EFI_GET_MEMORY_MAP)(
    UINTN *MemoryMapSize,
    EFI_MEMORY_DESCRIPTOR *MemoryMap,
    UINTN *MapKey,
    UINTN *DescriptorSize,
    UINT32 *DescriptorVersion
);


typedef struct { UINT16 ScanCode; CHAR16 UnicodeChar; } EFI_INPUT_KEY;
typedef struct { UINT32 RedMask; UINT32 GreenMask; UINT32 BlueMask; UINT32 ReservedMask; } EFI_PIXEL_BITMASK;
typedef enum { PixelRedGreenBlueReserved8BitPerColor, PixelBlueGreenRedReserved8BitPerColor, PixelBitMask, PixelBltOnly, PixelFormatMax } EFI_GRAPHICS_PIXEL_FORMAT;
typedef struct { UINT32 Version; UINT32 HorizontalResolution; UINT32 VerticalResolution; EFI_GRAPHICS_PIXEL_FORMAT PixelFormat; EFI_PIXEL_BITMASK PixelInformation; UINT32 PixelsPerScanLine; } EFI_GRAPHICS_OUTPUT_MODE_INFORMATION;
typedef struct { UINT32 MaxMode; UINT32 Mode; EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *Info; UINT64 SizeOfInfo; UINT64 FrameBufferBase; UINT64 FrameBufferSize; } EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE;
typedef struct _EFI_GRAPHICS_OUTPUT_PROTOCOL { void *QueryMode; void *SetMode; void *Blt; EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE *Mode; } EFI_GRAPHICS_OUTPUT_PROTOCOL;
typedef struct _EFI_SIMPLE_TEXT_INPUT_PROTOCOL { void *Reset; EFI_STATUS (*ReadKeyStroke)(struct _EFI_SIMPLE_TEXT_INPUT_PROTOCOL *This, EFI_INPUT_KEY *Key); void *WaitForKey; } EFI_SIMPLE_TEXT_INPUT_PROTOCOL;
typedef struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL { void *Reset; EFI_STATUS (*OutputString)(struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This, CHAR16 *String); void *TestString; void *QueryMode; void *SetMode; EFI_STATUS (*SetAttribute)(struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This, UINT64 Attribute); EFI_STATUS (*ClearScreen)(struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This); } EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;
typedef EFI_STATUS (*EFI_LOCATE_PROTOCOL)(void *Protocol, void *Registration, void **Interface);

typedef struct {
    char _pad[48];
    EFI_GET_MEMORY_MAP GetMemoryMap;
    char _pad2[264];
    EFI_LOCATE_PROTOCOL LocateProtocol;
} EFI_BOOT_SERVICES;

typedef struct { char _header[24]; void *FirmwareVendor; UINT32 FirmwareRevision; void *ConsoleInHandle; EFI_SIMPLE_TEXT_INPUT_PROTOCOL *ConIn; void *ConsoleOutHandle; EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *ConOut; void *StandardErrorHandle; void *StdErr; void *RuntimeServices; EFI_BOOT_SERVICES *BootServices; } EFI_SYSTEM_TABLE;
static const struct { UINT32 D1; UINT16 D2; UINT16 D3; UINT8 D4[8]; } EFI_GOP_GUID = { 0x9042a9de, 0x23dc, 0x4a38, {0x96, 0xfb, 0x7a, 0xde, 0xd0, 0x80, 0x51, 0x6a} };