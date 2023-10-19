bits 16

section _ENTRY class=CODE

extern _cstart_             ; entry point from c
global entry

entry:
  cli
  mov ax, ds
  mov ss, ax
  mov sp, 0                 ; reset stack pointers
  mov bp, sp                ; reset base
  sti

  ; expect boot drive in dl, send it as an argument to cstart function
  xor dh, dh
  push dx
  call _cstart_

  cli
  hlt





















