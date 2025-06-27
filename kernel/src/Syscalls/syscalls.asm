[BITS 64]

section .text
GLOBAL KiSyscallHandler
EXTERN SyscallHandler

KiSyscallHandler:
	call SyscallHandler
	iretq
