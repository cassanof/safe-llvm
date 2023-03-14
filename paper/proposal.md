---
title: "SafeLLVM: A ROP Gadget-Free LLVM x64 Backend"
subtitle: "Project Proposal"
author: Federico, Charlie, Jake
geometry: "left=3cm,right=3cm,top=3cm,bottom=3cm"
output: pdf_document
---

#### Group

- Federico Cassano: cassano.f@northeastern.edu
- Charlie Bershatsky: bershatsky.c@northeastern.edu
- Jake Ginesin: ginesin.j@northeastern.edu

### Problem We Are Trying To Solve

Memory corruption vulnerabilities are critical vulnerabilities that may allow an attacker to gain control of a system remotely. These vulnerabilities
are often caused by buffer overflows, which are caused by a program writing more data to a buffer than it is allocated to hold. Additionally, one
may be able to trigger this kind of vulnerability via `printf`, `scanf`, and other functions that read and write to memory. Furthermore, Chrome engineers
stated that 70% of their security bugs were caused by memory corruption vulnerabilities,
which is a significant number.
Often attackers utilize shellcode to gain control of a system. Shellcode is a small program that
is composed by bytecode instructions that run on the target system. These instructions are often executed by the stack.
In August of 2004, the Linux kernel released a patch that included an option
to disallow execution of code on the stack. This option is often referred to as DEP or NX.
This option prevents the execution of shellcode directly given by an attacker.
However, a few years prior to this patch, a technique called ret2libc was developed, which
allows an attacker to execute shellcode indirectly without
having to write it directly to the stack. The technique involved overwriting the return address
of a function with the address of another function in `libc`, often the
`system()` function.
Yet, this technique does not allow the attacker to execute complex
shellcode, as the attacker is only limited to the functions in `libc`.
A new technique called ROP (Return Oriented Programming) was developed to overcome this limitation.
This technique involves chaining together gadgets, which are small pieces of code that
perform one or more instructions, and are appended with either
the `ret` instruction or a `jmp` instruction. The `ret` instruction returns to the address
stored on the stack, while the `jmp` instruction jumps to the address stored in a register.
However, for this technique to work, the binary needs to contain a
sufficient number of gadgets that can be chained together to execute the desired shellcode.
The problem we are trying to solve is to create a compiler that will
minimize the number of gadgets in a binary, thus making it harder for an attacker to perform a ROP-based memory corruption exploit.

### Context

StackGuard is the first technique that was developed to prevent buffer overflows, including ROP.
It inserts a canary value on the stack, which is a random value that is generated at runtime and stored
at the `fs:0x28` address.
The canary value is placed right before the return address of a function, and is checked before the return instruction of a function is executed.
If the canary value is not the same as the one stored at `fs:0x28`, then the program will exit,
reporting a stack smashing error.
This approach is great for preventing buffer overflows if the attacker
does not know the canary value (e.g. has a memory read primitive as well),
as the attacker will just be able to overwrite the return address of the function, and overwrite the canary value with the same value.

PIE (Position Independent Executables) is a technique that was developed to prevent ROP attacks,
as it scrambles the addresses of the functions in the binary
every time the binary is executed. This makes it harder for an attacker
to find the offset of a function in the binary, as the offset will be different every time the binary is executed.
Similarly to StackGuard, this approach is great for preventing ROP attacks if the attacker
does not know the addresses of the functions in the binary
at runtime (e.g. has a memory read primitive as well),
as the attacker can look-up the addresses of the functions in the running
binary before executing the ROP attack.

Finally, GFree is a series of techniques that were developed specifically to
defeat ROP attacks by eliminating the need for gadgets in the binary.
GFree seeks to protect free-branch instructions, which are instructions that
can be used to end a gadget sequence. These instructions include `ret`, `jmp` and `call`. Furthermore, the framework utilizes code rewriting to
eliminate unaligned free-branch instructions contained in
multi-byte instructions, such as `mov`, `add`, `sub`, etc.
This framework targets 32-bit x86 binaries, and is composed of 8 techniques:

1. A variant of StackGuard, that encrypts the return address of a function with a random key, and decrypts it before the return instruction is executed.
2. Alignment sleds that are inserted before every free-branch instructions,
   such that unaligned branches become aligned branches, and thus cannot be used as gadgets.
3. Frame cookies to prevent the attacker from exploiting `jmp` and `call` instructions.
4. Instruction transformation to prevent the attack from utilizing a multi-byte instruction containing a free-branch instruction.
5. Jump offset adjustment to prevent the attacker from utilizing a `jmp` instruction with an offset
   containing a free-branch instruction.
6. Immediate displacement reconstructions to prevent the attacker from utilizing an instruction where the immediate value encodes
   a free-branch instruction.
7. Inter-instruction barriers to prevent the use of `jmp`+`call` gadgets.

### Approach and Scope

We will be recreating parts of the GFree framework, but for 64-bit x86 binaries, using
LLVM as our compiler. We decided to use LLVM as our compiler because this
would allow us to target multiple languages, such as C, C++, and Rust.
More specifically, we will be adding a target backend to LLVM 10.0.1.
Furthermore, we decided to target 64-bit x86 binaries because this is the
most common architecture used in modern computers, and the most
studied modern architecture for ROP attacks.
The features we will be implementing are:

1. Alignment sleds
2. Return address encryption
3. Instruction rewriting
4. Immediate displacement reconstructions

Our compiler will be available to download on GitHub: https://github.com/cassanof/safe-llvm

### Evaluation

We will evaluate our approach on existing ROP attacks used in the wild,
and see if our newly compiled binaries are still vulnerable to these attacks.
Furthermore, we plan to compile glibc or musl libc with our compiler,
and see if all the functions in the libraries are still executable.
