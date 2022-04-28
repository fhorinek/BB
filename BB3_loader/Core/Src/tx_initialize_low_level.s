# 1 "../Core/Src/tx_initialize_low_level.S"
# 1 "<built-in>"
# 1 "<command-line>"
# 31 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 32 "<command-line>" 2
# 1 "../Core/Src/tx_initialize_low_level.S"
# 432 "../Core/Src/tx_initialize_low_level.S"
@
@
@
@
@
@
@
@
@
@
@
@
@
@
@
@
@
@
@
@
@
@
@
    .global _tx_thread_system_stack_ptr
    .global _tx_initialize_unused_memory
    .global __RAM_segment_used_end__
    .global _tx_timer_interrupt
    .global __main
    .global __tx_SVCallHandler
    .global __tx_PendSVHandler
    .global _vectors
    .global __tx_NMIHandler @ NMI
    .global __tx_BadHandler @ HardFault
    .global __tx_SVCallHandler @ SVCall
    .global __tx_DBGHandler @ Monitor
    .global __tx_PendSVHandler @ PendSV
    .global __tx_SysTickHandler @ SysTick
    .global __tx_IntHandler @ Int 0
@
@

SYSTEM_CLOCK = 280000000
SYSTICK_CYCLES = ((SYSTEM_CLOCK / 100) -1)

    .text 32
    .align 4
    .syntax unified
@
@
@
@
@
@
@
@
@
@
@
@
@
@
@
@
@
@
@
@
@
@
@
@
@
@
@
@
@
@
@
@
@
@
@
@
@
@
@
@
@
@
@
@
@
@VOID _tx_initialize_low_level(VOID)
@{
    .global _tx_initialize_low_level
    .thumb_func
_tx_initialize_low_level:
@
@
@
    CPSID i
@
@
@






@
@
@
    MOV r0, #0xE000E000 @ Build address of NVIC registers
    LDR r1, =g_pfnVectors @ Pickup address of vector table
    STR r1, [r0, #0xD08] @ Set vector table address
@
@
@
    LDR r0, =_tx_thread_system_stack_ptr @ Build address of system stack pointer
    LDR r1, =g_pfnVectors @ Pickup address of vector table
    LDR r1, [r1] @ Pickup reset stack pointer
    STR r1, [r0] @ Save system stack pointer
@
@
@
    LDR r0, =0xE0001000 @ Build address of DWT register
    LDR r1, [r0] @ Pickup the current value
    ORR r1, r1, #1 @ Set the CYCCNTENA bit
    STR r1, [r0] @ Enable the cycle count register
@
@
@
    MOV r0, #0xE000E000 @ Build address of NVIC registers
    LDR r1, =SYSTICK_CYCLES
    STR r1, [r0, #0x14] @ Setup SysTick Reload Value
    MOV r1, #0x7 @ Build SysTick Control Enable Value
    STR r1, [r0, #0x10] @ Setup SysTick Control
@
@
@
    LDR r1, =0x00000000 @ Rsrv, UsgF, BusF, MemM
    STR r1, [r0, #0xD18] @ Setup System Handlers 4-7 Priority Registers

    LDR r1, =0xFF000000 @ SVCl, Rsrv, Rsrv, Rsrv
    STR r1, [r0, #0xD1C] @ Setup System Handlers 8-11 Priority Registers
                                                    @ Note: SVC must be lowest priority, which is 0xFF

    LDR r1, =0x40FF0000 @ SysT, PnSV, Rsrv, DbgM
    STR r1, [r0, #0xD20] @ Setup System Handlers 12-15 Priority Registers
                                                    @ Note: PnSV must be lowest priority, which is 0xFF
@
@
@
    BX lr
@}
@

@
@
    .global __tx_BadHandler
    .thumb_func
__tx_BadHandler:
    B __tx_BadHandler

@

    .global __tx_HardfaultHandler
    .thumb_func
__tx_HardfaultHandler:
    B __tx_HardfaultHandler

@

    .global __tx_SVCallHandler
    .thumb_func
__tx_SVCallHandler:
    B __tx_SVCallHandler

@
    .global __tx_IntHandler
    .thumb_func
__tx_IntHandler:
@ VOID InterruptHandler (VOID)
@ {
    PUSH {r0, lr}




@
@




    POP {r0, lr}
    BX LR
@ }

@
    .global __tx_SysTickHandler
    .global SysTick_Handler
    .thumb_func
__tx_SysTickHandler:
    .thumb_func
SysTick_Handler:
@ VOID TimerInterruptHandler (VOID)
@ {
@
    PUSH {r0, lr}



    BL _tx_timer_interrupt



    POP {r0, lr}
    BX LR
@ }

@
    .global __tx_NMIHandler
    .thumb_func
__tx_NMIHandler:
    B __tx_NMIHandler

    .global __tx_DBGHandler
    .thumb_func
__tx_DBGHandler:
    B __tx_DBGHandler
