format ELF64
public pypt_main
section '.text' executable align 16


; Public Functions
; End Public Functions

pypt_main:
    push rbp
    mov rbp, rsp

    mov rax, 800
    push rax
    mov rax, 600
    push rax
    mov rax, [rbp-8]
    push rax

    mov rax, [rbp-16]
    push rax

    lea rax, [dat_0]
    push rax

    pop rdx
    pop rsi
    pop rdi
    mov rax, rsp
    sub rsp, 16
    and rsp, -16
    push 0
    push rax
    call InitWindow
    pop rsp
    mov rax, 60
    push rax

    pop rdi
    mov rax, rsp
    sub rsp, 16
    and rsp, -16
    push 0
    push rax
    call SetTargetFPS
    pop rsp
    sub rsp, 8
    mov rax, rsp
    push rax
    mov rax, [rbp-32]
    push rax
    mov rdx, 0
    imul rdx, 1
    pop rax
    add rax, rdx
    push rax
    mov rdx, 130
    pop rax
    mov [rax], dl
    mov rax, [rbp-32]
    push rax
    mov rdx, 1
    imul rdx, 1
    pop rax
    add rax, rdx
    push rax
    mov rdx, 130
    pop rax
    mov [rax], dl
    mov rax, [rbp-32]
    push rax
    mov rdx, 2
    imul rdx, 1
    pop rax
    add rax, rdx
    push rax
    mov rdx, 130
    pop rax
    mov [rax], dl
    mov rax, [rbp-32]
    push rax
    mov rdx, 3
    imul rdx, 1
    pop rax
    add rax, rdx
    push rax
    mov rdx, 255
    pop rax
    mov [rax], dl
    sub rsp, 8
    mov rax, rsp
    push rax
    mov rax, [rbp-48]
    push rax
    mov rdx, 0
    imul rdx, 1
    pop rax
    add rax, rdx
    push rax
    mov rdx, 255
    pop rax
    mov [rax], dl
    mov rax, [rbp-48]
    push rax
    mov rdx, 1
    imul rdx, 1
    pop rax
    add rax, rdx
    push rax
    mov rdx, 0
    pop rax
    mov [rax], dl
    mov rax, [rbp-48]
    push rax
    mov rdx, 2
    imul rdx, 1
    pop rax
    add rax, rdx
    push rax
    mov rdx, 0
    pop rax
    mov [rax], dl
    mov rax, [rbp-48]
    push rax
    mov rdx, 3
    imul rdx, 1
    pop rax
    add rax, rdx
    push rax
    mov rdx, 255
    pop rax
    mov [rax], dl
    mov rax, 0
    push rax
    mov rax, 0
    push rax
    mov rax, 0
    push rax
    mov rax, 0
    push rax
    mov rax, 50
    push rax
    mov rax, 2
    push rax
    mov rax, 2
    push rax
    .pypt_main_while_1_cond:
        mov rax, rsp
        sub rsp, 16
        and rsp, -16
        push 0
        push rax
        call WindowShouldClose
        pop rsp
        push rax
        mov rcx, 0
        pop rdx
        xor rax, rax
        cmp rdx, rcx
        sete al
        cmp rax, 0
        je .pypt_main_while_1_end
    .pypt_main_while_1_body:
        mov rax, [rbp-56]
        push rax
        mov rcx, [rbp-96]
        pop rdx
        add rdx, rcx
        mov [rbp-64], rdx
        mov rax, [rbp-72]
        push rax
        mov rcx, [rbp-104]
        pop rdx
        add rdx, rcx
        mov [rbp-80], rdx
        mov rax, [rbp-64]
        push rax
        mov rcx, 0
        pop rdx
        xor rax, rax
        cmp rdx, rcx
        setle al
        push rax
        mov rax, [rbp-64]
        push rax
        mov rcx, [rbp-88]
        pop rdx
        add rdx, rcx
        mov rax, rdx
        push rax
        mov rcx, [rbp-8]
        pop rdx
        xor rax, rax
        cmp rdx, rcx
        setge al
        mov rcx, rax
        pop rdx
        or rdx, rcx
        mov rax, rdx
        cmp rax, 0
        je .if_1_else
            mov rax, [rbp-96]
            push rax
            mov rcx, 1
            neg rcx
            pop rdx
            imul rdx, rcx
            mov [rbp-96], rdx
            jmp .if_1_end
        .if_1_else:
        .if_1_end:
        mov rax, [rbp-80]
        push rax
        mov rcx, 0
        pop rdx
        xor rax, rax
        cmp rdx, rcx
        setle al
        push rax
        mov rax, [rbp-80]
        push rax
        mov rcx, [rbp-88]
        pop rdx
        add rdx, rcx
        mov rax, rdx
        push rax
        mov rcx, [rbp-16]
        pop rdx
        xor rax, rax
        cmp rdx, rcx
        setge al
        mov rcx, rax
        pop rdx
        or rdx, rcx
        mov rax, rdx
        cmp rax, 0
        je .if_2_else
            mov rax, [rbp-104]
            push rax
            mov rcx, 1
            neg rcx
            pop rdx
            imul rdx, rcx
            mov [rbp-104], rdx
            jmp .if_2_end
        .if_2_else:
        .if_2_end:
        mov rax, [rbp-56]
        push rax
        mov rcx, [rbp-96]
        pop rdx
        add rdx, rcx
        mov [rbp-56], rdx
        mov rax, [rbp-72]
        push rax
        mov rcx, [rbp-104]
        pop rdx
        add rdx, rcx
        mov [rbp-72], rdx
        mov rax, rsp
        sub rsp, 16
        and rsp, -16
        push 0
        push rax
        call BeginDrawing
        pop rsp
        mov rax, [rbp-32]
        push rax
        mov rdx, 0
        imul rdx, 8
        pop rax
        add rax, rdx
        xor rcx, rcx
        mov rcx, [rax]
        mov rax, rcx
        push rax

        pop rdi
        mov rax, rsp
        sub rsp, 16
        and rsp, -16
        push 0
        push rax
        call ClearBackground
        pop rsp
        mov rax, [rbp-56]
        push rax

        mov rax, [rbp-72]
        push rax

        mov rax, [rbp-88]
        push rax

        mov rax, [rbp-88]
        push rax

        mov rax, [rbp-48]
        push rax
        mov rdx, 0
        imul rdx, 8
        pop rax
        add rax, rdx
        xor rcx, rcx
        mov rcx, [rax]
        mov rax, rcx
        push rax

        pop r8
        pop rcx
        pop rdx
        pop rsi
        pop rdi
        mov rax, rsp
        sub rsp, 16
        and rsp, -16
        push 0
        push rax
        call DrawRectangle
        pop rsp
        mov rax, rsp
        sub rsp, 16
        and rsp, -16
        push 0
        push rax
        call EndDrawing
        pop rsp
    jmp .pypt_main_while_1_cond
    .pypt_main_while_1_end:
    mov rax, rsp
    sub rsp, 16
    and rsp, -16
    push 0
    push rax
    call CloseWindow
    pop rsp
    mov rax, 0
pypt_main_ret:
    mov rsp, rbp
    pop rbp
    ret

; External funcs
extrn InitWindow
extrn SetTargetFPS
extrn WindowShouldClose
extrn BeginDrawing
extrn ClearBackground
extrn DrawRectangle
extrn EndDrawing
extrn CloseWindow

; Data section
section '.data'
    dat_0 db "Raylib por PYPT",0

