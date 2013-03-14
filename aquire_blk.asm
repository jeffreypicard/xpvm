;
; aquire_blk.asm
;
; Assembly function to attempt to aquire a block.
; 
; Author: Jeffrey Picard
;

section .text
  global aquire_blk_asm

aquire_blk_asm:
  push  rbp                   ; Save old frame pointer
  mov   rbp, rsp              ; Establish new frame pointer
;
  push  r12                   ; Save callee saved registers
  push  r13
  push  r14
  push  r15
; 
  mov   r12, rdi              ; &owner
  mov   r13, rsi              ; newowner
;
  xor   rax, rax              ; owner should be NULL or it's owned and this fails
  lock  cmpxchg [r12], r13    ; owner = newowner
  jnz   fail
  mov   rax, $1
  jmp   success
;
fail:
  mov   rax, $0
success:
;
  pop   r15                   ; Return callee saved registers
  pop   r14
  pop   r13
  pop   r12
;
  pop rbp                     ; Re-establish old frame pointer
  ret                         ; Return
