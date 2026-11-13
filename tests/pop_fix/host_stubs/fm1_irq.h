#pragma once
#define FM1_IRQ_ALNK0 0
static inline void fm1_irq_attach(int n, void (*isr)(void), unsigned p) { (void)n; (void)isr; (void)p; }
