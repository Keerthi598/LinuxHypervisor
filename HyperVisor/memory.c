#include "common.h"

VCPU_RESOURCES* g_vcpus = NULL;
int g_nr_cpus = 0;

int allocate_vmm_context(void) {
    uint64_t vmx_basic;
    int i;

    g_nr_cpus = num_online_cpus();
    g_vcpus = kzalloc(sizeof(VCPU_RESOURCES) * g_nr_cpus, GFP_KERNEL);
    if (!g_vcpus) return -ENOMEM;

    rdmsrl(MSR_IA32_VMX_BASIC, vmx_basic);

    for (i = 0; i < g_nr_cpus; i++) {
        g_vcpus[i].cpu_id = i;
        g_vcpus[i].vmx_revision_id = (uint32_t)vmx_basic;

        g_vcpus[i].vmxon_virt = (void*)__get_free_pages(GFP_KERNEL | __GFP_ZERO, 0);
        if (!g_vcpus[i].vmxon_virt) goto fail;
        g_vcpus[i].vmxon_phys = virt_to_phys(g_vcpus[i].vmxon_virt);
        *(uint32_t*)g_vcpus[i].vmxon_virt = g_vcpus[i].vmx_revision_id;

        g_vcpus[i].vmcs_virt = (void*)__get_free_pages(GFP_KERNEL | __GFP_ZERO, 0);
        if (!g_vcpus[i].vmcs_virt) goto fail;
        g_vcpus[i].vmcs_phys = virt_to_phys(g_vcpus[i].vmcs_virt);
        *(uint32_t*)g_vcpus[i].vmcs_virt = g_vcpus[i].vmx_revision_id;

        g_vcpus[i].stack_virt = (void*)__get_free_pages(GFP_KERNEL, get_order(KERNEL_STACK_SIZE));
        if (!g_vcpus[i].stack_virt) goto fail;
        memset(g_vcpus[i].stack_virt, 0, KERNEL_STACK_SIZE);
    }
    
    return 0;

fail:
    free_vmm_context();
    return -ENOMEM;
}

void free_vmm_context(void) {
    if (!g_vcpus) return;
    for (int i = 0; i < g_nr_cpus; i++) {
        if (g_vcpus[i].vmxon_virt) free_pages((unsigned long)g_vcpus[i].vmxon_virt, 0);
        if (g_vcpus[i].vmcs_virt) free_pages((unsigned long)g_vcpus[i].vmcs_virt, 0);
        if (g_vcpus[i].stack_virt) free_pages((unsigned long)g_vcpus[i].stack_virt, get_order(KERNEL_STACK_SIZE));
    }

    kfree(g_vcpus);
    g_vcpus = NULL;
}