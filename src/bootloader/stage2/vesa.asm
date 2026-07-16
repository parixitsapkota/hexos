%define ENDL 0x0D, 0x0A

DATA_SEGMENT32    equ 10h
CODE_SEGMENT32    equ 08h

; Functions
global setup_vesa
extern enable32

; Variables
global g_framebuffer
global g_pitch
global g_bpp
global g_width
global g_height

section .text
[BITS 16]
setup_vesa:
  ; query mode 0x115 (800x600 24/32-bit) details to find its LFB address
  mov ax, 0x4F01             ; VBE Get Mode Info function
  mov cx, 0x0118             ; Mode number we want to query
  mov di, mode_info_buffer   ; ES:DI points to our 256-byte real mode buffer
  int 0x10
  
  cmp ax, 0x004F
  jne .fail

  ; The 32-bit physical base address is at offset 40 (0x28) of the Mode Info Block
  mov eax, [mode_info_buffer + 0x28]
  mov [g_framebuffer], eax   ; Save it to our global variable1

  mov ax, [mode_info_buffer + 0x10]  ; Offset 16: Bytes Per Scanline (Pitch)
  mov [g_pitch], ax

  mov al, [mode_info_buffer + 0x19]  ; Offset 25: Bits Per Pixel (BPP)
  mov [g_bpp], al

  mov ax, [mode_info_buffer + 0x12]  ; Offset 18 (0x12): X Resolution (Width)
  mov [g_width], ax

  mov ax, [mode_info_buffer + 0x14]  ; Offset 20 (0x14): Y Resolution (Height)
  mov [g_height], ax

  mov ax, 0x4F02  ; VBE Set Mode function
  mov bx, 0x4118  ; Mode 0x118 + Linear Frame Buffer bit (0x4000)
  int 0x10

  cmp ax, 0x004F
  je .success
  .fail:
    mov si, msg_vesa_fail
    call puts
  .success:
    ret

[BITS 32]
global setup_vesa16
setup_vesa16:
  pusha
  mov [saved_esp], esp
  jmp 0x18:.pmode16

  [BITS 16]
  .pmode16:
    mov ax, 0x20
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    lidt [rmode_idtr]

    mov eax, cr0
    and al, ~1
    mov cr0, eax

    jmp 0x0000:.rmode

  .rmode:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov sp, 0x7C00

    ; mov di, mode_info_buffer   ; Ensure ES is 0 before calling setup_vesa
    call setup_vesa

    call enable32
    jmp dword CODE_SEGMENT32:.pmode32

  [BITS 32]
  .pmode32:
    mov ax, DATA_SEGMENT32
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov esp, [saved_esp]
    popa
    ret

[BITS 16]
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

section .data
mode_info_buffer: times 256 db 0

saved_esp:     dd 0

; Global variables for VESA framebuffer information
g_framebuffer: dd 0
g_pitch:       dw 0
g_width:       dw 0
g_height:      dw 0
g_bpp:         db 0

section .rodata
align 4
rmode_idtr:
  dw 0x3FF
  dd 0

msg_vesa_fail: db 'VESA Setup failed!', ENDL, 0
msg_reboot: db 'Press any key to reboot...', ENDL, 0
