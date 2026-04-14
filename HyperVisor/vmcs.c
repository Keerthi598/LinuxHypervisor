#include <linux/kernel.h>
#include <asm/msr.h>
#include <asm/desc.h>
#include <asm/desc_defs.h>
#include <asm/special_insns.h>
#include "common.h"
#include "vmcs.h"
#include "vmx.h"

static uint32_t adjust_vmcs_controls(uint32_t value, uint32_t msr) {
    uint64_t msr_val;
    rdmsrl(msr, msr_val);

    value |= (uint32_t)msr_val;
    value &= (uint32_t)(msr_val >> 32);

    return value;
}

void initialize_vmcs_controls(int cpu) {
    uint64_t vmx_basic;
    uint32_t pin_msr, proc_msr, exit_msr, entry_msr;

    // bool isDebugMode = false;

    rdmsrl(MSR_IA32_VMX_BASIC, vmx_basic);

    if (vmx_basic & (1ULL << 55)) {
        pin_msr = MSR_IA32_VMX_TRUE_PINBASED_CTLS;
        proc_msr = MSR_IA32_VMX_TRUE_PROCBASED_CTLS;
        exit_msr = MSR_IA32_VMX_TRUE_EXIT_CTLS;
        entry_msr = MSR_IA32_VMX_TRUE_ENTRY_CTLS;
    }
    else {
        pin_msr = MSR_IA32_VMX_PINBASED_CTLS;
        proc_msr = MSR_IA32_VMX_PROCBASED_CTLS;
        exit_msr = MSR_IA32_VMX_EXIT_CTLS;
        entry_msr = MSR_IA32_VMX_ENTRY_CTLS;
    }

    vm_write(PIN_BASED_VM_EXEC_CONTROL, adjust_vmcs_controls(0, pin_msr));
    vm_write(CPU_BASED_VM_EXEC_CONTROL, adjust_vmcs_controls(0, proc_msr));

    vm_write(VM_EXIT_CONTROLS, adjust_vmcs_controls(1 << 9, exit_msr));
    vm_write(VM_ENTRY_CONTROLS, adjust_vmcs_controls(1 << 9, entry_msr));

    uint64_t fixed0, fixed1, cr0, cr4;

    vm_write(HOST_CR3, __read_cr3());
    
    rdmsrl(MSR_IA32_VMX_CR4_FIXED0, fixed0);
    rdmsrl(MSR_IA32_VMX_CR4_FIXED1, fixed1);
    cr4 = __read_cr4();
    cr4 |= fixed0;
    cr4 &= fixed1;
    vm_write(HOST_CR4, cr4);
    
    rdmsrl(MSR_IA32_VMX_CR0_FIXED0, fixed0);
    rdmsrl(MSR_IA32_VMX_CR0_FIXED1, fixed1);
    cr0 = read_cr0();
    cr0 |= fixed0;
    cr0 &= fixed1;
    vm_write(HOST_CRO, cr0);

    vm_write(HOST_RIP, (uint64_t)vmm_exit_handler_entry);
    vm_write(HOST_RSP, (uint64_t)g_vcpus[cpu].stack_virt + KERNEL_STACK_SIZE);

    
    vm_write(GUEST_CR3, __read_cr3());
    
    rdmsrl(MSR_IA32_VMX_CR4_FIXED0, fixed0);
    rdmsrl(MSR_IA32_VMX_CR4_FIXED1, fixed1);
    cr4 = __read_cr4();
    cr4 |= fixed0;
    cr4 &= fixed1;
    vm_write(GUEST_CR4, cr4);
    
    rdmsrl(MSR_IA32_VMX_CR0_FIXED0, fixed0);
    rdmsrl(MSR_IA32_VMX_CR0_FIXED1, fixed1);
    cr0 = read_cr0();
    cr0 |= fixed0;
    cr0 &= fixed1;
    vm_write(GUEST_CR0, cr0);

    void* guest_stack_mem = (void*)__get_free_page(GFP_KERNEL);
    vm_write(GUEST_RSP, (uint64_t)guest_stack_mem + KERNEL_STACK_SIZE);
    vm_write(GUEST_RIP, (uint64_t)guest_entry_stub);

    vm_write(GUEST_CS_SELECTOR, read_cs());
    vm_write(GUEST_DS_SELECTOR, read_ds());
    vm_write(GUEST_ES_SELECTOR, read_es());
    vm_write(GUEST_FS_SELECTOR, read_fs());
    vm_write(GUEST_GS_SELECTOR, read_gs());
    vm_write(GUEST_SS_SELECTOR, read_ss());
    vm_write(GUEST_TR_SELECTOR, read_tr());

    vm_write(HOST_CS_SELECTOR, read_cs());
    vm_write(HOST_TR_SELECTOR, read_tr());

    struct desc_ptr gdt_ptr;
    native_store_gdt(&gdt_ptr);

    struct desc_ptr idt_ptr;
    store_idt(&idt_ptr);

    vm_write(GUEST_TR_BASE, get_segment_base(gdt_ptr.address, read_tr()));
    vm_write(GUEST_TR_LIMIT, 0x67);
    vm_write(GUEST_TR_AR, 0x008B);

    vm_write(GUEST_LDTR_BASE, 0);
    vm_write(GUEST_LDTR_LIMIT, 0);
    vm_write(GUEST_LDTR_AR, 0x10000);

    vm_write(GUEST_RFLAGS, 0x02);
    vm_write(GUEST_INTERRUPTIBILITY_INFO, 0);
    vm_write(GUEST_ACTIVITY_STATE, 0);

    vm_write(GUEST_GDTR_BASE, gdt_ptr.address);
    vm_write(GUEST_GDTR_LIMIT, gdt_ptr.size);
    vm_write(GUEST_IDTR_BASE, idt_ptr.address);
    vm_write(GUEST_IDTR_LIMIT, idt_ptr.size);

    vm_write(GUEST_LDTR_AR, 0x10000);
    vm_write(GUEST_GS_AR, 0x10000);
    vm_write(GUEST_FS_AR, 0x10000);

    vm_write(GUEST_CS_AR, 0xA09B);
    vm_write(GUEST_SS_AR, 0xC093);
    vm_write(GUEST_DS_AR, 0xC093);
    vm_write(GUEST_ES_AR, 0xC093);

    vm_write(GUEST_CS_BASE, 0);
    vm_write(GUEST_SS_BASE, 0);
    vm_write(GUEST_DS_BASE, 0);
    vm_write(GUEST_ES_BASE, 0);

    vm_write(GUEST_CS_LIMIT, 0xFFFFFFFF);
    vm_write(GUEST_SS_LIMIT, 0xFFFFFFFF);
    vm_write(GUEST_DS_LIMIT, 0xFFFFFFFF);
    vm_write(GUEST_ES_LIMIT, 0xFFFFFFFF);

    vm_write(VMCS_LINK_POINTER, ~0ULL);
}


