DATA_SEGMENT64    equ 30h
CODE_SEGMENT64    equ 28h

kernel_load_vadr  equ 0xFFFFFFFF80100000

; Functions
global calc_M
global switch64

; Variables
extern g_PML4
global g_M

section .entry
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

[BITS 32]
switch64:
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

section .data
; Global variables for paging
g_M:           dd 0
