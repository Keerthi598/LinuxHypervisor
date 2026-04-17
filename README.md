## Hypervisor - x86_64

#### A Hypvervisor for x86_64, using Intel VT-x(VMX) extensions to explore hardware-assisted virtualization and low-level system state management.

### Technical Overview

This project is a deep dive into the Intel SDM Volume 3C. <br>
The goal is to establish a stable VMX environment and manage the transitions between VMX Root and Non-Root operations, allowing the hypervisor to gain complete control of a processor core. <br>
Unlike a traditional Type-2 hypervisor, this implementation allows direct hardware interaction. <br>
<br> <br>
This project is still in works. Successful entry into the vm is currently established.
