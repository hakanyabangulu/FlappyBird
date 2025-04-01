#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <vector>

#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 700
#define GRAVITY 0.35f
#define JUMP -6.5f
#define BIRD_X 100
#define PIPE_GAP 180
#define PIPE_WIDTH 60
#define PIPE_COUNT 3
#define PIPE_SPEED_BASE 4.5f
#define PIPE_MIN_GAP_Y 150
#define PIPE_MAX_GAP_Y 400
#define GROUND_HEIGHT 120
#define FPS 60

struct Pipe {
    float x;
    int gapY;
    bool scored;
};

struct Particle {
    float x, y;
    float vx, vy;
    int alpha;
    float lifetime;
};

class FlappyBird {
private:
    float birdY;
    float velocity;
    int score;
    int highScore;
    bool gameOver;
    bool gameStarted;
    bool paused;
    bool spacePressed;
    Pipe pipes[PIPE_COUNT];
    float pipeSpeed;
    float transitionTimer;
    float cloudOffset;

    // Kuş bileşenleri
    SDL_Rect birdBody;
    SDL_Rect birdWingUp;
    SDL_Rect birdWingDown;
    SDL_Rect birdBeak;
    SDL_Rect birdEye;
    SDL_Rect birdPupil;
    int birdFrame;
    float birdAnimationTimer;
    float birdTilt;

    // Parçacıklar
    std::vector<Particle> particles;

    TTF_Font* font;
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color red = {200, 0, 0, 255};
    SDL_Color black = {0, 0, 0, 255};
    SDL_Color yellow = {255, 215, 0, 255};

public:
    FlappyBird() : 
        birdY(WINDOW_HEIGHT / 2), 
        velocity(0), 
        score(0), 
        highScore(0), 
        gameOver(false), 
        gameStarted(false), 
        paused(false), 
        spacePressed(false), 
        pipeSpeed(PIPE_SPEED_BASE), 
        transitionTimer(0.0f), 
        cloudOffset(0.0f), 
        birdFrame(0), 
        birdAnimationTimer(0.0f), 
        birdTilt(0.0f) {
        initGame();
    }

    ~FlappyBird() {
        TTF_CloseFont(font);
    }

    void initGame() {
        srand(static_cast<unsigned>(time(0)));

        // Kuş tasarımı
        birdBody = {BIRD_X - 18, static_cast<int>(birdY) - 12, 36, 24};
        birdWingUp = {BIRD_X - 5, static_cast<int>(birdY) - 15, 20, 12};
        birdWingDown = {BIRD_X - 5, static_cast<int>(birdY) + 3, 20, 12};
        birdBeak = {BIRD_X + 18, static_cast<int>(birdY) - 2, 10, 5};
        birdEye = {BIRD_X + 10, static_cast<int>(birdY) - 6, 6, 6};
        birdPupil = {BIRD_X + 12, static_cast<int>(birdY) - 5, 2, 2};

        resetPipes();

        if (TTF_Init() == -1) {
            std::cerr << "TTF_Init: " << TTF_GetError() << std::endl;
            exit(1);
        }
        font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 30);
        if (!font) {
            std::cerr << "TTF_OpenFont: " << TTF_GetError() << std::endl;
            exit(1);
        }
    }

    void update(float deltaTime) {
        if (!gameStarted) {
            transitionTimer += deltaTime;
            birdTilt = sin(transitionTimer * 4.0f) * 5.0f;
            updateBirdPosition();
            return;
        }
        if (gameOver || paused) {
            if (gameOver) transitionTimer += deltaTime;
            updateParticles(deltaTime);
            return;
        }

        // Fizik
        velocity += GRAVITY * deltaTime * FPS;
        birdY += velocity * deltaTime * FPS;
        birdTilt = std::min(std::max(velocity * 2.0f, -15.0f), 30.0f);

        // Animasyon
        birdAnimationTimer += deltaTime * 20.0f;
        if (birdAnimationTimer >= 1.0f) {
            birdAnimationTimer = 0.0f;
            birdFrame = (birdFrame + 1) % 2;
        }

        // Borular ve bulutlar
        pipeSpeed = PIPE_SPEED_BASE + (score / 20.0f);
        if (pipeSpeed > 8.0f) pipeSpeed = 8.0f;
        cloudOffset += deltaTime * 20.0f;
        if (cloudOffset > WINDOW_WIDTH) cloudOffset -= WINDOW_WIDTH;

        for (int i = 0; i < PIPE_COUNT; i++) {
            pipes[i].x -= pipeSpeed * deltaTime * FPS;
            if (pipes[i].x + PIPE_WIDTH < 0) {
                pipes[i].x = WINDOW_WIDTH + (rand() % 100);
                pipes[i].gapY = PIPE_MIN_GAP_Y + rand() % (PIPE_MAX_GAP_Y - PIPE_MIN_GAP_Y - PIPE_GAP + 1);
                pipes[i].scored = false;
            }
            if (!pipes[i].scored && pipes[i].x + PIPE_WIDTH < BIRD_X) {
                score++;
                pipes[i].scored = true;
                spawnScoreParticles();
            }
        }

        checkCollisions();
        updateBirdPosition();
        updateParticles(deltaTime);
    }

    void updateBirdPosition() {
        birdBody.y = static_cast<int>(birdY) - 12;
        birdWingUp.y = static_cast<int>(birdY) - 15 + static_cast<int>(birdTilt / 3);
        birdWingDown.y = static_cast<int>(birdY) + 3 + static_cast<int>(birdTilt / 3);
        birdBeak.y = static_cast<int>(birdY) - 2;
        birdEye.y = static_cast<int>(birdY) - 6;
        birdPupil.y = static_cast<int>(birdY) - 5;
    }

    void checkCollisions() {
        float groundHeight = WINDOW_HEIGHT - GROUND_HEIGHT;
        if (birdY + 12 >= groundHeight) {
            birdY = groundHeight - 12;
            velocity = 0;
            gameOver = true;
            spawnCrashParticles();
        }
        if (birdY - 12 < 0) {
            birdY = 12;
            velocity = 0;
        }

        for (int i = 0; i < PIPE_COUNT; i++) {
            if (BIRD_X + 18 >= pipes[i].x && BIRD_X - 18 <= pipes[i].x + PIPE_WIDTH &&
                (birdY - 12 < pipes[i].gapY || birdY + 12 > pipes[i].gapY + PIPE_GAP)) {
                gameOver = true;
                spawnCrashParticles();
            }
        }
    }

    void updateParticles(float deltaTime) {
        for (auto it = particles.begin(); it != particles.end();) {
            it->x += it->vx * deltaTime * FPS;
            it->y += it->vy * deltaTime * FPS;
            it->lifetime -= deltaTime;
            it->alpha = static_cast<int>(255 * (it->lifetime / 1.0f));
            if (it->lifetime <= 0) {
                it = particles.erase(it);
            } else {
                ++it;
            }
        }
    }

    void spawnCrashParticles() {
        for (int i = 0; i < 20; i++) {
            Particle p;
            p.x = BIRD_X;
            p.y = birdY;
            p.vx = (rand() % 5 - 2) * 1.0f;
            p.vy = (rand() % 5 - 2) * 1.0f;
            p.alpha = 255;
            p.lifetime = 1.0f;
            particles.push_back(p);
        }
    }

    void spawnScoreParticles() {
        for (int i = 0; i < 5; i++) {
            Particle p;
            p.x = BIRD_X;
            p.y = birdY - 20;
            p.vx = (rand() % 3 - 1) * 0.5f;
            p.vy = -1.0f - (rand() % 2) * 0.5f;
            p.alpha = 255;
            p.lifetime = 0.5f;
            particles.push_back(p);
        }
    }

    void draw(SDL_Renderer* renderer) {
    // Gökyüzü
    SDL_SetRenderDrawColor(renderer, 70, 130, 180, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 135, 206, 235, 255);
    SDL_Rect skyBottom = {0, WINDOW_HEIGHT / 3, WINDOW_WIDTH, WINDOW_HEIGHT * 2 / 3};
    SDL_RenderFillRect(renderer, &skyBottom);

    // Bulutlar
    drawClouds(renderer);

    // Güneş (Sağ üst köşe)
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // Parlak sarı dış halka
    SDL_Rect sunOuter = {WINDOW_WIDTH - 100, 50, 60, 60};
    SDL_RenderFillRect(renderer, &sunOuter); // Kare taban, yuvarlak efekt için aşağıda iç halka ekliyoruz
    SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255); // Daha yumuşak sarı iç halka
    SDL_Rect sunInner = {WINDOW_WIDTH - 90, 60, 40, 40};
    SDL_RenderFillRect(renderer, &sunInner);

    // Zemin
    SDL_SetRenderDrawColor(renderer, 34, 139, 34, 255); // Çim
    SDL_Rect grass = {0, WINDOW_HEIGHT - GROUND_HEIGHT, WINDOW_WIDTH, GROUND_HEIGHT / 2};
    SDL_RenderFillRect(renderer, &grass);
    SDL_SetRenderDrawColor(renderer, 50, 205, 50, 255); // Açık çim
    SDL_Rect grassTop = {0, WINDOW_HEIGHT - GROUND_HEIGHT - 20, WINDOW_WIDTH, 20};
    SDL_RenderFillRect(renderer, &grassTop);
    SDL_SetRenderDrawColor(renderer, 139, 69, 19, 255); // Toprak
    SDL_Rect dirt = {0, WINDOW_HEIGHT - GROUND_HEIGHT / 2, WINDOW_WIDTH, GROUND_HEIGHT / 2};
    SDL_RenderFillRect(renderer, &dirt);

    // Borular
    for (int i = 0; i < PIPE_COUNT; i++) {
        SDL_SetRenderDrawColor(renderer, 34, 139, 34, 255); // Boru gövdesi
        SDL_Rect bottomPipe = {static_cast<int>(pipes[i].x), pipes[i].gapY + PIPE_GAP,
                              PIPE_WIDTH, WINDOW_HEIGHT - GROUND_HEIGHT - (pipes[i].gapY + PIPE_GAP)};
        SDL_Rect topPipe = {static_cast<int>(pipes[i].x), 0, PIPE_WIDTH, pipes[i].gapY};
        SDL_RenderFillRect(renderer, &bottomPipe);
        SDL_RenderFillRect(renderer, &topPipe);

        // Boru kenarları
        SDL_SetRenderDrawColor(renderer, 0, 100, 0, 255);
        SDL_Rect bottomEdge = {static_cast<int>(pipes[i].x), pipes[i].gapY + PIPE_GAP - 10, PIPE_WIDTH, 10};
        SDL_Rect topEdge = {static_cast<int>(pipes[i].x), pipes[i].gapY, PIPE_WIDTH, 10};
        SDL_RenderFillRect(renderer, &bottomEdge);
        SDL_RenderFillRect(renderer, &topEdge);

        // Gölge efekti
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 50);
        SDL_Rect shadow = {static_cast<int>(pipes[i].x) + 5, pipes[i].gapY + PIPE_GAP,
                          PIPE_WIDTH - 5, WINDOW_HEIGHT - GROUND_HEIGHT - (pipes[i].gapY + PIPE_GAP)};
        SDL_RenderFillRect(renderer, &shadow);
    }

    // Parçacıklar
    for (const auto& p : particles) {
        SDL_SetRenderDrawColor(renderer, 255, 215, 0, p.alpha);
        SDL_Rect particleRect = {static_cast<int>(p.x), static_cast<int>(p.y), 4, 4};
        SDL_RenderFillRect(renderer, &particleRect);
    }

    // Kuş
    SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255); // Gövde
    SDL_RenderFillRect(renderer, &birdBody);
    SDL_SetRenderDrawColor(renderer, 255, 165, 0, 255); // Kanat
    SDL_RenderFillRect(renderer, birdFrame ? &birdWingUp : &birdWingDown);
    SDL_SetRenderDrawColor(renderer, 255, 69, 0, 255);  // Gaga
    SDL_RenderFillRect(renderer, &birdBeak);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // Göz beyazı
    SDL_RenderFillRect(renderer, &birdEye);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);     // Göz bebeği
    SDL_RenderFillRect(renderer, &birdPupil);

    // Gölge
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 50);
    SDL_Rect birdShadow = {BIRD_X - 15, static_cast<int>(birdY) - 9 + 5, 30, 18};
    SDL_RenderFillRect(renderer, &birdShadow);

    // UI
    drawText(renderer, "Score: " + std::to_string(score), WINDOW_WIDTH / 2 - 70, 20, white);
    drawText(renderer, "High Score: " + std::to_string(highScore), WINDOW_WIDTH / 2 - 70, 50, yellow);
    if (gameOver) {
        drawText(renderer, "GAME OVER!\nPress R to Restart or Q to Quit",
                WINDOW_WIDTH / 2 - 200, WINDOW_HEIGHT / 2 - 50 + static_cast<int>(transitionTimer * 50), red);
    } else if (!gameStarted) {
        drawText(renderer, "Flappy Bird\nPress Space to Start",
                WINDOW_WIDTH / 2 - 170, WINDOW_HEIGHT / 2 - 50 - static_cast<int>(transitionTimer * 50), black);
    } else if (paused) {
        drawText(renderer, "PAUSED", WINDOW_WIDTH / 2 - 80, WINDOW_HEIGHT / 2, black);
    }

    SDL_RenderPresent(renderer);
}

    void drawClouds(SDL_Renderer* renderer) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 220);
        SDL_Rect cloud1 = {static_cast<int>(250 - cloudOffset), 100, 140, 70};
        SDL_Rect cloud2 = {static_cast<int>(600 - cloudOffset), 150, 160, 80};
        SDL_Rect cloud3 = {static_cast<int>(900 - cloudOffset), 120, 120, 60};
        SDL_RenderFillRect(renderer, &cloud1);
        SDL_RenderFillRect(renderer, &cloud2);
        SDL_RenderFillRect(renderer, &cloud3);

        // Yumuşak kenarlar için ek katmanlar
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 150);
        SDL_Rect cloud1Inner = {static_cast<int>(255 - cloudOffset), 105, 130, 60};
        SDL_Rect cloud2Inner = {static_cast<int>(605 - cloudOffset), 155, 150, 70};
        SDL_Rect cloud3Inner = {static_cast<int>(905 - cloudOffset), 125, 110, 50};
        SDL_RenderFillRect(renderer, &cloud1Inner);
        SDL_RenderFillRect(renderer, &cloud2Inner);
        SDL_RenderFillRect(renderer, &cloud3Inner);
    }

    void drawText(SDL_Renderer* renderer, const std::string& text, int x, int y, SDL_Color color) {
        SDL_Surface* surface = TTF_RenderText_Blended_Wrapped(font, text.c_str(), color, WINDOW_WIDTH);
        if (!surface) return;
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        if (!texture) {
            SDL_FreeSurface(surface);
            return;
        }
        SDL_Rect dst = {x, y, surface->w, surface->h};
        SDL_RenderCopy(renderer, texture, NULL, &dst);
        SDL_FreeSurface(surface);
        SDL_DestroyTexture(texture);
    }

    void handleInput(SDL_Event& event, SDL_Window* window) {
        if (event.type == SDL_QUIT) {
            SDL_DestroyWindow(window);
            SDL_Quit();
            exit(0);
        }

        if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_SPACE && !spacePressed) {
                spacePressed = true;
                if (!gameStarted) gameStarted = true;
                velocity = JUMP;
            }
            if (event.key.keysym.sym == SDLK_p) paused = !paused;
            if (gameOver && transitionTimer > 0.5f) {
                if (event.key.keysym.sym == SDLK_r) reset();
                if (event.key.keysym.sym == SDLK_q) {
                    SDL_DestroyWindow(window);
                    SDL_Quit();
                    exit(0);
                }
            }
        }
        if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_SPACE) {
            spacePressed = false;
        }
    }

    void reset() {
        birdY = WINDOW_HEIGHT / 2;
        velocity = 0;
        if (score > highScore) highScore = score;
        score = 0;
        gameOver = false;
        birdFrame = 0;
        birdAnimationTimer = 0.0f;
        pipeSpeed = PIPE_SPEED_BASE;
        transitionTimer = 0.0f;
        cloudOffset = 0.0f;
        resetPipes();
        gameStarted = false;
        particles.clear();
    }

private:
    void resetPipes() {
        for (int i = 0; i < PIPE_COUNT; i++) {
            pipes[i].x = WINDOW_WIDTH + (i * (WINDOW_WIDTH / PIPE_COUNT)) + (rand() % 50);
            pipes[i].gapY = PIPE_MIN_GAP_Y + rand() % (PIPE_MAX_GAP_Y - PIPE_MIN_GAP_Y - PIPE_GAP + 1);
            pipes[i].scored = false;
        }
    }
};

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL_Init: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Flappy Bird: Enhanced",
                                         SDL_WINDOWPOS_CENTERED,
                                         SDL_WINDOWPOS_CENTERED,
                                         WINDOW_WIDTH, WINDOW_HEIGHT,
                                         SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "SDL_CreateWindow: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "SDL_CreateRenderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    FlappyBird game;
    Uint32 lastTime = SDL_GetTicks();

    while (true) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            game.handleInput(event, window);
        }

        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = std::min((currentTime - lastTime) / 1000.0f, 0.1f);
        lastTime = currentTime;

        game.update(deltaTime);
        game.draw(renderer);

        SDL_Delay(1000 / FPS);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}