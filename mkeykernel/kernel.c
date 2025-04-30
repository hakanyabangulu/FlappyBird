#include "keyboard_map.h"

#define LINES 25
#define COLUMNS 80
#define SCREEN_SIZE (LINES * COLUMNS * 2) // Ekran bellek boyutu
#define VIDEO_MEMORY 0xb8000 // Bellek adresi
#define SPACEBAR 32
#define ESCAPE 27
#define Q_KEY 16 // Scan code

// Klavye portları
#define KB_DATA_PORT 0x60 // Tuş kodlarını okumak için
#define KB_STATUS_PORT 0x64 // Klavye Durum kontrolü

// IDT ayarları
#define IDT_SIZE 256// Klavye kesme boyutu
#define INT_GATE 0x8e // Tür
#define KERNEL_CS 0x08 // Code segment.

// Oyun sabitleri
#define BIRD_X 10
#define GAP_SIZE_INITIAL 6 // Borular arası boşluk
#define GRAVITY 1
#define FLAP_VELOCITY -2 // Zıplama hızı
#define PIPE_SPEED 2 //Sola kayma hızı
#define FPS 80 // Kare sayısı	
#define UPDATE_INTERVAL 15 //Kaç karede bir güncelle. Hızla alakalı bu da.
#define MAX_PIPES 2
#define PIPE_SPACING 50 // Yatay mesafe borular arası
#define PIPE_WIDTH 5 // Boru genişliği

int rand(void);

extern void keyboard_handler(void);
extern unsigned char read_port(unsigned short port);
extern void write_port(unsigned short port, unsigned char data);
extern void load_idt(unsigned long *idt_ptr);

char *screen = (char*)VIDEO_MEMORY; // Ekran belleği işaretçisi

// Oyun durumu değişkenleri
int bird_y = LINES / 2;
int bird_vel = 0;
int pipe_x[MAX_PIPES] = {COLUMNS + 20, COLUMNS + 20 - PIPE_SPACING}; // Boruların Yatay konumları.
int pipe_gap[MAX_PIPES] = {10, 10}; // Dikey başlangıç noktaları.
int gap_size = GAP_SIZE_INITIAL; // Boru boşluğu yüksekliği
int score = 0;
int high_score = 0;
int running = 0;
int flap = 0;
int frame = 0;
int game_over = 0;
int update_counter = 0;
int space_pressed = 0;
int exit_game = 0;

// IDT giriş yapısı
struct IDT_entry { // Interrupt descriptor Table 
    unsigned short offset_low; // Alt 16 bit.
    unsigned short selector; // which code segment. Kernel_cs işaret eder
    unsigned char zero; // Reserving
    unsigned char type_attr; // tür ve özellik. INT_GATE
	unsigned short offset_high; // üst 16 bit.
} IDT[IDT_SIZE];

void clear_screen() {
    for (int i = 0; i < SCREEN_SIZE; i += 2) {
        screen[i] = ' ';
        screen[i + 1] = 0x0B; /* Açık mavi arka plan */
    }
}

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

void reset_game() {
    bird_y = LINES / 2;
    bird_vel = 0;
    for (int i = 0; i < MAX_PIPES; i++) {
        pipe_x[i] = COLUMNS + 20 - i * PIPE_SPACING; /* 2 boru için */
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

void draw() {
    clear_screen();
    if (exit_game) {
        for (int x = COLUMNS / 2 - 15; x < COLUMNS / 2 + 15; x++) {
            screen[((LINES / 2 - 1) * COLUMNS + x) * 2] = '-';
            screen[((LINES / 2 + 1) * COLUMNS + x) * 2] = '-';
            screen[((LINES / 2 - 1) * COLUMNS + x) * 2 + 1] = 0x0F;
            screen[((LINES / 2 + 1) * COLUMNS + x) * 2 + 1] = 0x0F;
        }
        for (int y = LINES / 2 - 1; y <= LINES / 2 + 1; y++) {
            screen[(y * COLUMNS + COLUMNS / 2 - 15) * 2] = '|';
            screen[(y * COLUMNS + COLUMNS / 2 + 14) * 2] = '|';
            screen[(y * COLUMNS + COLUMNS / 2 - 15) * 2 + 1] = 0x0F;
            screen[(y * COLUMNS + COLUMNS / 2 + 14) * 2 + 1] = 0x0F;
        }
        const char *msg = ">>> Game Exited - Bye! <<<";
        for (int i = 0; msg[i]; i++) {
            screen[((LINES / 2) * COLUMNS + (COLUMNS / 2 - 13) + i) * 2] = msg[i];
            screen[((LINES / 2) * COLUMNS + (COLUMNS / 2 - 13) + i) * 2 + 1] = 0x0F;
        }
        /* QEMU'yu kapatmak için sistemi durdur */
        asm volatile ("hlt");
        while (1) {
            asm volatile ("hlt");
        }
        return; /* Buraya ulaşılmaz, ama derleyici için */
    }

    if (!running) {
        if (!game_over) {
            /* Başlangıç ekranı çerçeveli */
            for (int x = COLUMNS / 2 - 18; x < COLUMNS / 2 + 18; x++) {
                screen[((LINES / 2 - 1) * COLUMNS + x) * 2] = '-';
                screen[((LINES / 2 + 1) * COLUMNS + x) * 2] = '-';
                screen[((LINES / 2 - 1) * COLUMNS + x) * 2 + 1] = 0x0E;
                screen[((LINES / 2 + 1) * COLUMNS + x) * 2 + 1] = 0x0E;
            }
            for (int y = LINES / 2 - 1; y <= LINES / 2 + 1; y++) {
                screen[(y * COLUMNS + COLUMNS / 2 - 18) * 2] = '|';
                screen[(y * COLUMNS + COLUMNS / 2 + 17) * 2] = '|';
                screen[(y * COLUMNS + COLUMNS / 2 - 18) * 2 + 1] = 0x0E;
                screen[(y * COLUMNS + COLUMNS / 2 + 17) * 2 + 1] = 0x0E;
            }
            const char *msg = "<<< Flappy Bird - X to Start! >>>";
            for (int i = 0; msg[i]; i++) {
                screen[((LINES / 2) * COLUMNS + (COLUMNS / 2 - 16) + i) * 2] = msg[i];
                screen[((LINES / 2) * COLUMNS + (COLUMNS / 2 - 16) + i) * 2 + 1] = 0x0E;
            }
        } else {
            /* Game Over ekranı çerçeveli */
            for (int x = COLUMNS / 2 - 22; x < COLUMNS / 2 + 22; x++) {
                screen[((LINES / 2 - 2) * COLUMNS + x) * 2] = '-';
                screen[((LINES / 2 + 3) * COLUMNS + x) * 2] = '-';
                screen[((LINES / 2 - 2) * COLUMNS + x) * 2 + 1] = 0x0C;
                screen[((LINES / 2 + 3) * COLUMNS + x) * 2 + 1] = 0x0C;
            }
            for (int y = LINES / 2 - 2; y <= LINES / 2 + 3; y++) {
                screen[(y * COLUMNS + COLUMNS / 2 - 22) * 2] = '|';
                screen[(y * COLUMNS + COLUMNS / 2 + 21) * 2] = '|';
                screen[(y * COLUMNS + COLUMNS / 2 - 22) * 2 + 1] = 0x0C;
                screen[(y * COLUMNS + COLUMNS / 2 + 21) * 2 + 1] = 0x0C;
            }
            const char *msg = "<<< Game Over! X to Restart, Q to Quit >>>";
            for (int i = 0; msg[i]; i++) {
                screen[((LINES / 2) * COLUMNS + (COLUMNS / 2 - 20) + i) * 2] = msg[i];
                screen[((LINES / 2) * COLUMNS + (COLUMNS / 2 - 20) + i) * 2 + 1] = 0x0C;
            }
            draw_number(score, COLUMNS / 2 - 2, LINES / 2 + 1, 0x0F);
            const char *hs_msg = "High Score: ";
            for (int i = 0; hs_msg[i]; i++) {
                screen[((LINES / 2 + 2) * COLUMNS + (COLUMNS / 2 - 6) + i) * 2] = hs_msg[i];
                screen[((LINES / 2 + 2) * COLUMNS + (COLUMNS / 2 - 6) + i) * 2 + 1] = 0x0A;
            }
            draw_number(high_score, COLUMNS / 2 + 5, LINES / 2 + 2, 0x0A);
        }
        return;
    }

    /* Arka plan: gökyüzü ve zemin */
    for (int y = 0; y < LINES - 1; y++) {
        for (int x = 0; x < COLUMNS; x += 10) {
            screen[(y * COLUMNS + x) * 2] = '~'; /* Bulutlar */
            screen[(y * COLUMNS + x) * 2 + 1] = 0x0F; /* Beyaz */
        }
    }
    for (int x = 0; x < COLUMNS; x++) {
        screen[((LINES - 1) * COLUMNS + x) * 2] = '#'; /* Zemin */
        screen[((LINES - 1) * COLUMNS + x) * 2 + 1] = 0x02; /* Yeşil */
    }

    /* Kuşu çiz */
    if (bird_y >= 0 && bird_y < LINES) {
        if (frame % 4 < 2) { /* Kanat yukarı */
            screen[(bird_y * COLUMNS + BIRD_X) * 2] = '/';
            screen[(bird_y * COLUMNS + BIRD_X + 1) * 2] = '*';
            screen[(bird_y * COLUMNS + BIRD_X) * 2 + 1] = 0x0E; /* Sarı kanat */
            screen[(bird_y * COLUMNS + BIRD_X + 1) * 2 + 1] = 0x0C; /* Kırmızı gövde */
        } else { /* Kanat aşağı */
            screen[(bird_y * COLUMNS + BIRD_X) * 2] = '\\';
            screen[(bird_y * COLUMNS + BIRD_X + 1) * 2] = '*';
            screen[(bird_y * COLUMNS + BIRD_X) * 2 + 1] = 0x0E; /* Sarı kanat */
            screen[(bird_y * COLUMNS + BIRD_X + 1) * 2 + 1] = 0x0C; /* Kırmızı gövde */
        }
    }

    /* Boruları çiz */
    for (int i = 0; i < MAX_PIPES; i++) {
        if (pipe_x[i] >= 0 && pipe_x[i] < COLUMNS - PIPE_WIDTH + 1) {
            for (int y = 0; y < LINES; y++) {
                if (y < pipe_gap[i]) { /* Üst boru */
                    screen[(y * COLUMNS + pipe_x[i]) * 2] = '[';
                    screen[(y * COLUMNS + pipe_x[i] + 1) * 2] = '#';
                    screen[(y * COLUMNS + pipe_x[i] + 2) * 2] = '#';
                    screen[(y * COLUMNS + pipe_x[i] + 3) * 2] = '#';
                    screen[(y * COLUMNS + pipe_x[i] + 4) * 2] = ']';
                    for (int j = 0; j < PIPE_WIDTH; j++) {
                        screen[(y * COLUMNS + pipe_x[i] + j) * 2 + 1] = 0x02; /* Yeşil */
                    }
                    if (y == pipe_gap[i] - 1) { /* Üst kapak */
                        for (int j = 0; j < PIPE_WIDTH; j++) {
                            screen[(y * COLUMNS + pipe_x[i] + j) * 2] = '=';
                            screen[(y * COLUMNS + pipe_x[i] + j) * 2 + 1] = 0x0A; /* Açık yeşil */
                        }
                    }
                } else if (y >= pipe_gap[i] + gap_size) { /* Alt boru */
                    screen[(y * COLUMNS + pipe_x[i]) * 2] = '[';
                    screen[(y * COLUMNS + pipe_x[i] + 1) * 2] = '#';
                    screen[(y * COLUMNS + pipe_x[i] + 2) * 2] = '#';
                    screen[(y * COLUMNS + pipe_x[i] + 3) * 2] = '#';
                    screen[(y * COLUMNS + pipe_x[i] + 4) * 2] = ']';
                    for (int j = 0; j < PIPE_WIDTH; j++) {
                        screen[(y * COLUMNS + pipe_x[i] + j) * 2 + 1] = 0x02; /* Yeşil */
                    }
                    if (y == pipe_gap[i] + gap_size) { /* Alt kapak */
                        for (int j = 0; j < PIPE_WIDTH; j++) {
                            screen[(y * COLUMNS + pipe_x[i] + j) * 2] = '=';
                            screen[(y * COLUMNS + pipe_x[i] + j) * 2 + 1] = 0x0A; /* Açık yeşil */
                        }
                    }
                }
            }
        }
    }

    /* Skoru çiz:*/
    for (int x = 0; x < 15; x++) {
        screen[x * 2] = '-';
        screen[x * 2 + 1] = 0x0F;
        screen[(1 * COLUMNS + x) * 2] = '-';
        screen[(1 * COLUMNS + x) * 2 + 1] = 0x0F;
    }
    screen[0 * 2] = '|';
    screen[14 * 2] = '|';
    screen[(1 * COLUMNS) * 2] = '|';
    screen[(1 * COLUMNS + 14) * 2] = '|';
    const char *msg = "Score: ";
    for (int i = 0; msg[i]; i++) {
        screen[(0 * COLUMNS + 2 + i) * 2] = msg[i];
        screen[(0 * COLUMNS + 2 + i) * 2 + 1] = 0x02;
    }
    draw_number(score, 9, 0, 0x0F);
}

void update() {
    if (!running) return;
    frame++; // animasyon için.
    update_counter++;
    if (update_counter >= UPDATE_INTERVAL) { //Her 15 karede 1
        update_counter = 0;
        bird_vel += GRAVITY;
        if (flap) {
            bird_vel = FLAP_VELOCITY;
            flap = 0;
        }
        bird_y += bird_vel;

        for (int i = 0; i < MAX_PIPES; i++) {
            pipe_x[i] -= PIPE_SPEED;
            if (pipe_x[i] < -PIPE_WIDTH) {
                pipe_x[i] = COLUMNS - 1;
                pipe_gap[i] = 5 + (rand() % (LINES - gap_size - 10));
                score++;
                gap_size = GAP_SIZE_INITIAL - (score / 10);
                if (gap_size < 3) gap_size = 3;
            }
        }
		// Sınır kontrolü
        bird_y = (bird_y < 0) ? 0 : (bird_y >= LINES) ? LINES - 1 : bird_y; 
        for (int i = 0; i < MAX_PIPES; i++) {
            pipe_x[i] = (pipe_x[i] < -PIPE_WIDTH) ? -PIPE_WIDTH : (pipe_x[i] >= COLUMNS) ? COLUMNS - 1 : pipe_x[i];
            pipe_gap[i] = (pipe_gap[i] < 0) ? 0 : (pipe_gap[i] >= LINES) ? LINES - 1 : pipe_gap[i];
            if (pipe_gap[i] + gap_size >= LINES) pipe_gap[i] = LINES - gap_size - 1;
        }

        if (bird_y <= 0 || bird_y >= LINES - 1) {
            running = 0;
            game_over = 1;
            if (score > high_score) high_score = score;
        }
        for (int i = 0; i < MAX_PIPES; i++) {
            if (pipe_x[i] >= BIRD_X - PIPE_WIDTH && pipe_x[i] <= BIRD_X + 2) { // +2 kuşun sağ tarafı kuş 2 karekter kaplıyor.
                if (bird_y < pipe_gap[i] || bird_y >= pipe_gap[i] + gap_size) {
                    running = 0;
                    game_over = 1;
                    if (score > high_score) high_score = score;
                }
            }
        }
    }
}

void delay() {
    for (int i = 0; i < 99000000 / FPS; i++);
    write_port(0x21, 0xFD);
}

int rand(void) {
    static unsigned int seed = 0xDEADBEEF;
    seed = (seed * 1103515245 + 12345) & 0x7fffffff;
    return seed;
}

void idt_init() {
    unsigned long kb_handler_addr = (unsigned long)keyboard_handler;
    IDT[0x21].offset_low = kb_handler_addr & 0xFFFF;
    IDT[0x21].selector = KERNEL_CS;
    IDT[0x21].zero = 0;
    IDT[0x21].type_attr = INT_GATE; // 32 bit
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

void kb_init() {
    write_port(0x21, 0xFD); // Klavye kesmesi.
}

// Klavye kesmelerini işler
void keyboard_handler_main() {
    // Klavye durumunu kontrol et
    unsigned char status = read_port(KB_STATUS_PORT);

    if (status & 0x01) { // bit 0 1 ise veri var demek;
        char keycode = read_port(KB_DATA_PORT); // tuşu aldı 
        unsigned char key = (keycode & 0x80) ? 0 : keyboard_map[(unsigned char)keycode]; // press or release.

        if (keycode & 0x80) { /* Tuş bırakma */ // 1
            if ((keycode & 0x7F) == 0x39) { /* Space bırakıldı */
                space_pressed = 0;
            }
        } else { /* Tuş basma */ // 0
            if (key == 'x' && !running && !exit_game) {
                reset_game();
            } else if (key == SPACEBAR && running && !space_pressed) {
                flap = 1;
                space_pressed = 1;
            } else if (key == ESCAPE && !exit_game) {
                running = !running;
            } else if (keycode == Q_KEY && game_over && !exit_game) {
                exit_game = 1;
            }
        }

        while (read_port(KB_STATUS_PORT) & 0x01) {
            read_port(KB_DATA_PORT);
        }

        write_port(0x20, 0x20);
        write_port(0xA0, 0x20);
    }
}

// Ana çekirdek fonksiyonu
void kmain() {
    // Başlangıç işlemlerini gerçekleştir
    clear_screen();
    idt_init();
    kb_init();
    while (1) {
        if (exit_game) {
            draw();
            return;
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
