/* SPDX-License-Identifier: GPL-3.0-only
 * Experimental CPU1 exception return; based on the JIANT FM1 multicore
 * vector (Juan Ignacio Antonucci) and original Felucca HAL (Leo Kuroshita).
 * Only the --dual-core-lab build links this file. */
	.section .text.fm1vec,"ax",@progbits
	.globl fm1_fatal_stubs
	.balign 2
fm1_fatal_stubs:
	.rept 128
	[--sp] = {rets}
	call fm1_fatal_common
	.endr
	.section .text.fm1vec_common,"ax",@progbits
	.globl fm1_fatal_common
fm1_fatal_common:
	trigger
	[--sp] = {sp, ssp, usp, icfg, psr, rets, retx, rete, reti}
	[--sp] = {r15-r0}
	r0 = sp
	call ru_cpu1_fault_dispatch
	/* Returns ONLY after the other core's fault; interrupted CPU0 resumes.
	 * Native faults take the existing fm1_fault_c recovery, never here. */
	r0 = [sp+64]
	r1 = [sp+100]
	r2 = [sp+80]
	[sp+92] = r0
	[sp+96] = r1
	[sp+100] = r2
	{r15-r0} = [sp++]
	sp += 28
	{psr, rets, reti} = [sp++]
	csync
	rti
