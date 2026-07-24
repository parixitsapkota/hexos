extern g_M
global calc_M

section .text

calc_M:
	; Query CPUID for address sizes
	mov eax, 0x80000008 ; Leaf 80000008H 
	cpuid               ; Executes CPUID. Returns info in EAX, EBX, ECX, EDX
	; EAX bits 7:0 contain MAXPHYADDR (M)
	and eax, 0xFF       ; Mask out everything except the lower 8 bits of EAX
	; EAX now holds the integer value of M (e.g., 36, 40, 46, 48, or 52)
  mov [g_M], eax
  ret
