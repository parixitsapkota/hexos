[ORG 0x7C00]
[BITS 16]

%define ENDL 0x0D, 0x0A
STAGE2ADR equ 0x07E00

_start:
  ; Standardize data segments and stack
  xor ax, ax
  mov ds, ax
  mov es, ax
  mov ss, ax
  mov sp, 0x7C00

  call clear_screen
  mov si, warinig_msg
  call puts

  cli
  hlt
  jmp $

puts:
  push si
  .loop:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0E
    int 0x10
    jmp .loop
  .done:
    pop si
    ret

clear_screen:
  mov ah, 06h
  mov al, 0
  mov bh, 07h
  mov ch, 0
  mov cl, 0
  mov dh, 24
  mov dl, 79
  int 10h
  mov ah, 0x02
  mov bh, 0x00
  mov dh, 0x00
  mov dl, 0x00
  int 0x10
  mov ah, 01h
  mov ch, 20h
  mov cl, 00h
  int 10h
  ret

warinig_msg: db 'Warning hexos requires UEFI mode...', ENDL, 'Retry booting with UEFI mode...', ENDL, 0

times 510-($-$$) db 0
dw 0xAA55
