# Global Descriptor Table

Source: [OSDev Global Descriptor Table](https://wiki.osdev.org/Global_Descriptor_Table)

---

## GDT
**GDT** is a binary data structure specific to the `IA_32` and `x86_64` architectures. It contains entries telling the CPU about memory segments. While the following GDT structure table is completely correct for 32-bit Protected Mode, 64-bit Long Mode ignores almost all of it.

### GDT Structure

| Bit       | Name   | Inner Bits |
| --------- | ------ | ---------- |
| **0:15**  | Limit  | **0:15**   |
| **15:31** | Base   | **0:15**   |
| **32:39** | Base   | **16:23**  |
| **40:47** | Access | **0:7**    |
| **48:51** | Limit  | **16:19**  |
| **52:55** | Flags  | **0:3**    |
| **56:63** | Base   | **24:31**  |

- **Base**: A 32-bit value containing the linear address where the segment begins.
- **Limit**: A 20-bit value, tells the maximum addressable unit, either in 1 byte units, or in `4KiB` pages. Hence, if you choose page granularity and set the **Limit** value to `0xFFFFF` the segment will span the full `4GiB` address space in 32-bit mode.

#### Access byte structure

| Bit     | Name                | Discription                                                                                                                |
| ------- | ------------------- | ---- |
| **0**   | Accessed            | The CPU will set it when the segment is accessed unless set in advance.                                                    |
| **1**   | Read/Write          | If clear data segments, write access and for code segment, read access, for current segment is not allowed.                |
| **2**   | Direction Privilege | If clear code in this segment can only be executed from the ring set in `Direction` else lower rings can execute the code. |
| **3**   | Descriptor Type     | If set  it defines a code segment which can be executed                                                                    |
| **4**   | Executable          | If set it defines a code or data segment.                                                                                  |
| **5:6** | Direction           | **0** : highest privilege, **3** :lowest privilege.                                                                        |
| **7**   | Present             | Must be set set for any valid segment.                                                                                     |

#### flag nibble structure

| Bit   | Name        | Discription                                                                                                                    |
| ----- | ----------- | -------- |
| **0** | Reserved    | Reserved by the CPU                                                                                                            |
| **1** | LM code     | If set, the descriptor defines a 64-bit code segment                                                                           |
| **2** | Size        | If clear, the descriptor defines a 16-bit protected mode segment. If set it defines a 32-bit protected mode segment.           |
| **3** | Granularity | If clear, the **Limit** is in `1Byte` blocks (byte granularity). If set, the **Limit** is in `4KiB` blocks (page granularity). |

---
# Paging

Source: [Intel® 64 and IA-32 Architectures Software Developer's Manual Volume 3A](https://www.intel.com/content/www/us/en/content-details/922487/intel-64-and-ia-32-architectures-software-developer-s-manual-volume-3a-system-programming-guide-part-1.html)

---

## Enabling Paging (From 32-bits for 64-bits)

Enabling Paging with PCIDE requires to:

- set `CR4.PAE` bit 5
- clear `CR4.LA57` bit 12
- set `CR4.PCID` bit 17
- set `IA32_EFER.LME` bit 8
- Load `PLM4` into `CR3`

Example to enable paging for 64-Bit mode:

```asm
[BITS 32]
ENABLE_PAGING:
	; Set CR4.PAE        : (Bit 5) = 1
	; Clear CR4.LA57     : (Bit 12) = 0
	; Clear CR4.PCIDE    : (Bit 17) = 1
	mov eax, cr4         ; Read current CR4 value
	bts eax, 5           ; Bit Test and Set: PAE (Bit 5) to 1
	btr eax, 12          ; Bit Test and Reset: LA57 (Bit 12) to 0
	bts eax, 17          ; Bit Test and Set: PCIDE (Bit 7) to 1
	mov cr4, eax         ; Write updated value back to CR4

	; Set IA32_EFER.LME  : (Bit 8) = 1
	mov ecx, 0xC0000080  ; Load IA32_EFER MSR index into ECX
	rdmsr                ; Read MSR into EDX:EAX (Lower 32 bits are in EAX)
	bts eax, 8           ; Bit Test and Set: LME (Bit 8) to 1
	wrmsr                ; Write updated EDX:EAX back to IA32_EFER MSR

	; load PML4 into CR3 (PML4 should be under 4G or this fails/truncates in 64-Bit initialization)
	mov eax, PML4        ; Load PML4 structure into EAX
	mov cr3, eax         ; Write updated EAX to CR3

	; Set CR0.PG         : (Bit 31) = 1
	mov eax, cr0         ; Read current CR0 value
	bts eax, 31          ; Bit Test and Set: PG (Bit 31) to 1
	mov cr0, eax         ; Write updated value back to CR0

	mov ax, DATA_SEGMENT
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	; Reload CS with a 64-bit code selector by performing a long jmp
	jmp CODE_SEGMENT:RELOAD_CS

	[BITS 64]
	RELOAD_CS:
		hlt
		jmp RELOAD_CS
		; 64-Bit Paging is now enabled
```

> 4-level paging uses `CR0.WP`, `CR4.PGE`, `CR4.PCIDE`, `CR4.SMEP`, `CR4.SMAP`, `CR4.PKE`, `CR4.CET`, `CR4.PKS`, and `IA32_EFER.NXE`

---

## Paging Structure

```
[PML4] ---> [PDPT] ---> [PD] ---> [PT] ---> [Memory]
```

### Use of `CR3` with Ordinary 4-Level Paging

Ordinary 4-level paging translate linear addresses using a hierarchy of in-memory paging structures located using the contents of `CR3`, which is used to locate the first paging `PML4` structure. Use of `CR3` with 4-level paging depends on whether `process-context identifiers` have been enabled by setting `CR4.PCIDE`:

#### `PML4` Paging structures

**Note 1:** If `EPT` is not enabled, `M` is an abbreviation for `MAXPHYADDR`. If `EPT` is enabled, M represents the value returned by `CPUID.80000008H:EAX[7:0]`.
**Note 2:** `LAM` is not a paging feature.

#### 4-Level Paging with `CR4.PCIDE` clear

| Bit          | sname | Contents                                                                                                                          |
| :----------- | :---- | :-------------------------------------------------------------------------------------------------------------------------------- |
| **2:0**      | -     | Usable by the OS                                                                                                                  |
| **3**        | PWT   | Page-level write-through; indirectly determines the memory type used to access the `PML4` table linear-address translation        |
| **4**        | PCD   | Page-level cache disable; indirectly determines the memory type used to access the `PML4` table during linear-address translation |
| **11:5**     | -     | Usable by the OS                                                                                                                  |
| **`M`–1:12** | ADR   | Physical address of the 4-KByte aligned `PML4` table used for linear-address translation                                          |
| **`M`:60**   | -     | Reserved (must be 0)                                                                                                              |
| **61**       | LAM57 | Enables `LAM57` for user pointers;                                                                                                |
| **62**       | LAM48 | Enables `LAM48` for user pointers; ignored if bit 61 is set.                                                                      |
| **63**       | -     | Reserved (must be 0)                                                                                                              |

#### 4-Level Paging with `CR4.PCIDE` set

| Bit          | sname | Contents                                                                               |
| :----------- | :---- | :------------------------------------------------------------------------------------- |
| **0:11**     | PCID  | PCID                                                                                   |
| **12:`M`-1** | ADR   | Physical address of the 4-KByte aligned PML4 table used for linear-address translation |
| **`M`:60**   | -     | Reserved (must be 0)                                                                   |
| **61**       | LAM57 | Enables `LAM57` for user pointers                                                      |
| **62**       | LAM48 | Enables `LAM48` for user pointers; ignored if bit 61 is set.                           |
| **63**       | -     | Reserved (must be 0)                                                                   |

Calculating `M`:

```asm
[BITS 32]
CALC_M:
	; Query CPUID for address sizes
	mov eax, 0x80000008 ; Leaf 80000008H 
	cpuid               ; Executes CPUID. Returns info in EAX, EBX, ECX, EDX
	; EAX bits 7:0 contain MAXPHYADDR (M)
	and eax, 0xFF       ; Mask out everything except the lower 8 bits of EAX
	; EAX now holds the integer value of M (e.g., 36, 40, 46, 48, or 52) 
```

