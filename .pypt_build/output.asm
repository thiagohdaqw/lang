format ELF64
public pypt_main
section '.text' executable align 16


; Public Functions
; End Public Functions

pypt_main:
    push rbp
    mov rbp, rsp

    sub rsp, 8
    mov rax, rsp
    push rax
    mov rax, [rbp-16]
    push rax
    mov rdx, 0
    imul rdx, 1
    pop rax
    add rax, rdx
    push rax
    mov rdx, 104
    pop rax
    mov [rax], dl
    mov rax, [rbp-16]
    push rax
    mov rdx, 1
    imul rdx, 1
    pop rax
    add rax, rdx
    push rax
    mov rdx, 101
    pop rax
    mov [rax], dl
    mov rax, [rbp-16]
    push rax
    mov rdx, 2
    imul rdx, 1
    pop rax
    add rax, rdx
    push rax
    mov rdx, 108
    pop rax
    mov [rax], dl
    mov rax, [rbp-16]
    push rax
    mov rdx, 3
    imul rdx, 1
    pop rax
    add rax, rdx
    push rax
    mov rdx, 108
    pop rax
    mov [rax], dl
    mov rax, [rbp-16]
    push rax
    mov rdx, 4
    imul rdx, 1
    pop rax
    add rax, rdx
    push rax
    mov rdx, 111
    pop rax
    mov [rax], dl
    mov rax, [rbp-16]
    push rax
    mov rdx, 5
    imul rdx, 1
    pop rax
    add rax, rdx
    push rax
    mov rdx, 10
    pop rax
    mov [rax], dl
    mov rax, [rbp-16]
    push rax

    pop rdi
    call escreva
    mov rax, 8
    push rax
    .pypt_main_while_1_cond:
        mov rax, [rbp-24]
        push rax
        mov rcx, 5
        pop rdx
        xor rax, rax
        cmp rdx, rcx
        setg ah
        cmp rax, 0
        je .pypt_main_while_1_end
    .pypt_main_while_1_body:
        mov rax, dat_0
        push rax

        pop rdi
        call escreval
        mov rax, [rbp-24]
        push rax
        mov rcx, 1
        neg rcx
        pop rdx
        add rdx, rcx
        mov [rbp-24], rdx
    jmp .pypt_main_while_1_cond
    .pypt_main_while_1_end:
    mov rcx, rax
    xor rax, rax
    mov rax, rcx
pypt_main_ret:
    mov rsp, rbp
    pop rbp
    ret

; External funcs
extrn escreva
extrn escreval

; Data section
section '.data'
    dat_0 db "ola",0

