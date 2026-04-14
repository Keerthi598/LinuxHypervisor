#ifndef COMMON_HEADER_H
#define COMMON_HEADER_H

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <asm/io.h>
#include <asm/msr.h>
#include <asm/processor.h>

#define KERNEL_STACK_SIZE 16384 // 16KB

typedef struct _VCPU_RESOURCES {
    int cpu_id;
    uint32_t vmx_revision_id;

    void* vmxon_virt;
    uint64_t vmxon_phys;

    void* vmcs_virt;
    uint64_t vmcs_phys;

    void* stack_virt;
    // void* guest_stack_

    bool is_active;
} VCPU_RESOURCES;

extern VCPU_RESOURCES* g_vcpus;
extern int g_nr_cpus;

int allocate_vmm_context(void);
void free_vmm_context(void);
void vmx_enable(void *info);
void vmx_disable(void *info);
void launch_vmm(void *info);

#endif // COMMON_HEADER_H
