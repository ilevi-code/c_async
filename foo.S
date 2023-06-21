#include "foo.h"

.section .data

.global current
current:
.quad 0


.section .text

.global yield
yield:
    /* store current stack poistion */
    push    %rbp
    /* load generator struct */
    lea     current(%rip), %rax
    movq    (%rax), %rax

    /* store generator stack position */
    movq    %rsp, GENERATOR__GEN_STACK(%rax)
    /* load caller stack */
    movq    GENERATOR__CALLER_STACK(%rax), %rsp
    pop     %rbp

    movq    %rdi, %rax
    ret


.global next
next:
    /* store callee-saved registers */
    push    %rbp

    /* store current generator in global variable */
    lea     current(%rip), %rax
    movq    %rdi, (%rax)
    /* store current stack in generator struct */
    movq    %rsp, GENERATOR__CALLER_STACK(%rdi)

    /* switch stack */
    movq    GENERATOR__GEN_STACK(%rdi), %rsp
    /* resume flow */
    pop     %rbp
    ret

    /* switch back to caller stack */
    movq    GENERATOR__CALLER_STACK(%rdi), %rsp
    ret
