%macro x86_EnterRealMode 0
  [bits 32]
  jmp word 18h:.pmode16         ; jump to 16-bit protected mode segment

.pmode16:
  [bits 16]
  ; disable protected mode bit in CR0
  mov eax, cr0
  and al, ~1
  mov cr0, eax

  ; jump to real mode
  jmp word 00h:.rmode

.rmode:
  ; setup segments
  mov ax, 0
  mov ds, ax
  mov ss, ax

  ; enable interrupts
  sti

%endmacro

%macro x86_EnterProtectedMode 0
  cli

  ; set protection enable flag in CR0
  mov eax, cr0
  or al, 1
  mov cr0, eax

  ; far jump into protected mode
  jmp dword 08h:.pmode

.pmode:
  [bits 32]

  ; setup segment registers
  mov ax, 0x10
  mov ds, ax
  mov ss, ax

%endmacro

;
; Convert linear address to segment::offset address
; Args:
;   1 - linear address
;   2 - (out) target segment (e.g. es)
;   3 - target 32-bit register to use (e.g. eax)
;   4 - target lower 16-bit half of #3 (e.g. ax)
%macro LinearToSegOffset 4
  mov %3, %1        ; linear address to eax
  shr %3, 4
  mov %2, %4
  mov %3, %1
  and %3, 0xf       ; discard the other bits
%endmacro

global x86_outb
x86_outb:
  [bits 32]
  mov dx, [esp + 4]
  mov al, [esp + 8]
  out dx, al
  ret

global x86_inb
x86_inb:
  [bits 32]
  mov dx, [esp + 4]
  xor eax, eax
  in al, dx
  ret

;
; http://www.ctyme.com/intr/rb-0621.htm
;
global x86_Disk_GetDriveParams
x86_Disk_GetDriveParams:
  [bits 32]

  ; make new call frame
  push ebp                ; save old call frame
  mov ebp, esp            ; initialize new call frame;

  x86_EnterRealMode
  [bits 16]

  ; save regs
  push es
  push bx
  push esi
  push di


  ; call int13h
  mov dl, [bp + 8]      ; dl - disk drive
  mov ah, 08h
  mov di, 0             ; es:di - 0000:0000
  mov es, di
  stc
  int 13h

  ; out params
  mov eax, 1
  sbb eax, 0

  ; drive type from bl
  LinearToSegOffset [bp + 12], es, esi, si
  mov es:[si], bl

  ; cylinders
  mov bl, ch            ; cylinders - lower bits in ch
  mov bh, cl            ; cylinders - upper bits in cl (6-7)
  shr bh, 6
  inc bx

  LinearToSegOffset [bp + 16], es, esi, si
  mov es:[si], bx

  ; sectors
  xor ch, ch            ; sectors - lower 5 bits in cl
  and cl, 3Fh

  LinearToSegOffset [bp + 20], es, esi, si
  mov es:[si], cx

  ;heads
  mov cl, dh            ; heads - dh
  inc cx

  LinearToSegOffset [bp + 24], es, esi, si
  mov es:[si], cx

  ; restore regs
  pop di
  pop esi
  pop bx
  pop es

  ; return
  push eax

  x86_EnterProtectedMode
  [bits 32]
  
  pop eax

  ; restore old call frame
  mov esp, ebp
  pop ebp
  ret

;
global x86_Disk_Reset
x86_Disk_Reset:
  [bits 32]

  ; make new call frame
  push ebp               ; save old call frame
  mov ebp, esp            ; initialize new call frame;

  x86_EnterRealMode

  mov ah, 0
  mov dl, [bp + 8]      ; dl - drive
  stc
  int 13h

  mov eax, 1
  ; the bios sets the carry flag to 0 for success, so to invert the result set ax to 1
  ; and then subtract the carry flag by using sbb (subtract with borrow)
  sbb eax, 0             ; 1 = true, 0 = false

  push eax

  x86_EnterProtectedMode

  pop eax

  ; restore old call frame
  mov esp, ebp
  pop ebp
  ret

;
; http://www.ctyme.com/intr/rb-0621.htm
;
global x86_Disk_Read
x86_Disk_Read:
  [bits 32]

  ; make new call frame
  push ebp               ; save old call frame
  mov ebp, esp            ; initialize new call frame;

  x86_EnterRealMode

  ; save modified regs
  push bx
  push es
 
  ; setup args
  mov dl, [bp + 8]      ; dl - drive

  mov ch, [bp + 12]      ; ch - cylinder (lower 8 bits)
  mov cl, [bp + 13]      ; cl - cylinder to bits 6-7
  shl cl, 6

  mov al, [bp + 16]      ; cl - sector to bits 0-5
  and al, 3Fh           ; apply a mas so that the other bits are 0
  or cl, al

  mov dh, [bp + 20]     ; dh - head

  mov al, [bp + 24]     ; al - count

  ; es:bx pointer to data out
  LinearToSegOffset [bp + 28], es, ebx, bx

; call int13h
  mov ah, 02h
  stc
  int 13h

  mov eax, 1
  sbb eax, 0             ; 1 = true, 0 = false

  push eax

  x86_EnterProtectedMode

  pop eax

  ; restore regs
  pop es
  pop ebx

  ; restore old call frame
  mov esp, ebp
  pop ebp
  ret
