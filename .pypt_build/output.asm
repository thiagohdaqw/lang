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
extrn CloseWindow

; Data section
section '.data'
    dat_0 db "Raylib por PYPT",0

