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
    lea     current(%rip), %r11
    movq    (%r11), %rax

    /* store generator stack position */
    movq    %rsp, (%rax)
    /* load caller stack */
    movq    8(%rax), %rsp
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
    movq    %rsp, 8(%rdi)

    /* switch stack */
    movq    (%rdi), %rsp
    /* resume flow */
    pop     %rbp
    ret
    
    /* switch back to caller stack */
    movq    8(%rdi), %rsp
    ret
