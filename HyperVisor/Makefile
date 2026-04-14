KDIR := /home/$(USER)/kernelHeaders/linux-source-6.8.0

obj-m += vnode_hv.o

vnode_hv-y := main.o memory.o vmx.o vmcs.o entry.o


all:
	make -C /lib/modules/${shell uname -r}/build M=${PWD} modules

clean:
	make -C /lib/modules/${shell uname -r}/build M=${PWD} clean
