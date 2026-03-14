format ELF64
public pypt_main
section '.text' executable align 16


; Public Functions
; End Public Functions

func_fibonacci:
    push rbp
    mov rbp, rsp

    mov rax, rdi
    push rax
    mov rax, 0
    push rax
    mov rax, 1
    push rax
    mov rax, 0
    push rax
    lea rax, [dat_0]
    push rax

    mov rax, [rbp-24]
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
    push rax
    .fibonacci_while_1_cond:
        mov rax, [rbp-40]
        push rax
        mov rcx, [rbp-8]
        pop rdx
        xor rax, rax
        cmp rdx, rcx
        setl ah
        cmp rax, 0
        je .fibonacci_while_1_end
    .fibonacci_while_1_body:
        mov rax, [rbp-16]
        push rax
        mov rcx, [rbp-24]
        pop rdx
        add rdx, rcx
        mov [rbp-32], rdx
        mov rax, [rbp-24]
        mov [rbp-16], rax
        mov rax, [rbp-32]
        mov [rbp-24], rax
        mov rax, [rbp-40]
        push rax
        mov rcx, 1
        pop rdx
        add rdx, rcx
        mov [rbp-40], rdx
        lea rax, [dat_0]
        push rax

        mov rax, [rbp-24]
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
    jmp .fibonacci_while_1_cond
    .fibonacci_while_1_end:
    lea rax, [dat_1]
    push rax

    pop rdi
    mov rax, rsp
    sub rsp, 16
    and rsp, -16
    push 0
    push rax
    call escreva
    pop rsp
    mov rcx, rax
    xor rax, rax
    mov rax, rcx
func_fibonacci_ret:
    mov rsp, rbp
    pop rbp
    ret
    ; END func_fibonacci

pypt_main:
    push rbp
    mov rbp, rsp

    lea rax, [dat_2]
    push rax

    pop rdi
    mov rax, rsp
    sub rsp, 16
    and rsp, -16
    push 0
    push rax
    call escreval
    pop rsp
    mov rax, 20
    push rax

    pop rdi
    call func_fibonacci
    mov rax, 0
pypt_main_ret:
    mov rsp, rbp
    pop rbp
    ret

; External funcs
extrn escrevaf
extrn escreva
extrn escreval

; Data section
section '.data'
    dat_0 db "%d ",0
    dat_1 db "",10,0
    dat_2 db "Sequencia de fibonacci de 10 elementos",0

