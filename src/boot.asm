; Simple bootloader that loads kernel into memory
; Uses BIOS interrupts to load sectors from disk

[org 0x7c00]
[bits 16]

KERNEL_OFFSET equ 0x1000

    mov [BOOT_DRIVE], dl    ; BIOS stores boot drive in DL

    mov bp, 0x9000          ; Set up stack
    mov sp, bp

    mov bx, MSG_REAL_MODE
    call print_string

    call load_kernel
    call switch_to_pm       ; Switch to 32-bit protected mode

    jmp $                   ; Hang if we return

%include "print.asm"
%include "disk.asm"
%include "gdt.asm"
%include "switch_pm.asm"

[bits 16]
load_kernel:
    mov bx, MSG_LOAD_KERNEL
    call print_string

    mov bx, KERNEL_OFFSET   ; Load kernel to this address
    mov dh, 32              ; Load 32 sectors (16KB)
    mov dl, [BOOT_DRIVE]
    call disk_load
    ret

[bits 32]
BEGIN_PM:
    mov ebx, MSG_PROT_MODE
    call print_string_pm

    call KERNEL_OFFSET      ; Jump to kernel

    jmp $                   ; Hang

BOOT_DRIVE      db 0
MSG_REAL_MODE   db "Started in 16-bit Real Mode", 0
MSG_PROT_MODE   db "Entered 32-bit Protected Mode", 0
MSG_LOAD_KERNEL db "Loading kernel...", 0

times 510-($-$$) db 0
dw 0xaa55
