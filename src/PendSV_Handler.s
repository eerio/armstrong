.syntax unified
.arch armv6-m

.text

.extern next_task
.globl PendSV_Handler

.thumb_func
PendSV_Handler:
	//cpsid i
	push {r4, lr}
	bl led_on_delay
	bl led_off_delay
	//cpsie i
	pop {r4, pc}

