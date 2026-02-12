CC = clang
ASM = nasm
LD = lld-link

CFLAGS = -target x86_64-unknown-windows -ffreestanding -fshort-wchar -mno-red-zone -mno-stack-arg-probe -nostdlib -c
ASMFLAGS = -f win64
LDFLAGS = -subsystem:efi_application -entry:efi_main

all: BOOTX64.EFI

# کامپایل بخش HAL
uefi_hal.obj: uefi_hal.c uefi_hal.h
	$(CC) $(CFLAGS) -o uefi_hal.obj uefi_hal.c

# اسمبل کردن کد دانشجو
main.obj: main.asm
	$(ASM) $(ASMFLAGS) -o main.obj main.asm

# لینک کردن همه با هم
BOOTX64.EFI: uefi_hal.obj main.obj
	$(LD) $(LDFLAGS) -out:BOOTX64.EFI uefi_hal.obj main.obj

clean:
	rm -f *.obj *.EFI

run: BOOTX64.EFI
	mkdir -p iso/EFI/BOOT
	cp BOOTX64.EFI iso/EFI/BOOT/
	# استفاده از bios.bin که قبلا پیدا کردیم
	qemu-system-x86_64 -bios /usr/share/ovmf/OVMF.fd -net none -drive format=raw,file=fat:rw:iso
