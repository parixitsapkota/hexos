%define ENDL 0x0D, 0x0A

KbdControllerDataPort               equ 0x60
KbdControllerCommandPort            equ 0x64
KbdControllerDisableKeyboard        equ 0xAD
KbdControllerEnableKeyboard         equ 0xAE
KbdControllerReadCtrlOutputPort     equ 0xD0
KbdControllerWriteCtrlOutputPort    equ 0xD1

DATA_SEGMENT32    equ 10h
CODE_SEGMENT32    equ 08h
DATA_SEGMENT64    equ 30h
CODE_SEGMENT64    equ 28h

kernel_load_vadr  equ 0xFFFFFFFF80100000
; kernel_load_vadr  equ 0x00100000

; Functions
extern bmain
global _start
global setup_vesa16
global calc_M
global _PAGING

; Variables
extern g_PML4
global g_M
global g_framebuffer
global g_pitch
global g_bpp
global g_width
global g_height

section .entry
[BITS 16]
_start:
  cli                     ; Disable interrupts
  call EnableA20          ; Enable A20 gate
  lgdt [g_GDTDesc]      ; Load GDT
  call Enable32           ; Setup flag
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

    call Enable32
    jmp dword 0x08:.pmode32

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

[BITS 32]
_PAGING:
	; Set CR4.PAE        : (Bit 5) = 1
	; Clear CR4.LA57     : (Bit 12) = 0
	; Clear CR4.PCIDE    : (Bit 17) = 1
	mov eax, cr4         ; Read current CR4 value
	bts eax, 5           ; Bit Test and Set: PAE (Bit 5) to 1
	btr eax, 12          ; Bit Test and Reset: LA57 (Bit 12) to 0
	; bts eax, 17          ; Bit Test and Set: PCIDE (Bit 7) to 1
	mov cr4, eax         ; Write updated value back to CR4

	; Set IA32_EFER.LME  : (Bit 8) = 1
	mov ecx, 0xC0000080  ; Load IA32_EFER MSR index into ECX
	rdmsr                ; Read MSR into EDX:EAX (Lower 32 bits are in EAX)
	bts eax, 8           ; Bit Test and Set: LME (Bit 8) to 1
	wrmsr                ; Write updated EDX:EAX back to IA32_EFER MSR

	; load PML4 into CR3 (PML4 should be under 4G or this fails/truncates in 64-Bit initialization)
	mov eax, g_PML4        ; Load PML4 structure into EAX
	mov cr3, eax         ; Write updated EAX to CR3

	; Set CR0.PG         : (Bit 31) = 1
	mov eax, cr0         ; Read current CR0 value
	bts eax, 31          ; Bit Test and Set: PG (Bit 31) to 1
	mov cr0, eax         ; Write updated value back to CR0
  ; 64-Bit Paging is now enabled
	; Reload CS with a 64-bit code selector by performing a long jmp
	jmp CODE_SEGMENT64:RELOAD_CS

	[BITS 64]
	RELOAD_CS:
    mov ax, DATA_SEGMENT64
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov rax, kernel_load_vadr   ; Load RAX kernel entry point address
    jmp rax                     ; Jump to your 64-bit kernel entry point

    cli
    hlt
    jmp $

[BITS 32]
calc_M:
	; Query CPUID for address sizes
	mov eax, 0x80000008 ; Leaf 80000008H 
	cpuid               ; Executes CPUID. Returns info in EAX, EBX, ECX, EDX
	; EAX bits 7:0 contain MAXPHYADDR (M)
	and eax, 0xFF       ; Mask out everything except the lower 8 bits of EAX
	; EAX now holds the integer value of M (e.g., 36, 40, 46, 48, or 52)
  mov [g_M], eax
  ret

[BITS 16]
Enable32:
  mov eax, cr0
  or al, 1
  mov cr0, eax
  ret

EnableA20:
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
  or al, 2                                    ; bit 2 = A20 bit
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

wait_key_and_reboot:
  mov si, msg_reboot
  call puts
  mov ah, 0
  int 16h
  jmp 0FFFFh:0

section .data
mode_info_buffer: times 256 db 0

g_M:           dd 0

saved_esp:     dd 0

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

msg_vesa_fail: db 'VESA Setup failed!', ENDL, 0
msg_reboot: db 'Press any key to reboot...', ENDL, 0
