#include "common.h"
#include <linux/delay.h>

int my_init(void) {
    int target_cpu = 3;

    int ret;
    printk(KERN_INFO "VNode: Initializing Hypervisor...\n");

    ret = allocate_vmm_context();
    if (ret < 0) return ret;

    smp_call_function_single(target_cpu, vmx_enable, NULL, 1);

    mdelay(1000);

    smp_call_function_single(target_cpu, launch_vmm, NULL, 1);

    return 0;
}

void my_exit(void) {
    on_each_cpu(vmx_disable, NULL, 1);

    mdelay(10);

    if (g_vcpus)
        free_vmm_context();
        
    printk(KERN_INFO "Vnode: Hypervisor uninstalled cleanly.\n");
}


module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("It's a me a Mario");