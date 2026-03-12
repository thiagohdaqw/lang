format ELF64
public pypt_main
section '.text' executable align 16


; Public Functions
; End Public Functions

func_hello:
    push rbp
    mov rbp, rsp

    mov rax, dat_0
    push rax
    pop rdi
    call escreval
func_hello_ret:
    mov rsp, rbp
    pop rbp
    ret
    ; END func_hello

pypt_main:
    push rbp
    mov rbp, rsp

    call func_hello
    mov rcx, 69
    mov rax, rcx
pypt_main_ret:
    mov rsp, rbp
    pop rbp
    ret

; External funcs
extrn escreval

; Data section
section '.data'
    dat_0 db "ola",0
