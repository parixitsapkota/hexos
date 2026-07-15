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
  mov [drive_number], dl

  call disk_read_lba
  call clear_screen
  jmp dword 0x0000:STAGE2ADR

  ; Fallback safety (should never be reached)
  cli
  hlt
  jmp $

disk_read_lba:
  push si
  mov si, dap
  mov ah, 42h
  mov dl, [drive_number]
  int 13h
  jc .fail
  pop si
  ret
  .fail:
    pop si
    mov si, msg_disk_fail
    call puts
    call wait_key_and_reboot

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
  mov ah, 06h  ; BIOS function 06h: Scroll window up
  mov al, 0    ; AL = 0 means clear the entire screen
  mov bh, 07h  ; BH = Attribute (07h is standard light grey text on black background)
  mov ch, 0    ; CH = Upper left corner row (0)
  mov cl, 0    ; CL = Upper left corner column (0)
  mov dh, 24   ; DH = Lower right corner row (24)
  mov dl, 79   ; DL = Lower right corner column (79)
  int 10h      ; Call BIOS video interrupt
  mov ah, 01h  ; BIOS function 01h: Set cursor shape
  mov ch, 20h  ; Setting bit 5 of CH (0x20) hides the cursor
  mov cl, 00h  ; CL = bottom scanline (ignored when hiding)
  int 10h
  ret

wait_key_and_reboot:
  mov si, msg_reboot
  call puts
  mov ah, 0
  int 16h
  jmp 0FFFFh:0            ; Far jump to BIOS reset vector

drive_number: db 0

; Disk Address Packet
align 4
dap:
  db 10h        ; Size of packet
  db 0          ; Reserved
  dw 20         ; Number of sectors to read
  dw STAGE2ADR  ; Buffer offset
  dw 0x0000     ; Buffer segment
  dq 1          ; LBA (Starting at sector 1, the second sector)

msg_disk_fail: db 'Disk read failed!', ENDL, 0
msg_reboot: db 'Press any key to reboot...', ENDL, 0

times 510-($-$$) db 0
dw 0xAA55
