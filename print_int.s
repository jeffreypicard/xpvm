#
# print_int.s
#
# Intel IA-32 (Linux) assembler source for the print_int native function call.
#
# Author: Jeffrey Picard
#
  .text
  .align 4
  .globl print_int_a
print_int_a:
  pushl %ebp          # Save the old frame pointer.
  movl  %esp,%ebp     # Establish new frame pointer.
#
  movl  $1,%ecx       # Initialize loop variable.
  movl  8(%ebp),%edx  # Pull reg array off the stack.
  addl  $8,%edx       # Start by referencing reg 1.
#
.L1:                  # Label to return to start of loop.
  cmp   12(%ebp),%ecx # Compare loop variable against number of args.
  jg    .L2           # Branch if %ecx > num args.
  #pushl 4(%edx)      # Push current register (64 bits) onto the stack.
  pushl (%edx)        # Unfortunately I must assume everything is 32bits.
  addl  $1,%ecx       # Increment loop variable.
  addl  $8,%edx       # Point to next register.
  jmp   .L1           # Loop.
#
.L2:
  call  printf        # Call the native function. 
#
  movl  12(%ebp),%ecx # Get number of args.
  movl  $4,%edi       # Size of arguments.
  imull %ecx,%edi     # Total amount to remove from stack.
  addl  %edi,%esp     # Remove args from stack.
#
  popl  %ebp          # Restore the frame pointer.
  ret                 # Return to caller

