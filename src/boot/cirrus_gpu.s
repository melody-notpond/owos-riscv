/*
(qemu) info pci
  Bus  0, device   0, function 0:
    Host bridge: PCI device 1b36:0008
      PCI subsystem 1af4:1100
      id ""
  Bus  0, device   1, function 0:
    VGA controller: PCI device 1013:00b8
      PCI subsystem 1af4:1100
      BAR0: 32 bit prefetchable memory at 0xffffffffffffffff [0x01fffffe].
      BAR1: 32 bit memory at 0xffffffffffffffff [0x00000ffe].
      BAR6: 32 bit memory at 0xffffffffffffffff [0x0000fffe].
      id ""

static const MemMapEntry virt_memmap[] = {
    [VIRT_DEBUG] =       {        0x0,         0x100 },
    [VIRT_MROM] =        {     0x1000,        0xf000 },
    [VIRT_TEST] =        {   0x100000,        0x1000 },
    [VIRT_RTC] =         {   0x101000,        0x1000 },
    [VIRT_CLINT] =       {  0x2000000,       0x10000 },
    [VIRT_PCIE_PIO] =    {  0x3000000,       0x10000 },
    [VIRT_PLIC] =        {  0xc000000, VIRT_PLIC_SIZE(VIRT_CPUS_MAX * 2) },
    [VIRT_UART0] =       { 0x10000000,         0x100 },
    [VIRT_VIRTIO] =      { 0x10001000,        0x1000 },
    [VIRT_FW_CFG] =      { 0x10100000,          0x18 },
    [VIRT_FLASH] =       { 0x20000000,     0x4000000 },
    [VIRT_PCIE_ECAM] =   { 0x30000000,    0x10000000 },
    [VIRT_PCIE_MMIO] =   { 0x40000000,    0x40000000 },
    [VIRT_DRAM] =        { 0x80000000,           0x0 },
};
*/

.section .text
.global gpu_init

gpu_init:

    ret

