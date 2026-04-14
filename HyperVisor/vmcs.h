#ifndef VMCS_H
#define VMCS_H

#include <linux/types.h>

#define VMCS_REVISION_ID_OFFSET 0

#define GUEST_ES_SELECTOR           0x00000800
#define HOST_ES_SELECTOR            0x00000C00
#define VMX_INSTRUCTION_ERROR       0x00004400

#define VIRTUAL_PROCESSOR_ID        0x00000000

#define PIN_BASED_VM_EXEC_CONTROL   0x00004000
#define CPU_BASED_VM_EXEC_CONTROL   0x00004002
#define VM_EXIT_CONTROLS            0x0000400C
#define VM_ENTRY_CONTROLS           0x00004012

#define MSR_IA32_VMX_TRUE_PINBASED_CTLS     0x48D
#define MSR_IA32_VMX_TRUE_PROCBASED_CTLS    0x48E
#define MSR_IA32_VMX_TRUE_EXIT_CTLS         0x48F
#define MSR_IA32_VMX_TRUE_ENTRY_CTLS        0x490

#define HOST_CRO            0x00006C00
#define HOST_CR3            0x00006C02
#define HOST_CR4            0x00006C04
#define HOST_RSP            0x00006C14
#define HOST_RIP            0x00006C16

#define GUEST_CR0           0x00006800
#define GUEST_CR3           0x00006802
#define GUEST_CR4           0x00006804
#define GUEST_RSP           0x0000681C
#define GUEST_RIP           0x0000681E

#define VMCS_LINK_POINTER   0x00002800

#define HOST_CS_SELECTOR    0x00000C02
#define HOST_TR_SELECTOR    0x00000C0C
#define GUEST_CS_SELECTOR   0x00000802
#define GUEST_DS_SELECTOR   0x00000806
#define GUEST_ES_SELECTOR   0x00000800
#define GUEST_FS_SELECTOR   0x00000808
#define GUEST_GS_SELECTOR   0x0000080A
#define GUEST_SS_SELECTOR   0x00000804
#define GUEST_TR_SELECTOR   0x0000080E
#define GUEST_LDTR_SELECTOR 0x0000080C

#define GUEST_TR_LIMIT      0x0000480E
#define GUEST_TR_BASE       0x00006814
#define GUEST_TR_AR         0x00004822
#define GUEST_LDTR_LIMIT    0x00004812
#define GUEST_LDTR_BASE     0x00006818
#define GUEST_LDTR_AR       0x00004820

#define GUEST_RFLAGS                0x00006820
#define GUEST_ACTIVITY_STATE        0x00004826
#define GUEST_INTERRUPTIBILITY_INFO 0x00004824

#define GUEST_GDTR_BASE     0x00006816
#define GUEST_GDTR_LIMIT    0x00004810
#define GUEST_IDTR_BASE     0x00006818
#define GUEST_IDTR_LIMIT    0x00004812

#define GUEST_LDTR_AR       0x00004820
#define GUEST_GS_AR         0x0000481E
#define GUEST_FS_AR         0x0000481C

#define GUEST_CS_AR         0x00004816
#define GUEST_SS_AR         0x00004818
#define GUEST_DS_AR         0x0000481A
#define GUEST_ES_AR         0x00004814

#define GUEST_CS_BASE       0x00006808
#define GUEST_SS_BASE       0x0000680A
#define GUEST_DS_BASE       0x0000680C
#define GUEST_ES_BASE       0x00006806

#define GUEST_CS_LIMIT      0x00004802
#define GUEST_SS_LIMIT      0x00004804
#define GUEST_DS_LIMIT      0x00004806
#define GUEST_ES_LIMIT      0x00004800





static inline void vm_write(unsigned long field, unsigned long value) {
    uint8_t error;
    asm volatile(
        "vmwrite %2, %1;"
        "setna %0"
        : "=q" (error)
        : "r" (field), "rm" (value)
        : "cc"  
    );
    if (error) {
        pr_err("VNode: VMWrite Failed!!!!! Field: 0x%lx, Value: 0x%lx\n", field, value);
    }
};

static inline unsigned long vm_read(unsigned long field) {
    unsigned long value;
    asm volatile(
        "vmread %1, %0"
        : "=rm" (value)
        : "r" (field)  
        : "cc"
    );

    return value;
}

void initialize_vmcs_controls(int cpu);

static uint64_t get_segment_base(uint64_t gdt_base, uint16_t selector) {
    struct desc_struct* gdt = (struct desc_struct*)gdt_base;
    struct desc_struct* desc = &gdt[selector >> 3];

    uint64_t base = (uint64_t)desc->base0 | ((uint64_t)desc->base1 << 16) | ((uint64_t)desc->base2 << 24);

    if (!(desc->s)) {
        uint64_t upper_base = *(uint64_t*)((uint64_t)desc + 8);
        base |= (upper_base << 32);
    }
    return base;
}

static inline uint16_t read_cs(void) { uint16_t v; asm("mov %%cs, %0" : "=r"(v)); return v;}
static inline uint16_t read_ds(void) { uint16_t v; asm("mov %%ds, %0" : "=r"(v)); return v;}
static inline uint16_t read_es(void) { uint16_t v; asm("mov %%es, %0" : "=r"(v)); return v;}
static inline uint16_t read_fs(void) { uint16_t v; asm("mov %%fs, %0" : "=r"(v)); return v;}
static inline uint16_t read_gs(void) { uint16_t v; asm("mov %%gs, %0" : "=r"(v)); return v;}
static inline uint16_t read_ss(void) { uint16_t v; asm("mov %%ss, %0" : "=r"(v)); return v;}
static inline uint16_t read_tr(void) { uint16_t v; asm("str %0" : "=r"(v)); return v;}

#endif // VMCS_H