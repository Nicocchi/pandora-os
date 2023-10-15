org 0x7c00
bits 16

; nasm macro for the hex codes
%define ENDL 0x0D, 0x0A

start:
  jmp main

;
; Prints a string to the screen.
; Prints characters until it encounters a null character
; Params:
;   - ds:si points to string
;
puts:
  ; save registers we will modify
  push si
  push ax

.loop:
  lodsb                 ; loads next character in al
  or al, al             ; verify if next character is null?
  jz .done

  mov ah, 0x0e          ; call bios interrupt
  mov bh, 0
  int 0x10

  jmp .loop

.done:
  pop ax
  pop si
  ret
  

main:

  ; setup data segments
  mov ax, 0     ; can't write to ds/es directly
  mov ds, ax

  ; setup stack - set stack segment to 0 and a stack pointer to the beginning of the
  ; program.
  mov ss, ax
  mov sp, 0x7C00  ; stack grows downwards from where we are loaded in memory

  ; print message
  mov si, msg_hello
  call puts

  hlt

; in certain cases the CPU can start executing again so run an infinite loop
.halt:
  jmp .halt

msg_hello: db 'Hello world!', 0

; fill up 512 bytes
times 510-($-$$) db 0 ;$-$$ is the size of the prgram so far in bytes

; declare signature
dw 0AA55h
