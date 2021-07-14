# cpu

参考:

- [Instruction Set Assembly Guide for Armv7 and earlier Arm architectures Reference Guide](https://developer.arm.com/documentation/100076/0200)
- [Armv7 和更早的 Arm 架构的指令集汇编指南参考指南](https://cnfatal.gitbook.io/arm-documentation/armv7-he-geng-zao-de-arm-jia-gou-de-zhi-ling-ji-hui-bian-zhi-nan-can-kao-zhi-nan)

## 硬件

架构： Arm Cortex-A7 AArch32

## 初始化

```assembly
mrs r0, CPSR            ; read current program status register
bic r0, r0, #0x1f       ; load System (ARMv4+) R0-R14, CPSR, PC as MASK
orr r0, r0, #0x13       ; set SVC mode (supervisor) R0-R12, R13_svc R14_svc CPSR, SPSR_IRQ, PC
orr r0, r0, #0x40       ; enable FIQ interrupts
bic r0, r0, #0x200      ; set little endianess
```
