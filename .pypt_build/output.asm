format ELF64
public pypt_main
section '.text' executable align 16


; Public Functions
; End Public Functions

func_inicializa:
    push rbp
    mov rbp, rsp

    mov rax, rdi
    push rax
    mov rax, rsi
    push rax
    mov rax, 0
    push rax
    .inicializa_while_1_cond:
        mov rax, [rbp-24]
        push rax
        mov rcx, [rbp-16]
        pop rdx
        xor rax, rax
        cmp rdx, rcx
        setl ah
        cmp rax, 0
        je .inicializa_while_1_end
    .inicializa_while_1_body:
        mov rax, [rbp-8]
        push rax
        mov rdx, [rbp-24]
        imul rdx, 8
        pop rax
        add rax, rdx
        push rax
        mov rdx, 0
        pop rax
        mov [rax], rdx
        mov rax, [rbp-24]
        push rax
        mov rcx, 1
        pop rdx
        add rdx, rcx
        mov [rbp-24], rdx
    jmp .inicializa_while_1_cond
    .inicializa_while_1_end:
    mov rax, [rbp-16]
    push rax
    mov rcx, 1
    pop rdx
    shr rdx, cl
    mov rax, rdx
    push rax
    mov rax, [rbp-8]
    push rax
    mov rdx, [rbp-32]
    imul rdx, 8
    pop rax
    add rax, rdx
    push rax
    mov rdx, 1
    pop rax
    mov [rax], rdx
    mov rcx, rax
    xor rax, rax
    mov rax, rcx
func_inicializa_ret:
    mov rsp, rbp
    pop rbp
    ret
    ; END func_inicializa

func_escreva_lista:
    push rbp
    mov rbp, rsp

    mov rax, rdi
    push rax
    mov rax, rsi
    push rax
    mov rax, 0
    push rax
    .escreva_lista_while_1_cond:
        mov rax, [rbp-24]
        push rax
        mov rcx, [rbp-16]
        pop rdx
        xor rax, rax
        cmp rdx, rcx
        setl ah
        cmp rax, 0
        je .escreva_lista_while_1_end
    .escreva_lista_while_1_body:
        mov rax, [rbp-8]
        push rax
        mov rdx, [rbp-24]
        imul rdx, 8
        pop rax
        add rax, rdx
        xor rcx, rcx
        mov rcx, [rax]
        mov rax, rcx
        cmp rax, 0
        je .if_1_else
            lea rax, [dat_0]
            push rax

            pop rdi
            mov rax, rsp
            sub rsp, 16
            and rsp, -16
            push 0
            push rax
            call escreva
            pop rsp
            jmp .if_1_end
        .if_1_else:
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
        .if_1_end:
        mov rax, [rbp-24]
        push rax
        mov rcx, 1
        pop rdx
        add rdx, rcx
        mov [rbp-24], rdx
    jmp .escreva_lista_while_1_cond
    .escreva_lista_while_1_end:
    lea rax, [dat_2]
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
func_escreva_lista_ret:
    mov rsp, rbp
    pop rbp
    ret
    ; END func_escreva_lista

func_rule110:
    push rbp
    mov rbp, rsp

    mov rax, rdi
    push rax
    mov rax, rsi
    push rax
    mov rax, rdx
    push rax
    mov rax, [rbp-8]
    push rax
    mov rcx, 1
    pop rdx
    xor rax, rax
    cmp rdx, rcx
    sete ah
    push rax
    mov rax, [rbp-16]
    push rax
    mov rcx, 1
    pop rdx
    xor rax, rax
    cmp rdx, rcx
    sete ah
    mov rcx, rax
    pop rdx
    and rdx, rcx
    mov rax, rdx
    push rax
    mov rax, [rbp-24]
    push rax
    mov rcx, 1
    pop rdx
    xor rax, rax
    cmp rdx, rcx
    sete ah
    mov rcx, rax
    pop rdx
    and rdx, rcx
    mov rax, rdx
    cmp rax, 0
    je .if_1_else
            mov rax, 0
            mov rcx, rax
    xor rax, rax
    mov rax, rcx
            jmp func_rule110_ret
        jmp .if_1_end
    .if_1_else:
    .if_1_end:
    mov rax, [rbp-8]
    push rax
    mov rcx, 1
    pop rdx
    xor rax, rax
    cmp rdx, rcx
    sete ah
    push rax
    mov rax, [rbp-16]
    push rax
    mov rcx, 1
    pop rdx
    xor rax, rax
    cmp rdx, rcx
    sete ah
    mov rcx, rax
    pop rdx
    and rdx, rcx
    mov rax, rdx
    push rax
    mov rax, [rbp-24]
    push rax
    mov rcx, 0
    pop rdx
    xor rax, rax
    cmp rdx, rcx
    sete ah
    mov rcx, rax
    pop rdx
    and rdx, rcx
    mov rax, rdx
    cmp rax, 0
    je .if_2_else
            mov rax, 1
            mov rcx, rax
    xor rax, rax
    mov rax, rcx
            jmp func_rule110_ret
        jmp .if_2_end
    .if_2_else:
    .if_2_end:
    mov rax, [rbp-8]
    push rax
    mov rcx, 1
    pop rdx
    xor rax, rax
    cmp rdx, rcx
    sete ah
    push rax
    mov rax, [rbp-16]
    push rax
    mov rcx, 0
    pop rdx
    xor rax, rax
    cmp rdx, rcx
    sete ah
    mov rcx, rax
    pop rdx
    and rdx, rcx
    mov rax, rdx
    push rax
    mov rax, [rbp-24]
    push rax
    mov rcx, 1
    pop rdx
    xor rax, rax
    cmp rdx, rcx
    sete ah
    mov rcx, rax
    pop rdx
    and rdx, rcx
    mov rax, rdx
    cmp rax, 0
    je .if_3_else
            mov rax, 1
            mov rcx, rax
    xor rax, rax
    mov rax, rcx
            jmp func_rule110_ret
        jmp .if_3_end
    .if_3_else:
    .if_3_end:
    mov rax, [rbp-8]
    push rax
    mov rcx, 1
    pop rdx
    xor rax, rax
    cmp rdx, rcx
    sete ah
    push rax
    mov rax, [rbp-16]
    push rax
    mov rcx, 0
    pop rdx
    xor rax, rax
    cmp rdx, rcx
    sete ah
    mov rcx, rax
    pop rdx
    and rdx, rcx
    mov rax, rdx
    push rax
    mov rax, [rbp-24]
    push rax
    mov rcx, 0
    pop rdx
    xor rax, rax
    cmp rdx, rcx
    sete ah
    mov rcx, rax
    pop rdx
    and rdx, rcx
    mov rax, rdx
    cmp rax, 0
    je .if_4_else
            mov rax, 0
            mov rcx, rax
    xor rax, rax
    mov rax, rcx
            jmp func_rule110_ret
        jmp .if_4_end
    .if_4_else:
    .if_4_end:
    mov rax, [rbp-8]
    push rax
    mov rcx, 0
    pop rdx
    xor rax, rax
    cmp rdx, rcx
    sete ah
    push rax
    mov rax, [rbp-16]
    push rax
    mov rcx, 1
    pop rdx
    xor rax, rax
    cmp rdx, rcx
    sete ah
    mov rcx, rax
    pop rdx
    and rdx, rcx
    mov rax, rdx
    push rax
    mov rax, [rbp-24]
    push rax
    mov rcx, 1
    pop rdx
    xor rax, rax
    cmp rdx, rcx
    sete ah
    mov rcx, rax
    pop rdx
    and rdx, rcx
    mov rax, rdx
    cmp rax, 0
    je .if_5_else
            mov rax, 1
            mov rcx, rax
    xor rax, rax
    mov rax, rcx
            jmp func_rule110_ret
        jmp .if_5_end
    .if_5_else:
    .if_5_end:
    mov rax, [rbp-8]
    push rax
    mov rcx, 0
    pop rdx
    xor rax, rax
    cmp rdx, rcx
    sete ah
    push rax
    mov rax, [rbp-16]
    push rax
    mov rcx, 1
    pop rdx
    xor rax, rax
    cmp rdx, rcx
    sete ah
    mov rcx, rax
    pop rdx
    and rdx, rcx
    mov rax, rdx
    push rax
    mov rax, [rbp-24]
    push rax
    mov rcx, 0
    pop rdx
    xor rax, rax
    cmp rdx, rcx
    sete ah
    mov rcx, rax
    pop rdx
    and rdx, rcx
    mov rax, rdx
    cmp rax, 0
    je .if_6_else
            mov rax, 1
            mov rcx, rax
    xor rax, rax
    mov rax, rcx
            jmp func_rule110_ret
        jmp .if_6_end
    .if_6_else:
    .if_6_end:
    mov rax, [rbp-8]
    push rax
    mov rcx, 0
    pop rdx
    xor rax, rax
    cmp rdx, rcx
    sete ah
    push rax
    mov rax, [rbp-16]
    push rax
    mov rcx, 0
    pop rdx
    xor rax, rax
    cmp rdx, rcx
    sete ah
    mov rcx, rax
    pop rdx
    and rdx, rcx
    mov rax, rdx
    push rax
    mov rax, [rbp-24]
    push rax
    mov rcx, 1
    pop rdx
    xor rax, rax
    cmp rdx, rcx
    sete ah
    mov rcx, rax
    pop rdx
    and rdx, rcx
    mov rax, rdx
    cmp rax, 0
    je .if_7_else
            mov rax, 1
            mov rcx, rax
    xor rax, rax
    mov rax, rcx
            jmp func_rule110_ret
        jmp .if_7_end
    .if_7_else:
    .if_7_end:
    mov rax, [rbp-8]
    push rax
    mov rcx, 0
    pop rdx
    xor rax, rax
    cmp rdx, rcx
    sete ah
    push rax
    mov rax, [rbp-16]
    push rax
    mov rcx, 0
    pop rdx
    xor rax, rax
    cmp rdx, rcx
    sete ah
    mov rcx, rax
    pop rdx
    and rdx, rcx
    mov rax, rdx
    push rax
    mov rax, [rbp-24]
    push rax
    mov rcx, 0
    pop rdx
    xor rax, rax
    cmp rdx, rcx
    sete ah
    mov rcx, rax
    pop rdx
    and rdx, rcx
    mov rax, rdx
    cmp rax, 0
    je .if_8_else
            mov rax, 0
            mov rcx, rax
    xor rax, rax
    mov rax, rcx
            jmp func_rule110_ret
        jmp .if_8_end
    .if_8_else:
    .if_8_end:
    mov rcx, rax
    xor rax, rax
    mov rax, rcx
func_rule110_ret:
    mov rsp, rbp
    pop rbp
    ret
    ; END func_rule110

func_computa_proxima_geracao:
    push rbp
    mov rbp, rsp

    mov rax, rdi
    push rax
    mov rax, rsi
    push rax
    mov rax, rdx
    push rax
    mov rax, 0
    push rax
    mov rax, 0
    push rax
    mov rax, 0
    push rax
    mov rax, 0
    push rax
    .computa_proxima_geracao_while_1_cond:
        mov rax, [rbp-56]
        push rax
        mov rcx, [rbp-24]
        pop rdx
        xor rax, rax
        cmp rdx, rcx
        setl ah
        cmp rax, 0
        je .computa_proxima_geracao_while_1_end
    .computa_proxima_geracao_while_1_body:
        mov rax, [rbp-56]
        push rax
        mov rcx, 1
        neg rcx
        pop rdx
        add rdx, rcx
        mov [rbp-32], rdx
        mov rax, [rbp-32]
        push rax
        mov rcx, 0
        pop rdx
        xor rax, rax
        cmp rdx, rcx
        setl ah
        cmp rax, 0
        je .if_1_else
            mov rax, [rbp-24]
            push rax
            mov rcx, 1
            neg rcx
            pop rdx
            add rdx, rcx
            mov [rbp-32], rdx
            jmp .if_1_end
        .if_1_else:
        .if_1_end:
        mov rax, [rbp-56]
        mov [rbp-40], rax
        mov rax, [rbp-56]
        push rax
        mov rcx, 1
        pop rdx
        add rdx, rcx
        mov [rbp-48], rdx
        mov rax, [rbp-48]
        push rax
        mov rcx, [rbp-24]
        pop rdx
        xor rax, rax
        cmp rdx, rcx
        setge ah
        cmp rax, 0
        je .if_2_else
            mov rax, 0
            mov [rbp-48], rax
            jmp .if_2_end
        .if_2_else:
        .if_2_end:
        mov rax, [rbp-16]
        push rax
        mov rdx, [rbp-56]
        imul rdx, 8
        pop rax
        add rax, rdx
        push rax
        mov rax, [rbp-8]
        push rax
        mov rdx, [rbp-32]
        imul rdx, 8
        pop rax
        add rax, rdx
        xor rcx, rcx
        mov rcx, [rax]
        mov rax, rcx
        push rax

        mov rax, [rbp-8]
        push rax
        mov rdx, [rbp-40]
        imul rdx, 8
        pop rax
        add rax, rdx
        xor rcx, rcx
        mov rcx, [rax]
        mov rax, rcx
        push rax

        mov rax, [rbp-8]
        push rax
        mov rdx, [rbp-48]
        imul rdx, 8
        pop rax
        add rax, rdx
        xor rcx, rcx
        mov rcx, [rax]
        mov rax, rcx
        push rax

        pop rdx
        pop rsi
        pop rdi
        call func_rule110
        mov rdx, rax
        pop rax
        mov [rax], rdx
        mov rax, [rbp-56]
        push rax
        mov rcx, 1
        pop rdx
        add rdx, rcx
        mov [rbp-56], rdx
    jmp .computa_proxima_geracao_while_1_cond
    .computa_proxima_geracao_while_1_end:
    mov rcx, rax
    xor rax, rax
    mov rax, rcx
func_computa_proxima_geracao_ret:
    mov rsp, rbp
    pop rbp
    ret
    ; END func_computa_proxima_geracao

pypt_main:
    push rbp
    mov rbp, rsp

    mov rax, 100
    push rax
    mov rax, 80
    push rax
    sub rsp, 648
    mov rax, rsp
    push rax
    sub rsp, 648
    mov rax, rsp
    push rax
    mov rax, [rbp-672]
    push rax

    mov rax, [rbp-16]
    push rax

    pop rsi
    pop rdi
    call func_inicializa
    mov rax, 0
    push rax
    mov rax, 0
    push rax
    .pypt_main_while_1_cond:
        mov rax, [rbp-1336]
        push rax
        mov rcx, [rbp-8]
        pop rdx
        xor rax, rax
        cmp rdx, rcx
        setl ah
        cmp rax, 0
        je .pypt_main_while_1_end
    .pypt_main_while_1_body:
        mov rax, [rbp-672]
        push rax

        mov rax, [rbp-16]
        push rax

        pop rsi
        pop rdi
        call func_escreva_lista
        mov rax, [rbp-672]
        push rax

        mov rax, [rbp-1328]
        push rax

        mov rax, [rbp-16]
        push rax

        pop rdx
        pop rsi
        pop rdi
        call func_computa_proxima_geracao
        mov rax, [rbp-672]
        mov [rbp-1344], rax
        mov rax, [rbp-1328]
        mov [rbp-672], rax
        mov rax, [rbp-1344]
        mov [rbp-1328], rax
        mov rax, [rbp-1336]
        push rax
        mov rcx, 1
        pop rdx
        add rdx, rcx
        mov [rbp-1336], rdx
    jmp .pypt_main_while_1_cond
    .pypt_main_while_1_end:
    mov rax, 0
pypt_main_ret:
    mov rsp, rbp
    pop rbp
    ret

; External funcs
extrn escreva

; Data section
section '.data'
    dat_0 db "#",0
    dat_1 db " ",0
    dat_2 db "",10,0

