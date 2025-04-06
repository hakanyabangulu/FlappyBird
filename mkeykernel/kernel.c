/*
 * Flappy Bird Kernel - mkeykernel tabanlı (Q ile Çıkış Düzeltildi)
 * Tarih: Nisan 1, 2025
 */
#include "keyboard_map.h"

/* VGA text mode sabitleri */
#define LINES 25
#define COLUMNS 80
#define SCREEN_SIZE (LINES * COLUMNS * 2)
#define VIDEO_MEMORY 0xb8000
#define SPACEBAR 32
#define ESCAPE 27
#define Q_KEY 16 /* q tuşunun keycode’u */

/* Klavye portları */
#define KB_DATA_PORT 0x60
#define KB_STATUS_PORT 0x64

/* IDT ayarları */
#define IDT_SIZE 256
#define INT_GATE 0x8e
#define KERNEL_CS 0x08

/* Oyun sabitleri */
#define BIRD_X 10
#define GAP_SIZE_INITIAL 6
#define GRAVITY 1
#define FLAP_VELOCITY -2
#define PIPE_SPEED 2
#define FPS 45
#define UPDATE_INTERVAL 15
#define MAX_PIPES 3
#define PIPE_SPACING 30

/* Rastgele sayı için prototip */
int rand(void);

/* kernel.asm içinde tanımlı fonksiyonlar */
extern void keyboard_handler(void);
extern unsigned char read_port(unsigned short port);
extern void write_port(unsigned short port, unsigned char data);
extern void load_idt(unsigned long *idt_ptr);

/* Ekran belleği */
char *screen = (char*)VIDEO_MEMORY;

/* Oyun durumu */
int bird_y = LINES / 2;
int bird_vel = 0;
int pipe_x[MAX_PIPES] = {COLUMNS - 1, -PIPE_SPACING, -2 * PIPE_SPACING};
int pipe_gap[MAX_PIPES] = {10, 10, 10};
int gap_size = GAP_SIZE_INITIAL;
int score = 0;
int high_score = 0;
int running = 0;
int flap = 0;
int frame = 0;
int game_over = 0;
int update_counter = 0;
int space_pressed = 0;
int exit_game = 0;

/* IDT yapısı */
struct IDT_entry {
    unsigned short offset_low;
    unsigned short selector;
    unsigned char zero;
    unsigned char type_attr;
    unsigned short offset_high;
} IDT[IDT_SIZE];

/* Ekranı temizle */
void clear_screen() {
    for (int i = 0; i < SCREEN_SIZE; i += 2) {
        screen[i] = ' ';
        screen[i + 1] = 0x07;
    }
}

/* Sayıyı string'e çevir */
void draw_number(int num, int x, int y, char color) {
    int pos = x;
    if (num == 0) {
        screen[(y * COLUMNS + pos) * 2] = '0';
        screen[(y * COLUMNS + pos) * 2 + 1] = color;
        return;
    }
    char digits[10];
    int i = 0;
    int temp = num < 0 ? -num : num;
    if (num < 0) {
        screen[(y * COLUMNS + pos) * 2] = '-';
        screen[(y * COLUMNS + pos) * 2 + 1] = color;
        pos++;
    }
    while (temp > 0) {
        digits[i++] = (temp % 10) + '0';
        temp /= 10;
    }
    for (int j = i - 1; j >= 0; j--) {
        screen[(y * COLUMNS + pos++) * 2] = digits[j];
        screen[(y * COLUMNS + pos - 1) * 2 + 1] = color;
    }
}

/* Oyun durumunu sıfırla */
void reset_game() {
    bird_y = LINES / 2;
    bird_vel = 0;
    for (int i = 0; i < MAX_PIPES; i++) {
        pipe_x[i] = COLUMNS - 1 - i * PIPE_SPACING;
        pipe_gap[i] = 5 + (rand() % (LINES - gap_size - 10));
    }
    gap_size = GAP_SIZE_INITIAL;
    score = 0;
    flap = 0;
    frame = 0;
    update_counter = 0;
    running = 1;
    game_over = 0;
    space_pressed = 0;
    write_port(0x21, 0xFD);
}

/* Oyunu çiz */
void draw() {
    clear_screen();

    if (exit_game) {
        const char *msg = "Game Exited - Bye!";
        for (int i = 0; msg[i]; i++) {
            screen[((LINES / 2) * COLUMNS + (COLUMNS / 2 - 9) + i) * 2] = msg[i];
            screen[((LINES / 2) * COLUMNS + (COLUMNS / 2 - 9) + i) * 2 + 1] = 0x0F;
        }
        return;
    }

    if (!running) {
        if (!game_over) {
            const char *msg = "Flappy Bird - X to Start!";
            for (int i = 0; msg[i]; i++) {
                screen[((LINES / 2) * COLUMNS + (COLUMNS / 2 - 14) + i) * 2] = msg[i];
                screen[((LINES / 2) * COLUMNS + (COLUMNS / 2 - 14) + i) * 2 + 1] = 0x0E;
            }
        } else {
            const char *msg = "Game Over! X to Restart, Q to Quit";
            for (int i = 0; msg[i]; i++) {
                screen[((LINES / 2) * COLUMNS + (COLUMNS / 2 - 16) + i) * 2] = msg[i];
                screen[((LINES / 2) * COLUMNS + (COLUMNS / 2 - 16) + i) * 2 + 1] = 0x0C;
            }
            draw_number(score, COLUMNS / 2 - 2, LINES / 2 + 1, 0x07);
            const char *hs_msg = "High Score: ";
            for (int i = 0; hs_msg[i]; i++) {
                screen[((LINES / 2 + 2) * COLUMNS + (COLUMNS / 2 - 6) + i) * 2] = hs_msg[i];
                screen[((LINES / 2 + 2) * COLUMNS + (COLUMNS / 2 - 6) + i) * 2 + 1] = 0x07;
            }
            draw_number(high_score, COLUMNS / 2 + 5, LINES / 2 + 2, 0x07);
        }
        return;
    }

    /* Kuşu çiz */
    if (bird_y >= 0 && bird_y < LINES) {
        screen[(bird_y * COLUMNS + BIRD_X) * 2] = (frame % 2) ? '>' : '-';
        screen[(bird_y * COLUMNS + BIRD_X) * 2 + 1] = 0x0E;
    }

    /* Boruları çiz */
    for (int i = 0; i < MAX_PIPES; i++) {
        if (pipe_x[i] >= 0 && pipe_x[i] < COLUMNS) {
            for (int y = 0; y < LINES; y++) {
                if (y < pipe_gap[i] || y >= pipe_gap[i] + gap_size) {
                    screen[(y * COLUMNS + pipe_x[i]) * 2] = '|';
                    screen[(y * COLUMNS + pipe_x[i]) * 2 + 1] = 0x02;
                }
            }
        }
    }

    /* Skoru çiz */
    const char *msg = "Score: ";
    for (int i = 0; msg[i]; i++) {
        screen[i * 2] = msg[i];
        screen[i * 2 + 1] = 0x07;
    }
    draw_number(score, 7, 0, 0x07);
}

/* Oyunu güncelle */
void update() {
    if (!running) return;

    frame++;

    update_counter++;
    if (update_counter >= UPDATE_INTERVAL) {
        update_counter = 0;

        bird_vel += GRAVITY;
        if (flap) {
            bird_vel = FLAP_VELOCITY;
            flap = 0;
        }
        bird_y += bird_vel;

        for (int i = 0; i < MAX_PIPES; i++) {
            pipe_x[i] -= PIPE_SPEED;
            if (pipe_x[i] < -1) {
                pipe_x[i] = COLUMNS - 1;
                pipe_gap[i] = 5 + (rand() % (LINES - gap_size - 10));
                score++;
                gap_size = GAP_SIZE_INITIAL - (score / 10);
                if (gap_size < 3) gap_size = 3;
            }
        }

        bird_y = (bird_y < 0) ? 0 : (bird_y >= LINES) ? LINES - 1 : bird_y;
        for (int i = 0; i < MAX_PIPES; i++) {
            pipe_x[i] = (pipe_x[i] < -1) ? -1 : (pipe_x[i] >= COLUMNS) ? COLUMNS - 1 : pipe_x[i];
            pipe_gap[i] = (pipe_gap[i] < 0) ? 0 : (pipe_gap[i] >= LINES) ? LINES - 1 : pipe_gap[i];
            if (pipe_gap[i] + gap_size >= LINES) pipe_gap[i] = LINES - gap_size - 1;
        }

        if (bird_y <= 0 || bird_y >= LINES - 1) {
            running = 0;
            game_over = 1;
            if (score > high_score) high_score = score;
        }
        for (int i = 0; i < MAX_PIPES; i++) {
            if (pipe_x[i] >= BIRD_X - 2 && pipe_x[i] <= BIRD_X + 2) {
                if (bird_y < pipe_gap[i] || bird_y >= pipe_gap[i] + gap_size) {
                    running = 0;
                    game_over = 1;
                    if (score > high_score) high_score = score;
                }
            }
        }
    }
}

/* FPS kontrolü için kısa gecikme */
void delay() {
    for (int i = 0; i < 70000000 / FPS; i++);
    write_port(0x21, 0xFD);
}

/* Rastgele sayı üreteci */
int rand(void) {
    static unsigned int seed = 0xDEADBEEF;
    seed = (seed * 1103515245 + 12345) & 0x7fffffff;
    return seed;
}

/* IDT başlat */
void idt_init() {
    unsigned long kb_handler_addr = (unsigned long)keyboard_handler;
    IDT[0x21].offset_low = kb_handler_addr & 0xFFFF;
    IDT[0x21].selector = KERNEL_CS;
    IDT[0x21].zero = 0;
    IDT[0x21].type_attr = INT_GATE;
    IDT[0x21].offset_high = (kb_handler_addr >> 16) & 0xFFFF;

    write_port(0x20, 0x11);
    write_port(0xA0, 0x11);
    write_port(0x21, 0x20);
    write_port(0xA1, 0x28);
    write_port(0x21, 0x04);
    write_port(0xA1, 0x02);
    write_port(0x21, 0x01);
    write_port(0xA1, 0x01);
    write_port(0x21, 0xFD);
    write_port(0xA1, 0xFF);

    unsigned long idt_addr = (unsigned long)IDT;
    unsigned long idt_ptr[2] = { (sizeof(struct IDT_entry) * IDT_SIZE) + ((idt_addr & 0xFFFF) << 16), idt_addr >> 16 };
    load_idt(idt_ptr);
}

/* Klavye başlat */
void kb_init() {
    write_port(0x21, 0xFD);
}

/* Klavye işleyici */
void keyboard_handler_main() {
    unsigned char status = read_port(KB_STATUS_PORT);

    if (status & 0x01) {
        char keycode = read_port(KB_DATA_PORT);
        unsigned char key = (keycode & 0x80) ? 0 : keyboard_map[(unsigned char)keycode];

        if (keycode & 0x80) { /* Tuş bırakma */
            if ((keycode & 0x7F) == 0x39) { /* Space bırakıldı */
                space_pressed = 0;
            }
        } else { /* Tuş basma */
            if (key == 'x' && !running && !exit_game) {
                reset_game();
            } else if (key == SPACEBAR && running && !space_pressed) {
                flap = 1;
                space_pressed = 1;
            } else if (key == ESCAPE && !exit_game) {
                running = !running;
            } else if (keycode == Q_KEY && game_over && !exit_game) { /* Q ile çıkış */
                exit_game = 1;
            }
        }

        // Buffer temizleme
        while (read_port(KB_STATUS_PORT) & 0x01) {
            read_port(KB_DATA_PORT);
        }

        // EOI
        write_port(0x20, 0x20);
        write_port(0xA0, 0x20);
    }
}

/* Ana kernel fonksiyonu */
void kmain() {
    clear_screen();
    idt_init();
    kb_init();

    bird_y = LINES / 2;
    bird_vel = 0;
    for (int i = 0; i < MAX_PIPES; i++) {
        pipe_x[i] = COLUMNS - 1 - i * PIPE_SPACING;
        pipe_gap[i] = 10;
    }
    gap_size = GAP_SIZE_INITIAL;
    score = 0;
    flap = 0;
    frame = 0;
    update_counter = 0;
    running = 0;
    game_over = 0;
    space_pressed = 0;
    exit_game = 0;

    while (1) { /* Sonsuz döngü, çıkış için return kullanacağız */
        if (exit_game) {
            draw(); /* Çıkış ekranını çiz */
            return; /* Döngüden çık ve kernelı sonlandır */
        }
        if (running) {
            update();
            draw();
            delay();
        } else {
            draw();
            delay();
        }
    }
}