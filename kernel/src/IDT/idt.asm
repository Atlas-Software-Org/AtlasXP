%macro isr_err_stub 1
global isr_stub_%+%1
isr_stub_%+%1:
    pop rsi          ; error code → rsi
    push rdi
    mov rdi, %1      ; exception number → rdi
    call KiExceptionHandler
    pop rdi
    iretq
%endmacro

%macro isr_no_err_stub 1
global isr_stub_%+%1
isr_stub_%+%1:
    push rsi
    push rdi
    xor rsi, rsi     ; no error code → rsi = 0
    mov rdi, %1      ; exception number → rdi
    call KiExceptionHandler
    pop rdi
    pop rsi
    iretq
%endmacro

extern KiExceptionHandler
isr_no_err_stub 0
isr_no_err_stub 1
isr_no_err_stub 2
isr_no_err_stub 3
isr_no_err_stub 4
isr_no_err_stub 5
isr_no_err_stub 6
isr_no_err_stub 7
isr_err_stub    8
isr_no_err_stub 9
isr_err_stub    10
isr_err_stub    11
isr_err_stub    12
isr_err_stub    13
isr_err_stub    14
isr_no_err_stub 15
isr_no_err_stub 16
isr_err_stub    17
isr_no_err_stub 18
isr_no_err_stub 19
isr_no_err_stub 20
isr_no_err_stub 21
isr_no_err_stub 22
isr_no_err_stub 23
isr_no_err_stub 24
isr_no_err_stub 25
isr_no_err_stub 26
isr_no_err_stub 27
isr_no_err_stub 28
isr_no_err_stub 29
isr_err_stub    30
isr_no_err_stub 31

global isr_stub_table
isr_stub_table:
%assign i 0 
%rep    32 
    dq isr_stub_%+i ; use DQ instead if targeting 64-bit
%assign i i+1 
%endrep
