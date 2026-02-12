#include "uefi_hal.h"

// متغیرهای گلوبال داخلی (دانشجو این‌ها را نمی‌بیند)
static EFI_SYSTEM_TABLE *gST;
static EFI_GRAPHICS_OUTPUT_PROTOCOL *gGop;

// تعریف تابع اصلی اسمبلی که باید صدا زده شود
extern void asm_main();
extern void GetCPUName(char* buffer);

// --- توابع کمکی داخلی ---
void *memcpy(void *dest, const void *src, unsigned long long n) {
    unsigned char *d = (unsigned char *)dest; const unsigned char *s = (const unsigned char *)src;
    while (n--) *d++ = *s++; return dest;
}
void *memset(void *dest, int c, unsigned long long n) {
    unsigned char *d = (unsigned char *)dest; while (n--) *d++ = (unsigned char)c; return dest;
}

// =============================================================
//  API برای استفاده در اسمبلی (HAL Functions)
// =============================================================

// 1. چاپ متن (ورودی: اشاره‌گر به رشته Wide String)
void HAL_Print(CHAR16 *Str) {
    gST->ConOut->OutputString(gST->ConOut, Str);
}

// 2. پاک کردن صفحه متنی
void HAL_ClearTextScreen() {
    gST->ConOut->ClearScreen(gST->ConOut);
}

// 3. گرفتن کلید فشرده شده (خروجی در RAX برمی‌گردد. اگر 0 بود یعنی کلیدی نیست)
UINT16 HAL_GetKey() {
    EFI_INPUT_KEY Key;
    EFI_STATUS Status = gST->ConIn->ReadKeyStroke(gST->ConIn, &Key);
    if (Status == 0) return Key.UnicodeChar;
    return 0;
}

// 4. دریافت آدرس فریم‌بافر (برای رسم گرافیک)
UINT64 HAL_GetVideoBase() {
    return gGop->Mode->FrameBufferBase;
}

// 5. دریافت عرض صفحه
UINT64 HAL_GetVideoWidth() {
    return (UINT64)gGop->Mode->Info->HorizontalResolution;
}

// 6. دریافت ارتفاع صفحه
UINT64 HAL_GetVideoHeight() {
    return (UINT64)gGop->Mode->Info->VerticalResolution;
}

// 7. پخش صدای بوق (ساده)
void HAL_Beep() {
    // کد قدیمی پورت 61 (ممکن است روی همه سیستم‌ها کار نکند ولی استاندارد آموزشی است)
    // اسمبلی هم می‌توانست این کار را کند، ولی گذاشتیم اینجا که راحت باشید
    asm volatile (
        "mov $0x61, %%dx; in %%dx, %%al; or $3, %%al; out %%al, %%dx;"
        "mov $0x5000000, %%rcx; 1: dec %%rcx; jnz 1b;" // Delay
        "in %%dx, %%al; and $0xFC, %%al; out %%al, %%dx;"
        ::: "rax", "rdx", "rcx"
    );
}

UINT64 HAL_StrLen(CHAR16 *str) {
    UINT64 len = 0;
    while (str[len] != 0) len++;
    return len;

}

void HAL_Itoa(UINT64 n, CHAR16 *str, UINT32 base) {
    UINT64 i = 0;
    UINT64 temp = n;

    if (temp == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return;
    }

    while (temp > 0) {
        UINT64 digit = temp % base;
        str[i++] = (digit < 10) ? (digit + '0') : (digit - 10 + 'A');
        temp /= base;
    }
    str[i] = '\0';

    for (UINT64 j = 0; j < i / 2; j++) {
        CHAR16 t = str[j];
        str[j] = str[i - j - 1];
        str[i - j - 1] = t;
    }
}

void PrintMenuItem(CHAR16 *Text, int IsSelected) {
    if (IsSelected) {
        gST->ConOut->SetAttribute(gST->ConOut, 0x70); 
    } else {
        gST->ConOut->SetAttribute(gST->ConOut, 0x0F);
    }
    HAL_Print(Text);
    HAL_Print(L"\r\n");
    
    gST->ConOut->SetAttribute(gST->ConOut, 0x0F);
}

// =============================================================
//  نقطه شروع برنامه (Entry Point)
// =============================================================

void Menu_SystemInfo() {
    HAL_ClearTextScreen();
    HAL_Print(L"--- SYSTEM INFORMATION ---\r\n\n");

    char cpu_name_ascii[49];
    memset(cpu_name_ascii, 0, 49);
    GetCPUName(cpu_name_ascii);

    CHAR16 wide_cpu[49];
    for(int i=0; i<48; i++) wide_cpu[i] = (CHAR16)cpu_name_ascii[i];
    wide_cpu[48] = L'\0';

    HAL_Print(L"Processor: ");
    HAL_Print(wide_cpu);
    HAL_Print(L"\r\n");

    UINTN MapSize = 0;
    UINTN MapKey, DescriptorSize;
    UINT32 DescriptorVersion;
    
    gST->BootServices->GetMemoryMap(&MapSize, NULL, &MapKey, &DescriptorSize, &DescriptorVersion);
    MapSize += 2048;
    
    HAL_Print(L"Calculated Map Size: ");
    CHAR16 size_str[20];
    HAL_Itoa(MapSize, size_str, 10);
    HAL_Print(size_str);
    HAL_Print(L" bytes\r\n");

    HAL_Print(L"\nPress any key to return...");
    while(HAL_GetKey() == 0);
}

void Menu_GraphicsBall() {
    HAL_ClearTextScreen();
    
    UINT32 *Base = (UINT32*)HAL_GetVideoBase();
    UINT32 W = (UINT32)HAL_GetVideoWidth();
    UINT32 H = (UINT32)HAL_GetVideoHeight();
    
    int x = 100, y = 100;
    int dx = 2, dy = 2;
    int size = 30;
    int size2 = 30;
    int paused = 0;

    while (1) {
        EFI_INPUT_KEY Key;
        if (gST->ConIn->ReadKeyStroke(gST->ConIn, &Key) == 0) {
            if (Key.ScanCode == 0x17) break; 
            if (Key.UnicodeChar == L' ') paused = 1 - paused;
        }

        if (!paused) {
            for(int i = 0; i < size; i++) {
                for(int j = 0; j < size2; j++) {
                    Base[(y+j) * W + (x+i)] = 0x00000000; 
                }
            }

            x += dx; y += dy;

            if (x <= 0 || x >= (int)W - size) dx = -dx;
            if (y <= 0 || y >= (int)H - size) dy = -dy;
            for(int i = 0; i < size; i++) {
                for(int j = 0; j < size2; j++) {
                    Base[(y+j) * W + (x+i)] = 0x00FFFF00; 
                }
            }
        }


        

        for(volatile int d = 0; d < 1000000; d++);
    }
}

void Menu_ColorCalibration() {
    HAL_ClearTextScreen();
    HAL_Print(L"--- COLOR CALIBRATION ---\r\n");
    HAL_Print(L"Keys: [R]ed, [G]reen, [B]lue, [W]hite, [C]lear\r\n");
    HAL_Print(L"      [A]uto-Cycle, [D]efault, [ESC] Return\r\n\n");

    UINT32 *Base = (UINT32*)HAL_GetVideoBase();
    UINT64 Width = HAL_GetVideoWidth();
    UINT64 Height = HAL_GetVideoHeight();
    UINT64 TotalPixels = Width * Height;

    int auto_mode = 0;
    int default_mode = 0;
    UINT32 auto_colors[] = {0x00FF0000, 0x0000FF00, 0x000000FF, 0x00FFFFFF, 0x00000000};
    int current_color_idx = 0;

    while (1) {
        EFI_INPUT_KEY Key;
        if (gST->ConIn->ReadKeyStroke(gST->ConIn, &Key) == 0) {
            UINT32 color = 0;
            int apply = 0;

            if (Key.ScanCode == 0x17) break; // ESC

            switch (Key.UnicodeChar) {
                case L'r': case L'R': color = 0x00FF0000; apply = 1; default_mode = 0; auto_mode = 0; break;
                case L'g': case L'G': color = 0x0000FF00; apply = 1; default_mode = 0; auto_mode = 0; break;
                case L'b': case L'B': color = 0x000000FF; apply = 1; default_mode = 0; auto_mode = 0; break;
                case L'w': case L'W': color = 0x00FFFFFF; apply = 1; default_mode = 0; auto_mode = 0; break;
                case L'c': case L'C': color = 0x00000000; apply = 1; default_mode = 0; auto_mode = 0; break;
                case L'd': case L'D': apply = 1; default_mode = 1; auto_mode = 0; break;
                case L'a': case L'A': auto_mode = 1; break;
            }

            if (apply) {
                if(default_mode) {
                    HAL_ClearTextScreen();
                    HAL_Print(L"--- COLOR CALIBRATION ---\r\n");
                    HAL_Print(L"Keys: [R]ed, [G]reen, [B]lue, [W]hite, [C]lear\r\n");
                    HAL_Print(L"      [A]uto-Cycle, [D]efault, [ESC] Return\r\n\n");
                }
                else
                    for (UINT64 i = 0; i < TotalPixels; i++) Base[i] = color;
            }
        }

        if (auto_mode) {
            for (UINT64 i = 0; i < TotalPixels; i++) Base[i] = auto_colors[current_color_idx];
            
            current_color_idx = (current_color_idx + 1) % 5;
            
            for (volatile int d = 0; d < 20000000; d++); 
        }
    }
}

CHAR16 *VarName = L"TeamID";

void Menu_TeamSettings() {
    __attribute__((aligned(8))) static EFI_GUID LocalGuid = { 0x8BE4DF61, 0x93CA, 0x11d2, {0xAA, 0x0D, 0x00, 0xE0, 0x98, 0x03, 0x2B, 0x8C} };
    __attribute__((aligned(8))) CHAR16 *LocalVarName = L"TeamID";
    
    EFI_GET_VARIABLE GetVar = (EFI_GET_VARIABLE)gST->RuntimeServices->_ptrs[6];
    EFI_SET_VARIABLE SetVar = (EFI_SET_VARIABLE)gST->RuntimeServices->_ptrs[8];

    CHAR16 team_id[20];
    UINTN data_size = sizeof(team_id);
    memset(team_id, 0, data_size);

    EFI_STATUS status = GetVar(LocalVarName, &LocalGuid, NULL, &data_size, team_id);

    HAL_ClearTextScreen();
    HAL_Print(L"--- TEAM SETTINGS (NVRAM) ---\r\n\n");
    
    if (status == 0) {
        HAL_Print(L"Current ID: ");
        HAL_Print(team_id);
    } else {
        HAL_Print(L"Current ID: (Not Set)\r\n");
        HAL_Print(L"Status Code: ");
        CHAR16 err_str[20];
        HAL_Itoa(status, err_str, 16);
        HAL_Print(err_str);
    }

    HAL_Print(L"\r\n\n[S] Save 'Team_08' | [ESC] Return\r\n");

    while (1) {
        EFI_INPUT_KEY Key;
        if (gST->ConIn->ReadKeyStroke(gST->ConIn, &Key) == 0) {
            if (Key.ScanCode == 0x17) break; // ESC
            
            if (Key.UnicodeChar == L's' || Key.UnicodeChar == L'S') {
                CHAR16 *newData = L"Team_08";
                // 16 bytes = (7 chars + 1 null terminator) * 2 bytes/char
                UINTN newDataSize = 16; 

                status = SetVar(LocalVarName, &LocalGuid, 
                                EFI_VARIABLE_NON_VOLATILE | 
                                EFI_VARIABLE_BOOTSERVICE_ACCESS | 
                                EFI_VARIABLE_RUNTIME_ACCESS, 
                                newDataSize, (void*)newData);
                
                if (status == 0) {
                    HAL_Print(L"\r\nSuccess! Team ID updated.");
                } else {
                    HAL_Print(L"\r\nSetVar Error: ");
                    CHAR16 err_s[20];
                    HAL_Itoa(status, err_s, 16);
                    HAL_Print(err_s);
                }
            }
        }
    }
}

void Menu_SystemControl() {
    EFI_RESET_SYSTEM ResetSystem = gST->RuntimeServices->ResetSystem;
    
    EFI_SET_VARIABLE SetVar = (EFI_SET_VARIABLE)gST->RuntimeServices->_ptrs[8];

    while (1) {
        HAL_ClearTextScreen();
        HAL_Print(L"--- SYSTEM CONTROL ---\r\n\n");
        HAL_Print(L"1. Shutdown (Power Off)\r\n");
        HAL_Print(L"2. Reboot (Cold Reset)\r\n");
        HAL_Print(L"3. Reboot to Firmware (BIOS Settings)\r\n");
        HAL_Print(L"ESC. Return\r\n");

        EFI_INPUT_KEY Key;
        while (gST->ConIn->ReadKeyStroke(gST->ConIn, &Key) != 0);

        if (Key.ScanCode == 0x17) break; // ESC key

        if (Key.UnicodeChar == L'1') {
            ResetSystem(EfiResetShutdown, 0, 0, NULL);
        }
        else if (Key.UnicodeChar == L'2') {
            ResetSystem(EfiResetCold, 0, 0, NULL);
        }
        else if (Key.UnicodeChar == L'3') {
            UINT64 indications = 0x0000000000000001;
            __attribute__((aligned(8))) EFI_GUID GlobalGuid = { 0x8BE4DF61, 0x93CA, 0x11d2, {0xAA, 0x0D, 0x00, 0xE0, 0x98, 0x03, 0x2B, 0x8C} };
            
            SetVar(L"OsIndications", &GlobalGuid, 
                   EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS, 
                   sizeof(UINT64), &indications);

            ResetSystem(EfiResetWarm, 0, 0, NULL);
        }
    }
}

void HAL_Menu_System() {
    int selected = 0;
    int totalItems = 5;
    EFI_INPUT_KEY Key;
    CHAR16 *LocalVarName = L"TeamID";

    while (1) {
        HAL_ClearTextScreen();

        CHAR16 team_id[20];
        UINTN d_size = sizeof(team_id);
        memset(team_id, 0, d_size);

        EFI_GET_VARIABLE GetVar = (EFI_GET_VARIABLE)gST->RuntimeServices->_ptrs[6];
        GetVar(LocalVarName, (EFI_GUID*)&EFI_GLOBAL_VARIABLE_GUID, NULL, &d_size, team_id);
        
        HAL_Print(L"=== TEAM_08 UEFI OVERLORD ===\r\n");
        if (team_id[0] != 0) {
            HAL_Print(L"Current ID: ");
            HAL_Print(team_id);
            HAL_Print(L"\r\n");
        }

        HAL_Print(L"Use UP/DOWN arrows and ENTER to select\r\n\n");

        PrintMenuItem(L" 1. System Information (CPUID/Memory) ", (selected == 0));
        PrintMenuItem(L" 2. Graphics Test (Bouncing Ball)     ", (selected == 1));
        PrintMenuItem(L" 3. Color Calibration                 ", (selected == 2));
        PrintMenuItem(L" 4. Team Settings (NVRAM)             ", (selected == 3));
        PrintMenuItem(L" 5. System Control (Power/Reset)      ", (selected == 4));

        while (gST->ConIn->ReadKeyStroke(gST->ConIn, &Key) != 0);

        if (Key.ScanCode == 0x01 && selected > 0) selected--;
        else if (Key.ScanCode == 0x02 && selected < totalItems - 1) selected++;
        else if (Key.UnicodeChar == L'\r') {
            if (selected == 0) Menu_SystemInfo();
            if (selected == 1) Menu_GraphicsBall();
            if (selected == 2) Menu_ColorCalibration();
            if (selected == 3) Menu_TeamSettings();
            if (selected == 4) Menu_SystemControl();
        }
    }
}

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    gST = SystemTable;
    
    // راه‌اندازی گرافیک
    EFI_STATUS Status = gST->BootServices->LocateProtocol((void*)&EFI_GOP_GUID, 0, (void**)&gGop);
    if (Status != 0) {
        gST->ConOut->OutputString(gST->ConOut, L"CRITICAL ERROR: GOP NOT FOUND!\r\n");
        return Status;
    }

    // *** پرش به کد دانشجو ***
    // asm_main();

    HAL_Menu_System();

    // اگر دانشجو return کرد:
    gST->ConOut->OutputString(gST->ConOut, L"Assembly Kernel Exited.\r\n");
    while(1);
    return 0;
}
