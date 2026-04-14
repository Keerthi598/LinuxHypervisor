#include <linux/linkage.h>

.text
.global vmm_launch_failure_landing_zone
.global launch_vmm_assembly
.global vmm_exit_handler_logic

.align 16
.global vmm_exit_handler_entry
vmm_exit_handler_entry:
	pushq %r15; pushq %r14; pushq %r13; pushq %r12;
	pushq %r11; pushq %r10; pushq %r9; pushq %r8;
	pushq %rdi; pushq %rsi; pushq %rbp; pushq %rbx;
	pushq %rdx; pushq %rcx; pushq %rax;

	movq %rsp, %rdi;
	cld;
	call vmm_exit_handler_logic;

	popq %rax; popq %rcx; popq %rdx;
	popq %rbx; popq %rbp; popq %rsi; popq %rdi
	popq %r8; popq %r9; popq %r10; popq %r11;
	popq %r12; popq %r13; popq %r14; popq %r15;

	vmresume;
	nop;
	

launch_vmm_assembly:
	pushq %r15; pushq %r14; pushq %r13; pushq %r12;
	pushq %r11; pushq %r10; pushq %r9; pushq %r8;
	pushq %rdi; pushq %rsi; pushq %rbp; pushq %rbx;
	pushq %rdx; pushq %rcx; pushq %rax;

	vmlaunch;
	

.align 16
.global guest_entry_stub
guest_entry_stub:
	nop;
	jmp guest_entry_stub;
	
