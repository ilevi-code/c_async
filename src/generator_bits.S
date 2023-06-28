#include "generator.h"

.section .data

.global current
.type current, object
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

    push    %rax
    /* load previous generator and store as current */
    movq    GENERATOR__PREV_GENERATOR(%rax), %rax
    movq    %rax, current(%rip)
    pop     %rax

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
    movq    GENERATOR__STATUS(%rdi), %rax
    cmp     $GEN_STATUS_DONE, %rax
    jne     not_done
    xor     %rax, %rax
    ret

not_done:
    push    %rbp

    /* store current generator as previous */
    lea     current(%rip), %rax
    movq    (%rax), %rax
    movq    %rax, GENERATOR__PREV_GENERATOR(%rdi)
    /* store current generator */
    movq    %rdi, current(%rip)
    /* store current stack in generator struct */
    movq    %rsp, GENERATOR__CALLER_STACK(%rdi)

    /* switch stack */
    movq    GENERATOR__GEN_STACK(%rdi), %rsp
    /* resume flow */
    pop     %rbp
    ret

.global gen_ret
gen_ret:
    // mark as finished
    lea     current(%rip), %rax
    movq    (%rax), %rax
    movq    $GEN_STATUS_DONE, GENERATOR__STATUS(%rax)

    // yield 0
    xor     %rdi, %rdi
    jmp yield

.global gen_entry
gen_entry:
    pop     %rax
    pop     %rdi
    call    %rax
