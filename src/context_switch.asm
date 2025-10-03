; Context switching assembly code
[global perform_context_switch]

; void perform_context_switch(cpu_context_t* old_context, cpu_context_t* new_context)
perform_context_switch:
    mov eax, [esp + 4]  ; old_context pointer
    mov edx, [esp + 8]  ; new_context pointer

    ; Save old context
    mov [eax + 0], eax   ; Save EAX (we'll fix this)
    mov [eax + 4], ebx
    mov [eax + 8], ecx
    mov [eax + 12], edx  ; Save EDX (we'll fix this too)
    mov [eax + 16], esi
    mov [eax + 20], edi
    mov [eax + 24], ebp
    mov [eax + 28], esp

    ; Save EIP (return address)
    mov ebx, [esp]
    mov [eax + 32], ebx

    ; Save EFLAGS
    pushf
    pop ebx
    mov [eax + 36], ebx

    ; Save CR3 (page directory)
    mov ebx, cr3
    mov [eax + 40], ebx

    ; Fix EAX and EDX in saved context
    mov ebx, [esp + 4]  ; Get old EAX value
    mov [eax + 0], ebx
    mov ebx, [esp + 8]  ; Get old EDX value
    mov [eax + 12], ebx

    ; Restore new context
    mov eax, edx  ; new_context pointer

    ; Switch page directory (CR3)
    mov ebx, [eax + 40]
    mov cr3, ebx

    ; Restore registers
    mov ebx, [eax + 4]
    mov ecx, [eax + 8]
    mov edx, [eax + 12]
    mov esi, [eax + 16]
    mov edi, [eax + 20]
    mov ebp, [eax + 24]
    mov esp, [eax + 28]

    ; Restore EFLAGS
    mov ebx, [eax + 36]
    push ebx
    popf

    ; Get new EIP
    mov ebx, [eax + 32]

    ; Restore EAX last
    push eax
    mov eax, [eax + 0]

    ; Jump to new EIP
    jmp ebx
