#
# implementation of the parallel pi computation in vm520 assembler
#
# number of intervals must be evenly divisible by number of processors
#
# machine only supports single precision so cannot get very accurate value
# for pi.

#############################################################################
# computational core
#

# figure out how much work (number of intervals) to do
  getpn r0
  load r2, intervals
  divi r2, r0
  store r2, chunk        # chunk = number of intervals to process

# figure out where to start on the x-axis
#   this is a pain because there is no instruction to convert int to float
#   so: start_x = (chunk * processor_id + 0.5) * width
#   re-write to: start_x = (chunk * processor_id * width) + (0.5 * width)
#   and implement (chunk * processor_id * width) by repeated addition
#
  getpid r0
  load r1, chunk
  muli r0, r1            # r0 = chunk * processor_id
  ldimm r1, 0            # r1 = 0
  load r2, width         # r2 = width
  ldimm r3, 0            # r3 is the loop bound
  ldimm r4, 1            # r4 used to decrement loop index
lab1:                    # while (r0 > 0)
  beq r0, r3, lab2       #
  addf r1, r2            #   r1 += width
  subi r0, r4            #   r0 -= 1
  jmp lab1               # end while 
lab2:                    # now r1 = chunk * processor_id * width
  load r3, oneHalf       # r3 = 0.5
  mulf r2, r3            # r2 = (width * 0.5)
  addf r1, r2            # r1 = (chunk * processor_id * width) + (0.5 * width)

# iterate over the chunk of intervals summing f(x) = 4.0/(1.0 + x^2)
                         # r1 contains the initial x value
  load r0, chunk         # r0 = chunk (ie loop index)
  ldimm r2, 0            # r2 = 0 (loop bound)
  ldimm r5, 0            # r5 = 0 (sum)
  load r6, width         # r6 = width
  ldimm r7, 1            # r7 = 1 (to decrement loop index)
lab3:                    # while (r0 > r2)
  beq r0, r2, lab4
  ldimm r3, 0            #   r3 = x
  addf r3, r1
  mulf r3, r1            #   r3 = x * x
  load r4, one           #   r4 = 1.0
  addf r4, r3            #   r4 = 1.0 + (x * x)
  load r3, four          #   r3 = 4.0
  divf r3, r4            #   r3 = 4.0 / (1 + (x * x))
  addf r5, r3            #   sum += 4.0 / (1 + (x * x))
  addf r1, r6            #   x += width
  subi r0, r7            #   r0 -= 1
  jmp lab3               # end while
lab4:
 
# multiply the local sum by width
  load r0, width
  mulf r5, r0
 
# sum local answer into the answer word
#   use a lock to protect this update when multiple processors
#   busy wait on the lock
  ldimm r1, 1            # want to set the lock to 1
tryAgain:
  ldimm r2, 0            # need to wait until lock is 0
  cmpxchg r2, r1, lock   # if lock is 0 then lock it
  beq r2, r1, tryAgain   # else r2 will be set to 1, and if so repeat
  load r0, answer        # now have exclusive control
  addf r0, r5            # so safe to add in the local answer
  store r0, answer
  ldimm r2, 0            # unlock
  store r2, lock

# all done!
  halt

#############################################################################
# variables

# number of intervals per processor
chunk:
  word 0

# lock for exclusive access to the word to contain the answer
#   0 means lock is available
#   1 means it is locked
lock:
  word 0

#############################################################################
# constants and variables to be initialized via putWord

# 4.0
export four
four:
  word 0

# 1.0
export one
one:
  word 0

# 0.5
export oneHalf
oneHalf:
  word 0

# number of intervals to divide the x-axis on [0,1]
export intervals
intervals:
  word 0

# 1.0/intervals
export width
width:
  word 0

export answer
answer:
  word 0

