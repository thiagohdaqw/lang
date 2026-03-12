format ELF64
public pypt_main
section '.text' executable align 16


; Public Functions
; End Public Functions

pypt_main:
    push rbp
    mov rbp, rsp

    mov rax, 3
    push rax
    mov rax, 10
    push rax
    .pypt_main_while_1_cond:
        mov rax, [rbp-8]
        push rax
        mov rdx, 0
        pop rcx
        xor rax, rax
        cmp rcx, rdx
        setg ah
        push rax
        mov rax, [rbp-16]
        push rax
        mov rdx, 0
        pop rcx
        xor rax, rax
        cmp rcx, rdx
        setg ah
        mov rdx, rax
        pop rcx
        and rcx, rdx
        mov rax, rcx
        cmp rax, 0
        je .pypt_main_while_1_end
    .pypt_main_while_1_body:
        mov rax, dat_0
        push rax
        pop rdi
        call escreval
        mov rax, [rbp-8]
        push rax
        mov rdx, 1
        neg rdx
        pop rcx
        add rcx, rdx
        mov [rbp-8], rcx
        mov rax, [rbp-16]
        push rax
        mov rdx, 1
        neg rdx
        pop rcx
        add rcx, rdx
        mov [rbp-16], rcx
    jmp .pypt_main_while_1_cond
    .pypt_main_while_1_end:
    mov rax, dat_1
    push rax
    pop rdi
    call escreval
    mov rax, 1
    push rax
    mov rdx, 2
    pop rcx
    add rcx, rdx
    mov rax, rcx
pypt_main_ret:
    mov rsp, rbp
    pop rbp
    ret

; External funcs
extrn escreval

; Data section
section '.data'
    dat_0 db "...",0
    dat_1 db "Fim",0

