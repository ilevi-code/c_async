.section .data
/* .global caller_stack
.global coro_stack */
caller_stack:
.quad 0
caller_bp:
.quad 0
coro_stack:
.quad 0
coro_ip:
.quad 0

.section .text
/* .global coro
 coro: */



.global yield
yield:
    /* todo save r12, r13, r14, r15, rbx, rsp, rbp */
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
    /* store caller stack in global caller variable */
    lea caller_stack(%rip), %rax
    movq    %rsp, (%rax)
    lea caller_bp(%rip), %rax
    movq    %rbp, (%rax)

    /* switch stack */
    movq    %rsi, %rsp
    /* DO i neww this?*/
    movq    %rsi, %rbp
    push %rdi
    ret
    ; callq   *%rdi
    
    /* switch back to caller stack */
    lea caller_stack(%rip), %rax
    movq (%rax), %rsp

    ret
