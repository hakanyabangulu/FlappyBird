; Copyright (C) 2014  Arjun Sreedharan
; License: GPL version 2 or higher http://www.gnu.org/licenses/gpl.html

bits 32
section .text
    ; Multiboot spesifikasyonu i�in ba�l�k
    align 4
    dd 0x1BADB002            ; Magic number
    dd 0x00                  ; Flags (hi�bir �zellik belirtilmemi�)
    dd - (0x1BADB002 + 0x00) ; Checksum: magic + flags + checksum = 0

; Global fonksiyonlar
global start
global keyboard_handler
global read_port
global write_port
global load_idt

; D�� C fonksiyonlar�
extern kmain                ; kernel.c'deki ana fonksiyon
extern keyboard_handler_main ; kernel.c'deki klavye i�leyicisi

; Porttan okuma fonksiyonu
read_port:
    mov edx, [esp + 4]      ; Port numaras�n� al
    in al, dx               ; Porttan oku (8 bit)
    ret

; Porta yazma fonksiyonu
write_port:
    mov edx, [esp + 4]      ; Port numaras�n� al
    mov al, [esp + 8]       ; Yaz�lacak veriyi al (ikinci arg�man)
    out dx, al              ; Porta yaz
    ret

; IDT'yi y�kleme fonksiyonu
load_idt:
    mov edx, [esp + 4]      ; IDT pointer'�n� al
    lidt [edx]              ; IDT'yi y�kle
    sti                     ; Kesmeleri etkinle�tir
    ret

; Klavye kesme i�leyicisi
keyboard_handler:
    pusha                   ; T�m register'lar� kaydet
    mov ax, 0               ; Segment register'lar�n� s�f�rla
    mov ds, ax
    mov es, ax
    call keyboard_handler_main ; C'deki i�leyiciyi �a��r
    popa                    ; Register'lar� geri y�kle
    iret                    ; Kesmeden d�n

; Kernel ba�latma noktas�
start:
    cli                     ; Kesmeleri kapat
    mov esp, stack_space    ; Y���n i�aret�isini ayarla
    call kmain              ; C'deki ana fonksiyonu �a��r
    hlt                     ; CPU'yu durdur

section .bss
    resb 8192              ; Y���n i�in 8KB alan ay�r
stack_space:               ; Y���n�n ba�lang�� adresi

; Y���n�n �al��t�r�labilir olmad���n� belirtmek i�in GNU-stack notu
section .note.GNU-stack noalloc noexec nowrite progbits