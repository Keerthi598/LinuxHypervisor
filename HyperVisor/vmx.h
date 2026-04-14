#ifndef VMX_H
#define VMX_H

void vmx_enable(void *info);
void vmx_disable(void* info);

// asmlinkage noinline void vmm_exit_handler(void);

extern void vmm_exit_handler_entry(void);

void vmm_exit_handler_logic(uint64_t *guest_regs);

void guest_entry_stub(void);

extern void launch_vmm_assembly(void);

#endif // VMX_H