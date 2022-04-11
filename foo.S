.section .data

caller_stack:
.quad 0
caller_bp:
.quad 0
.global coro_stack
coro_stack:
.quad 0

.section .text

.global yield
yield:
    /* todo save r12, r13, r14, r15, rbx, rsp, rbp */
    /* store current stack poistion */
    lea coro_stack(%rip), %rax
    movq %rsp, (%rax)

    lea caller_stack(%rip), %rax
    movq (%rax), %rsp
    lea caller_bp(%rip), %rax
    movq    (%rax), %rbp

    movq %rdi, %rax
    ret


.global next
next:
    /* store caller stack and base-pointer in global variable */
    lea caller_stack(%rip), %rax
    movq    %rsp, (%rax)
    lea caller_bp(%rip), %rax
    movq    %rbp, (%rax)

    /* load stack */
    lea     coro_stack(%rip), %rax
    movq    (%rax), %rsi
    /* switch stack */
    movq    %rsi, %rsp
    subq    $0x8, %rsi
    movq    %rsi, %rbp
    ret
    
    /* switch back to caller stack */
    lea caller_stack(%rip), %rax
    movq (%rax), %rsp

    ret
