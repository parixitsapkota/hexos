KbdControllerDataPort               equ 0x60
KbdControllerCommandPort            equ 0x64
KbdControllerDisableKeyboard        equ 0xAD
KbdControllerEnableKeyboard         equ 0xAE
KbdControllerReadCtrlOutputPort     equ 0xD0
KbdControllerWriteCtrlOutputPort    equ 0xD1

DATA_SEGMENT32    equ 10h
CODE_SEGMENT32    equ 08h

; Functions
extern bmain
global switch32
global calc_M
global enable32

section .entry
[BITS 16]
switch32:
  cli                     ; Disable interrupts
  call enableA20          ; Enable A20 gate
  lgdt [g_GDTDesc]        ; Load GDT
  call enable32           ; Setup flag
  ; Far jump into protected mode
  jmp dword CODE_SEGMENT32:.pmode32

  [BITS 32]
  .pmode32:
    mov ax, DATA_SEGMENT32
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000        ; Move stack safely away from the code
    call bmain
    ; Fallback safety
    cli
    hlt
    jmp $

section .text
[BITS 16]
enable32:
  mov eax, cr0
  or al, 1
  mov cr0, eax
  ret

enableA20:
  ; disable keyboard
  call A20WaitInput
  mov al, KbdControllerDisableKeyboard
  out KbdControllerCommandPort, al

  ; read control output port
  call A20WaitInput
  mov al, KbdControllerReadCtrlOutputPort
  out KbdControllerCommandPort, al

  call A20WaitOutput
  in al, KbdControllerDataPort
  push eax

  ; write control output port
  call A20WaitInput
  mov al, KbdControllerWriteCtrlOutputPort
  out KbdControllerCommandPort, al
  
  call A20WaitInput
  pop eax
  or al, 2  ; bit 2 = A20 bit
  out KbdControllerDataPort, al

  ; enable keyboard
  call A20WaitInput
  mov al, KbdControllerEnableKeyboard
  out KbdControllerCommandPort, al

  call A20WaitInput
  ret

A20WaitInput:
  in al, KbdControllerCommandPort
  test al, 2
  jnz A20WaitInput
  ret

A20WaitOutput:
  in al, KbdControllerCommandPort
  test al, 1
  jz A20WaitOutput
  ret

section .rodata
g_GDT:
  ; NULL descriptor
  dq 0

  ; 32-bit code segment
  dw 0FFFFh     ; limit (bits 0-15) = 0xFFFFF for full 32-bit range
  dw 0          ; base  (bits 0-15) = 0x0
  db 0          ; base  (bits 16-23)
  db 10011010b  ; access (present, ring 0, code segment, executable, direction 0, readable)
  db 11001111b  ; granularity (4k pages, 32-bit pmode) + limit (bits 16-19)
  db 0          ; base high

  ; 32-bit data segment
  dw 0FFFFh     ; limit (bits 0-15) = 0xFFFFF for full 32-bit range
  dw 0          ; base  (bits 0-15) = 0x0
  db 0          ; base  (bits 16-23)
  db 10010010b  ; access (present, ring 0, data segment, executable, direction 0, writable)
  db 11001111b  ; granularity (4k pages, 32-bit pmode) + limit (bits 16-19)
  db 0          ; base high

  ; 16-bit code segment
  dw 0FFFFh     ; limit (bits 0-15) = 0xFFFFF
  dw 0          ; base (bits 0-15) = 0x0
  db 0          ; base (bits 16-23)
  db 10011010b  ; access (present, ring 0, code segment, executable, direction 0, readable)
  db 00001111b  ; granularity (1b pages, 16-bit pmode) + limit (bits 16-19)
  db 0          ; base high

  ; 16-bit data segment
  dw 0FFFFh     ; limit (bits 0-15) = 0xFFFFF
  dw 0          ; base (bits 0-15) = 0x0
  db 0          ; base (bits 16-23)
  db 10010010b  ; access (present, ring 0, data segment, executable, direction 0, writable)
  db 00001111b  ; granularity (1b pages, 16-bit pmode) + limit (bits 16-19)
  db 0          ; base high
  
  ; 64-bit Code Segment
  dw 0          ; Limit (ignored)
  dw 0          ; Base low (ignored)
  db 0          ; Base middle (ignored)
  db 10011010b  ; Access (Present, Ring 0, Code, Executable, Readable)
  db 00100000b  ; Flags (L=1 for 64-bit, D/B=0) + Limit high=0
  db 0          ; Base high (ignored)

  ; 64-bit Data Segment
  dw 0          ; Limit (ignored)
  dw 0          ; Base low (ignored)
  db 0          ; Base middle (ignored)
  db 10010010b  ; Access (Present, Ring 0, Data, Writable)
  db 00000000b  ; Flags (L=0, D/B=0 for 64-bit data) + Limit high=0
  db 0          ; Base high (ignored)

g_GDTDesc:  dw g_GDTDesc - g_GDT - 1    ; limit = size of GDT
            dd g_GDT                    ; address of GDT
