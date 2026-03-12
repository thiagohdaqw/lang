format ELF64
public pypt_main

section '.text' executable align 16

pypt_main:
    push rbp
    mov rbp, rsp

    mov rcx, 3
    mov rax, rcx
    push rax
    mov rcx, 10
    mov rax, rcx
    push rax
    mov rax, dat_0
    push rax
    .pypt_main_while_1_cond:
        mov rax, [rbp-16]
        push rax
        mov rcx, 10
        pop rdx
        xor rax, rax
        cmp rdx, rcx
        sete ah
        push rax
        mov rax, [rbp-8]
        push rax
        mov rcx, 0
        pop rdx
        xor rax, rax
        cmp rdx, rcx
        setg ah
        mov rcx, rax
        pop rdx
        and rcx, rdx
        mov rax, rcx
        push rax
        mov rax, [rbp-8]
        push rax
        mov rcx, 3
        pop rdx
        xor rax, rax
        cmp rdx, rcx
        setg ah
        mov rcx, rax
        pop rdx
        or rcx, rdx
        mov rax, rcx
        cmp rax, 0
        je .pypt_main_while_1_end
    .pypt_main_while_1_body:
        mov rcx, 1
        mov rax, rcx
        push rax
        mov rax, [rbp-24]
        push rax
        mov rax, [rbp-24]
        push rax
        pop rdi
        call strlen
        push rax
        pop rdx
        pop rsi
        pop rdi
        call write
        mov rax, [rbp-8]
        push rax
        mov rcx, 1
        neg rcx
        pop rdx
        add rcx, rdx
        mov [rbp-8], rcx
    jmp .pypt_main_while_1_cond
    .pypt_main_while_1_end:
    mov rcx, 69
    mov rax, rcx
pypt_main_ret:
    mov rsp, rbp
    pop rbp
    ret

; External funcs
extrn write
extrn strlen

; Data section
section '.data'
    dat_0 db "hello",10,0

