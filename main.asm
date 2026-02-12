section .text
global asm_main
global GetCPUName

; --- وارد کردن توابع HAL ---
extern HAL_Print
extern HAL_ClearTextScreen
extern HAL_GetKey
extern HAL_GetVideoBase
extern HAL_GetVideoWidth
extern HAL_GetVideoHeight
extern HAL_Beep

; --- ماکروی جادویی برای صدا زدن توابع C ---
; دانشجو به جای call باید بنویسد mcall
%macro mcall 1
    sub rsp, 40       ; رزرو فضای خالی برای ویندوز/UEFI
    call %1           ; صدا زدن تابع
    add rsp, 40       ; تمیز کردن استک
%endmacro

GetCPUName:
    push rbx                ; RBX is a callee-saved register, must save it!
    mov r10, rcx            ; Store buffer pointer in r10

    ; Call 1: 0x80000002
    mov eax, 0x80000002
    cpuid
    mov [r10], eax
    mov [r10 + 4], ebx
    mov [r10 + 8], ecx
    mov [r10 + 12], edx

    ; Call 2: 0x80000003
    mov eax, 0x80000003
    cpuid
    mov [r10 + 16], eax
    mov [r10 + 20], ebx
    mov [r10 + 24], ecx
    mov [r10 + 28], edx

    ; Call 3: 0x80000004
    mov eax, 0x80000004
    cpuid
    mov [r10 + 32], eax
    mov [r10 + 36], ebx
    mov [r10 + 40], ecx
    mov [r10 + 44], edx

    pop rbx
    ret

; =================================================================
; شروع برنامه
; =================================================================
asm_main:
    ; 1. پاک کردن صفحه متنی
    mcall HAL_ClearTextScreen

    ; 2. چاپ پیام خوش‌آمدگویی
    ; (در NASM برای رشته‌های UTF-16 باید دستی بایت‌ها را بنویسیم یا از __utf16__ استفاده کنیم)
    mov rcx, str_welcome
    mcall HAL_Print

    mov rcx, str_menu
    mcall HAL_Print

    ; 3. دریافت اطلاعات گرافیک (ذخیره در رجیسترهای امن R12, R13, R14)
    mcall HAL_GetVideoBase
    mov r12, rax      ; R12 = Base Address

    mcall HAL_GetVideoWidth
    mov r13, rax      ; R13 = Width

    mcall HAL_GetVideoHeight
    mov r14, rax      ; R14 = Height

; =================================================================
; حلقه اصلی برنامه (Main Loop)
; =================================================================
loop_start:
    ; الف) آیا کلیدی فشرده شده؟
    mcall HAL_GetKey    ; خروجی در RAX (در واقع AL)
    
    cmp ax, 0           ; اگر 0 بود یعنی خبری نیست
    je loop_start       ; برو اول حلقه

    ; ب) بررسی کلیدها
    cmp ax, 'r'         ; کلید r (قرمز)
    je do_red

    cmp ax, 'b'         ; کلید b (آبی)
    je do_blue

    cmp ax, 's'         ; کلید s (صدا)
    je do_beep
    
    cmp ax, 'q'         ; کلید q (خروج)
    je do_exit

    jmp loop_start

; --- هندلرها ---

do_red:
    mov rcx, str_msg_red
    mcall HAL_Print
    
    mov r9d, 0x00FF0000 ; رنگ قرمز
    call fill_screen
    jmp loop_start

do_blue:
    mov rcx, str_msg_blue
    mcall HAL_Print
    
    mov r9d, 0x000000FF ; رنگ آبی
    call fill_screen
    jmp loop_start

do_beep:
    mov rcx, str_msg_beep
    mcall HAL_Print
    
    mcall HAL_Beep      ; صدا زدن تابع بوق از C
    jmp loop_start

do_exit:
    ret                 ; پایان برنامه

; =================================================================
; تابع کمکی داخلی اسمبلی (پر کردن صفحه)
; ورودی: R9d = رنگ
; استفاده از متغیرهای ذخیره شده: R12, R13, R14
; =================================================================
fill_screen:
    push rdi            ; ذخیره رجیسترها
    push rax
    push rbx

    mov rax, r13        ; Width
    mul r14             ; * Height
    mov rbx, rax        ; تعداد پیکسل

    mov rdi, r12        ; آدرس شروع

.paint:
    mov [rdi], r9d      ; ریختن رنگ
    add rdi, 4
    dec rbx
    jnz .paint

    pop rbx
    pop rax
    pop rdi
    ret

; =================================================================
; داده‌ها (Strings)
; =================================================================
section .data

; تعریف رشته‌های یونیکد (UCS-2) برای UEFI
; هر کاراکتر 2 بایت است. پایان رشته باید 00 00 باشد.
str_welcome: db __utf16__(`Welcome to My Assembly OS!\r\n`), 0
str_menu:    db __utf16__(`[r] Red  [b] Blue  [s] Sound  [q] Quit\r\n`), 0
str_msg_red: db __utf16__(`> Painting RED...\r\n`), 0
str_msg_blue:db __utf16__(`> Painting BLUE...\r\n`), 0
str_msg_beep:db __utf16__(`> BEEP!\r\n`), 0