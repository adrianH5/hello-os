org 0x7C00
bits 16

%define ENDL 0x0D, 0x0A

;
; Prints a string to the screen
; params:
;   - ds:si points to string
; NOTE: ds -> data segment, si -> source index: offset in datasegment

start:
  jmp main


puts:
  push ds
  push si

.loop:
  lodsb
  or al, al
  jz .done

  ; call bios
  mov ah, 0x0e
  mov bh, 0
  int 0x10

  jmp .loop

.done:
  pop si
  pop ds
  ret


main:
  ; setup data segment
  mov ax, 0
  mov ds, ax
  mov es, ax

  ; setup stack segment
  mov ss, ax
  ; stack pointer measures offset, SS:SP
  mov sp, 0x7C00

  mov si, msg_hello
  call puts

  hlt

.halt:
  jmp .halt

msg_hello: db 'Hello World', ENDL, 0

times 510-($-$$) db 0
dw 0xAA55
