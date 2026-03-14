format ELF64
public pypt_main
section '.text' executable align 16


; Public Functions
; End Public Functions

func_fatorial:
    push rbp
    mov rbp, rsp

    mov rax, rdi
    push rax
    mov rax, 1
    push rax
    mov rax, 1
    push rax
    .fatorial_while_1_cond:
        mov rax, [rbp-24]
        push rax
        mov rcx, [rbp-8]
        pop rdx
        xor rax, rax
        cmp rdx, rcx
        setle ah
        cmp rax, 0
        je .fatorial_while_1_end
    .fatorial_while_1_body:
        mov rax, [rbp-16]
        push rax
        mov rcx, [rbp-24]
        pop rdx
        imul rdx, rcx
        mov [rbp-16], rdx
        mov rax, [rbp-24]
        push rax
        mov rcx, 1
        pop rdx
        add rdx, rcx
        mov [rbp-24], rdx
    jmp .fatorial_while_1_cond
    .fatorial_while_1_end:
    mov rcx, [rbp-16]
    xor rax, rax
    mov rax, rcx
    jmp func_fatorial_ret
func_fatorial_ret:
    mov rsp, rbp
    pop rbp
    ret
    ; END func_fatorial

pypt_main:
    push rbp
    mov rbp, rsp

    lea rax, [dat_0]
    push rax

    mov rax, 1
    push rax

    pop rdi
    call func_fatorial
    push rax

    pop rsi
    pop rdi
    mov rax, rsp
    sub rsp, 16
    and rsp, -16
    push 0
    push rax
    call escrevaf
    pop rsp
    lea rax, [dat_1]
    push rax

    mov rax, 2
    push rax

    pop rdi
    call func_fatorial
    push rax

    pop rsi
    pop rdi
    mov rax, rsp
    sub rsp, 16
    and rsp, -16
    push 0
    push rax
    call escrevaf
    pop rsp
    lea rax, [dat_2]
    push rax

    mov rax, 3
    push rax

    pop rdi
    call func_fatorial
    push rax

    pop rsi
    pop rdi
    mov rax, rsp
    sub rsp, 16
    and rsp, -16
    push 0
    push rax
    call escrevaf
    pop rsp
    lea rax, [dat_3]
    push rax

    mov rax, 4
    push rax

    pop rdi
    call func_fatorial
    push rax

    pop rsi
    pop rdi
    mov rax, rsp
    sub rsp, 16
    and rsp, -16
    push 0
    push rax
    call escrevaf
    pop rsp
    lea rax, [dat_4]
    push rax

    mov rax, 5
    push rax

    pop rdi
    call func_fatorial
    push rax

    pop rsi
    pop rdi
    mov rax, rsp
    sub rsp, 16
    and rsp, -16
    push 0
    push rax
    call escrevaf
    pop rsp
    mov rax, 0
    mov rcx, rax
    xor rax, rax
    mov rax, rcx
pypt_main_ret:
    mov rsp, rbp
    pop rbp
    ret

; External funcs
extrn escrevaf

; Data section
section '.data'
    dat_0 db "Fatorial de 1 é %d",10,0
    dat_1 db "Fatorial de 2 é %d",10,0
    dat_2 db "Fatorial de 3 é %d",10,0
    dat_3 db "Fatorial de 4 é %d",10,0
    dat_4 db "Fatorial de 5 é %d",10,0

