/* SPDX-License-Identifier: GPL-3.0-only
 * ROM CPU1 entry; based on JIANT FM1 (Juan Ignacio Antonucci), after
 * Melodee and X0X, GPL-3.0. Core1 has independent stack/interrupt bank. */
	.section .text.ru_c1_entry,"ax",@progbits
	.globl ru_cpu1_entry
ru_cpu1_entry:
	cli
	icfg = 0
	usp = _ru_c1_ustack_top
	sp = _ru_c1_sstack_top
	ssp = _ru_c1_sstack_top
	reti = ru_cpu1_worker
	rti
