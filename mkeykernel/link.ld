ENTRY(kmain) /* Kernel'in giriş noktası kernel.c'deki kmain fonksiyonu */

SECTIONS
{
    . = 0x100000; /* Kernel 1MB adresinden başlasın */

    /* Kod bölümü (okunabilir ve çalıştırılabilir) */
    .text ALIGN(0x1000) : {
        *(.multiboot)    /* Multiboot başlığı (GRUB uyumluluğu için) */
        *(.text)         /* Tüm kod buraya */
    } :text

    /* Salt okunur veri bölümü */
    .rodata ALIGN(0x1000) : {
        *(.rodata)       /* Sabit veriler (örneğin string literalleri) */
    } :rodata

    /* Okunabilir/yazılabilir veri bölümü */
    .data ALIGN(0x1000) : {
        *(.data)         /* Başlatılmış global ve statik değişkenler */
    } :data

    /* Sıfırlanmış veri bölümü (BSS) */
    .bss ALIGN(0x1000) : {
        *(COMMON)        /* Ortak semboller */
        *(.bss)          /* Sıfırlanmış global ve statik değişkenler */
    } :bss
}

PHDRS
{
    text   PT_LOAD FLAGS(5); /* R-X: Okunabilir ve çalıştırılabilir (5 = 4 + 1) */
    rodata PT_LOAD FLAGS(4); /* R--: Yalnızca okunabilir (4) */
    data   PT_LOAD FLAGS(6); /* RW-: Okunabilir ve yazılabilir (6 = 4 + 2) */
    bss    PT_LOAD FLAGS(6); /* RW-: Okunabilir ve yazılabilir (6 = 4 + 2) */
}