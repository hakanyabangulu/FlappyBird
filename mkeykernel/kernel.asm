; Copyright (C) 2014  Arjun Sreedharan
; License: GPL version 2 or higher http://www.gnu.org/licenses/gpl.html

bits 32
section .text
    ; Multiboot spesifikasyonu için baþlýk
    align 4
    dd 0x1BADB002            ; Magic number
    dd 0x00                  ; Flags (hiçbir özellik belirtilmemiþ)
    dd - (0x1BADB002 + 0x00) ; Checksum: magic + flags + checksum = 0

; Global fonksiyonlar
global start
global keyboard_handler
global read_port
global write_port
global load_idt

; Dýþ C fonksiyonlarý
extern kmain                ; kernel.c'deki ana fonksiyon
extern keyboard_handler_main ; kernel.c'deki klavye iþleyicisi

; Porttan okuma fonksiyonu
read_port:
    mov edx, [esp + 4]      ; Port numarasýný al
    in al, dx               ; Porttan oku (8 bit)
    ret

; Porta yazma fonksiyonu
write_port:
    mov edx, [esp + 4]      ; Port numarasýný al
    mov al, [esp + 8]       ; Yazýlacak veriyi al (ikinci argüman)
    out dx, al              ; Porta yaz
    ret

; IDT'yi yükleme fonksiyonu
load_idt:
    mov edx, [esp + 4]      ; IDT pointer'ýný al
    lidt [edx]              ; IDT'yi yükle
    sti                     ; Kesmeleri etkinleþtir
    ret

; Klavye kesme iþleyicisi
keyboard_handler:
    pusha                   ; Tüm register'larý kaydet
    mov ax, 0               ; Segment register'larýný sýfýrla
    mov ds, ax
    mov es, ax
    call keyboard_handler_main ; C'deki iþleyiciyi çaðýr
    popa                    ; Register'larý geri yükle
    iret                    ; Kesmeden dön

; Kernel baþlatma noktasý
start:
    cli                     ; Kesmeleri kapat
    mov esp, stack_space    ; Yýðýn iþaretçisini ayarla
    call kmain              ; C'deki ana fonksiyonu çaðýr
    hlt                     ; CPU'yu durdur

section .bss
    resb 8192              ; Yýðýn için 8KB alan ayýr
stack_space:               ; Yýðýnýn baþlangýç adresi

; Yýðýnýn çalýþtýrýlabilir olmadýðýný belirtmek için GNU-stack notu
section .note.GNU-stack noalloc noexec nowrite progbits