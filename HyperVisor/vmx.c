#include "common.h"
#include "vmcs.h"
#include "vmx.h"
#include <linux/delay.h>


#define VM_EXIT_REASON          0x00004402
#define VM_INSTRUCTION_ERROR    0x00004400
#define EXIT_REASON_CPUID       10

void vmx_enable(void *info) {
    int cpu = smp_processor_id();
    unsigned long cr4;
    unsigned char error_flag = 0;

    cr4 = __read_cr4();
    cr4 |= X86_CR4_VMXE;
    __write_cr4(cr4);

    asm volatile (
        "vmxon %[pa];"
        "setc %0;"
        : "=q" (error_flag)
        : [pa] "m" (g_vcpus[cpu].vmxon_phys)
        : "cc", "memory"
    );

    if (g_vcpus[cpu].is_active) {
        printk(KERN_ERR "VNode: VMXON failed on CPU %d\n\n", cpu);
        g_vcpus[cpu].is_active = false;
    }
    else {
        g_vcpus[cpu].is_active = true;
        printk(KERN_INFO "VNode: VMX Root Mode active on CPU %d\n\n", cpu);
    }

    uint8_t error;
    uint64_t vmcs_pa = g_vcpus[cpu].vmcs_phys;

    asm volatile (
        "vmptrld %1; setna %0"
        : "=r" (error)
        : "m" (vmcs_pa)
        : "cc", "memory"
    );

    if (error) {
        printk(KERN_INFO "VNode: VMPTRLD Failed on CPU %d\n", cpu);
    }
    else {
        printk(KERN_INFO "VNode: VMCS loaded on CPU %d\n", cpu);

        vm_write(GUEST_ES_SELECTOR, 0);
        initialize_vmcs_controls(cpu);
    }

}

void vmx_disable(void *info) {
    int cpu = smp_processor_id();
    unsigned long cr4;

    if (g_vcpus[cpu].is_active) {
        asm volatile ("vmxoff" ::: "cc", "memory");

        cr4 = __read_cr4();
        cr4 &= ~X86_CR4_VMXE;
        __write_cr4(cr4);

        g_vcpus[cpu].is_active = false;
        printk(KERN_INFO "VNode: VMX Disabled on CPU %d\n", cpu);
    }
}


void launch_vmm(void *info) {
    int cpu = smp_processor_id();

    pr_info("VNode: Attempting VMLaunch on core %d\n", cpu);
    launch_vmm_assembly();
    pr_err("VNode: VMLaunch FAILED on Core %d!!! Error: ", cpu, vm_read(VM_INSTRUCTION_ERROR));
}

void vmm_launch_failure_landing_zone(void) {
    pr_emerg("VNode: VMLaunch Error: %lx\n", vm_read(VM_INSTRUCTION_ERROR));
}

void vmm_exit_handler_logic(uint64_t *guest_regs)
{
    uint32_t exit_reason;
    uint64_t exit_qualification;
    uint64_t guest_rip;

    pr_emerg("VMExit just occurred");

    mdelay(10);

    exit_reason = vm_read(VM_EXIT_REASON) & 0xFFFF;
    guest_rip = vm_read(GUEST_RIP);

    if (exit_reason == EXIT_REASON_CPUID) {
        /**
         * The guest tried to run CPUID
         */
        vm_write(GUEST_RIP, guest_rip + 2);
        pr_info("VNode: Handled CPUID exit at RIP: %llx\n", guest_rip);
    }
    else {
        pr_err("VNode: Unknown exit! Reason: %d, RIP: %llx\n", exit_reason, guest_rip);
    }
}


