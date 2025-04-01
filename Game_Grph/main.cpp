#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <string>
#include <cmath>

#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 700
#define GRAVITY 0.40f
#define JUMP -6.5f
#define BIRD_X 100
#define PIPE_GAP 200
#define PIPE_WIDTH 70
#define PIPE_COUNT 3
#define PIPE_SPEED 4.0f
#define PIPE_MIN_GAP_Y 150
#define PIPE_MAX_GAP_Y 400
#define MAX_PIPES 50

struct Pipe {
    float x;
    int gapY;
    bool scored;
};
struct Cloud {
    float x, y;
    float speed;
};
class FlappyBird {
private:
    float birdY, velocity;
    int score, highScore, lives;
    bool gameOver, gameStarted, paused, menuActive, spacePressed, invincible;
    float invincibilityTimer;
    Pipe pipes[PIPE_COUNT];
    std::vector<Cloud> clouds;
    int pipesPassed;
    // Bird components
    sf::RectangleShape birdBody;
    sf::ConvexShape birdWingLeft, birdWingRight;
    sf::ConvexShape birdBeak;
    sf::CircleShape birdEye, birdPupil;
    int birdFrame;
    float birdAnimationTimer;
    // UI elements
    sf::Font font;
    sf::Text scoreText, highScoreText, gameOverText, welcomeText, pauseText, livesText;
    // Visual enhancements
    sf::CircleShape sun;
    std::vector<sf::CircleShape> particles;
    std::vector<sf::CircleShape> sunRays;
    // Private helper functions (sadece bildirim)
    void resetPipes();
    void checkCollisions();
    void updateClouds(float deltaTime);
    void setupText();
    void initGame();

public:
    FlappyBird() : 
        birdY(WINDOW_HEIGHT / 2), 
        velocity(0), 
        score(0), 
        highScore(0), 
        lives(2),
        gameOver(false), 
        gameStarted(false), 
        paused(false), 
        menuActive(true),
        spacePressed(false), 
        invincible(false), 
        invincibilityTimer(0.0f),
        pipesPassed(0),
        birdFrame(0), 
        birdAnimationTimer(0.0f)
    {
        initGame();
    }
    void update(float deltaTime);
    void draw(sf::RenderWindow& window);
    void handleInput(sf::Event& event, sf::RenderWindow& window);
    void reset();
};
// Fonksiyon tanımları (implementations) sınıf dışında
void FlappyBird::initGame() {
    srand(static_cast<unsigned>(time(0)));
    // Bird setup
    birdBody.setSize(sf::Vector2f(35, 25));
    birdBody.setFillColor(sf::Color(255, 215, 0));
    birdBody.setOutlineThickness(2);
    birdBody.setOutlineColor(sf::Color::Black);
    birdBody.setOrigin(17.5f, 12.5f);

    birdWingLeft.setPointCount(5);
    birdWingLeft.setPoint(0, sf::Vector2f(0, 0));
    birdWingLeft.setPoint(1, sf::Vector2f(-18, -6));
    birdWingLeft.setPoint(2, sf::Vector2f(-25, 10));
    birdWingLeft.setPoint(3, sf::Vector2f(-15, 18));
    birdWingLeft.setPoint(4, sf::Vector2f(0, 12));
    birdWingLeft.setFillColor(sf::Color(255, 165, 0));

    birdWingRight.setPointCount(5);
    birdWingRight.setPoint(0, sf::Vector2f(0, 0));
    birdWingRight.setPoint(1, sf::Vector2f(18, -6));
    birdWingRight.setPoint(2, sf::Vector2f(25, 10));
    birdWingRight.setPoint(3, sf::Vector2f(15, 18));
    birdWingRight.setPoint(4, sf::Vector2f(0, 12));
    birdWingRight.setFillColor(sf::Color(255, 165, 0));

    birdBeak.setPointCount(3);
    birdBeak.setPoint(0, sf::Vector2f(0, 0));
    birdBeak.setPoint(1, sf::Vector2f(15, -3));
    birdBeak.setPoint(2, sf::Vector2f(15, 3));
    birdBeak.setFillColor(sf::Color(255, 69, 0));

    birdEye.setRadius(4);
    birdEye.setFillColor(sf::Color::Black);
    birdPupil.setRadius(1.5f);
    birdPupil.setFillColor(sf::Color::White);

    // Initial setups
    resetPipes();
    // Font setup
    if (!font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf")) {
        std::cerr << "Font loading failed!" << std::endl;
        exit(1);
    }

    setupText();
    // Visuals
    float sunCenterX = WINDOW_WIDTH - 150 + 50; // Güneşin x koordinatı (yarıçap eklenerek merkez bulunur)
	float sunCenterY = 100 + 50; // Güneşin y koordinatı (yarıçap eklenerek merkez bulunur)

	sun.setRadius(50);
	sun.setFillColor(sf::Color(255, 215, 0));
	sun.setPosition(WINDOW_WIDTH - 150, 100);

	sunRays.clear(); // Önceki ışınları temizle

	int numRays = 12; // 12 ışın olacak
	float rayDistance = 70; // Güneşin merkezinden ışınların konumu
	float rayRadius = 10; // Işınların boyutu

	for (int i = 0; i < numRays; i++) {
		float angle = i * (360.0f / numRays); // 12 ışın için açıyı belirle
		float radian = angle * (3.14159265359f / 180.0f); // Açıları derece yerine radyana çevir

		// Trigonometrik hesaplamalarla ışınların pozisyonlarını belirleyelim
		float x = sunCenterX + cos(radian) * rayDistance - rayRadius;
		float y = sunCenterY + sin(radian) * rayDistance - rayRadius;

		sf::CircleShape ray(rayRadius);
		ray.setFillColor(sf::Color(255, 215, 0, 100));
		ray.setPosition(x, y);

		sunRays.push_back(ray);
	}
}
void FlappyBird::setupText() {
    scoreText.setFont(font);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setCharacterSize(30);
    scoreText.setStyle(sf::Text::Bold);
    scoreText.setPosition(WINDOW_WIDTH / 2 - 70, 20);

    highScoreText.setFont(font);
    highScoreText.setFillColor(sf::Color::Yellow);
    highScoreText.setCharacterSize(20);
    highScoreText.setPosition(WINDOW_WIDTH / 2 - 70, 60);

    livesText.setFont(font);
    livesText.setFillColor(sf::Color::Red);
    livesText.setCharacterSize(20);
    livesText.setPosition(20, 20);

    gameOverText.setFont(font);
    gameOverText.setFillColor(sf::Color(200, 0, 0));
    gameOverText.setCharacterSize(40);
    gameOverText.setStyle(sf::Text::Bold);
    gameOverText.setString("GAME OVER!\nPress R to Restart or Q to Quit");
    gameOverText.setPosition(WINDOW_WIDTH / 2 - 200, WINDOW_HEIGHT / 2 - 50);

    welcomeText.setFont(font);
    welcomeText.setFillColor(sf::Color::Black);
    welcomeText.setCharacterSize(40);
    welcomeText.setStyle(sf::Text::Bold);
    welcomeText.setString("Flappy Bird\nPress Space to Start");
    welcomeText.setPosition(WINDOW_WIDTH / 2 - 170, WINDOW_HEIGHT / 2 - 50);

    pauseText.setFont(font);
    pauseText.setFillColor(sf::Color::Black);
    pauseText.setCharacterSize(40);
    pauseText.setStyle(sf::Text::Bold);
    pauseText.setString("PAUSED");
    pauseText.setPosition(WINDOW_WIDTH / 2 - 80, WINDOW_HEIGHT / 2);
}
void FlappyBird::update(float deltaTime) {
    if (menuActive || gameOver || paused) return;
    if (!gameStarted) return;
    // Physics
    velocity += GRAVITY * deltaTime * 60.0f;
    birdY += velocity * deltaTime * 60.0f;

    // Bird animation
    birdAnimationTimer += deltaTime * 15.0f;
    if (birdAnimationTimer >= 1.0f) {
        birdAnimationTimer = 0.0f;
        birdFrame = !birdFrame;
    }
    // Pipe movement
    float dynamicSpeed = PIPE_SPEED + score * 0.05f;
    if (dynamicSpeed > 7.0f) dynamicSpeed = 7.0f;
    for (int i = 0; i < PIPE_COUNT; i++) {
        pipes[i].x -= dynamicSpeed * deltaTime * 60.0f;
        if (pipes[i].x + PIPE_WIDTH < 0) {
            pipes[i].x = WINDOW_WIDTH;
            pipes[i].gapY = rand() % (PIPE_MAX_GAP_Y - PIPE_MIN_GAP_Y + 1) + PIPE_MIN_GAP_Y;
            pipes[i].scored = false;
            pipesPassed++;
        }
        if (!pipes[i].scored && pipes[i].x + PIPE_WIDTH < BIRD_X) {
            score++;
            pipes[i].scored = true;
        }
    }
    // Invincibility
    if (invincible) {
        invincibilityTimer -= deltaTime;
        if (invincibilityTimer <= 0) invincible = false;
    }

    // Collision detection
    if (!invincible) checkCollisions();

    // Clouds
    updateClouds(deltaTime);
}
void FlappyBird::updateClouds(float deltaTime) {
    for (auto it = clouds.begin(); it != clouds.end();) {
        it->x -= it->speed * deltaTime * 60.0f;
        if (it->x < -100) {
            it->x = WINDOW_WIDTH + 50;
            it->y = rand() % 200 + 50;
            it->speed = 0.5f + rand() % 2;
        }
        ++it;
    }
}

void FlappyBird::checkCollisions() {
    float groundHeight = WINDOW_HEIGHT - 100;
    if (birdY + 12.5f >= groundHeight) {
        if (lives > 1) {
            lives--;
            birdY = WINDOW_HEIGHT / 2;
            velocity = 0;
            
        } else {
            birdY = groundHeight - 12.5f;
            velocity = 0;
            gameOver = true;
            
        }
    }
    if (birdY - 12.5f < 0) {
        birdY = 12.5f;
        velocity = 0;
    }

    for (int i = 0; i < PIPE_COUNT; i++) {
        if (BIRD_X + 17.5f >= pipes[i].x && BIRD_X - 17.5f <= pipes[i].x + PIPE_WIDTH &&
            (birdY - 12.5f < pipes[i].gapY || birdY + 12.5f > pipes[i].gapY + PIPE_GAP)) {
            if (lives > 1) {
                lives--;
                birdY = WINDOW_HEIGHT / 2;
                velocity = 0;
                
            } else {
                gameOver = true;
            }
        }
    }
}
void FlappyBird::draw(sf::RenderWindow& window) {
    window.clear();

    // Gradient background
    sf::VertexArray gradient(sf::Quads, 4);
    gradient[0].position = sf::Vector2f(0, 0);
    gradient[1].position = sf::Vector2f(WINDOW_WIDTH, 0);
    gradient[2].position = sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT);
    gradient[3].position = sf::Vector2f(0, WINDOW_HEIGHT);
    gradient[0].color = sf::Color(100, 150, 255);
    gradient[1].color = sf::Color(100, 150, 255);
    gradient[2].color = sf::Color(200, 230, 255);
    gradient[3].color = sf::Color(200, 230, 255);
    window.draw(gradient);

    // Clouds
    for (const auto& cloud : clouds) {
        sf::CircleShape c1(30), c2(20), c3(25);
        c1.setFillColor(sf::Color(255, 255, 255, 200));
        c2.setFillColor(sf::Color(255, 255, 255, 200));
        c3.setFillColor(sf::Color(255, 255, 255, 200));
        c1.setPosition(cloud.x, cloud.y);
        c2.setPosition(cloud.x + 20, cloud.y + 10);
        c3.setPosition(cloud.x + 40, cloud.y + 5);
        window.draw(c1);
        window.draw(c2);
        window.draw(c3);
    }

    // Sun with evenly spaced rays
	sun.setRadius(50);
	sun.setOrigin(50, 50);
	sun.setPosition(WINDOW_WIDTH - 150, 100);
	window.draw(sun);

	const float centerX = WINDOW_WIDTH - 150;
	const float centerY = 100;
	const float rayDistance = 80; // Güneşin etrafındaki ışınların mesafesi
	const size_t rayCount = sunRays.size();
	const float angleStep = 360.0f / rayCount;

	for (size_t i = 0; i < rayCount; i++) {
		float angle = i * angleStep;
		float radian = angle * 3.14159265359f / 180.0f; // Dereceyi radyana çeviriyoruz

		float rayX = centerX + cos(radian) * rayDistance;
		float rayY = centerY + sin(radian) * rayDistance;

		sunRays[i].setOrigin(sunRays[i].getRadius(), sunRays[i].getRadius()); // Işının merkezini ayarlıyoruz
		sunRays[i].setPosition(rayX, rayY);
		window.draw(sunRays[i]);
	}


    // Ground
    sf::RectangleShape ground(sf::Vector2f(WINDOW_WIDTH, 100));
    ground.setPosition(0, WINDOW_HEIGHT - 100);
    ground.setFillColor(sf::Color(50, 205, 50));
    window.draw(ground);
    
    sf::RectangleShape dirt(sf::Vector2f(WINDOW_WIDTH, 20));
    dirt.setPosition(0, WINDOW_HEIGHT - 100);
    dirt.setFillColor(sf::Color(139, 69, 19));
    window.draw(dirt);

    // Pipes
    for (int i = 0; i < PIPE_COUNT; i++) {
        float groundLevel = WINDOW_HEIGHT - 100;
        sf::RectangleShape bottomPipe(sf::Vector2f(PIPE_WIDTH, groundLevel - (pipes[i].gapY + PIPE_GAP)));
        bottomPipe.setPosition(pipes[i].x, pipes[i].gapY + PIPE_GAP);
        bottomPipe.setFillColor(sf::Color(34, 139, 34));
        bottomPipe.setOutlineThickness(3);
        bottomPipe.setOutlineColor(sf::Color(0, 100, 0));
        window.draw(bottomPipe); // Hata düzeltildi: bottomPipe olacak
        window.draw(bottomPipe);

        sf::RectangleShape topPipe(sf::Vector2f(PIPE_WIDTH, pipes[i].gapY));
        topPipe.setPosition(pipes[i].x, 0);
        topPipe.setFillColor(sf::Color(34, 139, 34));
        topPipe.setOutlineThickness(3);
        topPipe.setOutlineColor(sf::Color(0, 100, 0));
        window.draw(topPipe);

        sf::RectangleShape shadow(sf::Vector2f(5, groundLevel - (pipes[i].gapY + PIPE_GAP)));
        shadow.setPosition(pipes[i].x + PIPE_WIDTH, pipes[i].gapY + PIPE_GAP);
        shadow.setFillColor(sf::Color(0, 0, 0, 50));
        window.draw(shadow);
    }
    float birdAngle = velocity * 2.0f;
    birdBody.setPosition(BIRD_X, birdY);
    birdBody.setRotation(birdAngle);
    birdWingLeft.setPosition(BIRD_X, birdY + (birdFrame ? -5 : 5));
    birdWingLeft.setRotation(birdAngle);
    birdWingRight.setPosition(BIRD_X, birdY + (birdFrame ? -5 : 5));
    birdWingRight.setRotation(birdAngle);
    birdBeak.setPosition(BIRD_X + 17.5f, birdY);
    birdBeak.setRotation(birdAngle);
    birdEye.setPosition(BIRD_X + 12, birdY - 5);
    birdPupil.setPosition(BIRD_X + 13, birdY - 5);
    if (invincible && int(invincibilityTimer * 10) % 2 == 0) {
        birdBody.setFillColor(sf::Color(255, 255, 255, 150));
    } else {
        birdBody.setFillColor(sf::Color(255, 215, 0));
    }
    window.draw(birdBody);
    window.draw(birdWingLeft);
    window.draw(birdWingRight);
    window.draw(birdBeak);
    window.draw(birdEye);
    window.draw(birdPupil);

    // Particles
    for (auto it = particles.begin(); it != particles.end();) {
        it->move((rand() % 5 - 2), 2);
        it->setFillColor(sf::Color(it->getFillColor().r, it->getFillColor().g, it->getFillColor().b, it->getFillColor().a - 5));
        if (it->getFillColor().a <= 0) it = particles.erase(it);
        else {
            window.draw(*it);
            ++it;
        }
    }

    scoreText.setString("Score: " + std::to_string(score));
    highScoreText.setString("High Score: " + std::to_string(highScore));
    livesText.setString("Lives: " + std::to_string(lives));
    window.draw(scoreText);
    window.draw(highScoreText);
    window.draw(livesText);
    if (menuActive) window.draw(welcomeText);
    else if (gameOver) window.draw(gameOverText);
    else if (paused) window.draw(pauseText);

    window.display();
}
void FlappyBird::handleInput(sf::Event& event, sf::RenderWindow& window) {
    static bool f11Pressed = false;
    if (event.type == sf::Event::Closed) window.close();

    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::F11 && !f11Pressed) {
            f11Pressed = true;
            if (window.getSize().x == WINDOW_WIDTH) {
                window.create(sf::VideoMode::getDesktopMode(), "Flappy Bird", sf::Style::Fullscreen);
            } else {
                window.create(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Flappy Bird", sf::Style::Close);
                window.setPosition(sf::Vector2i((sf::VideoMode::getDesktopMode().width - WINDOW_WIDTH) / 2, 
                                                (sf::VideoMode::getDesktopMode().height - WINDOW_HEIGHT) / 2));
            }
            window.setFramerateLimit(60);
            window.setVerticalSyncEnabled(true);
        }
        if (menuActive && event.key.code == sf::Keyboard::Space) {
            menuActive = false;
            gameStarted = true;
        }
        if (event.key.code == sf::Keyboard::Space && !spacePressed && gameStarted) {
            spacePressed = true;
            velocity = JUMP;
        }
        if (event.key.code == sf::Keyboard::P) paused = !paused;
        if (gameOver) {
            if (event.key.code == sf::Keyboard::R) reset();
            if (event.key.code == sf::Keyboard::Q) window.close();
        }
    }
    if (event.type == sf::Event::KeyReleased) {
        if (event.key.code == sf::Keyboard::Space) spacePressed = false;
        if (event.key.code == sf::Keyboard::F11) f11Pressed = false;
    }
}

void FlappyBird::reset() {
    birdY = WINDOW_HEIGHT / 2;
    velocity = 0;
    if (score > highScore) highScore = score;
    score = 0;
    lives = 1;
    gameOver = false;
    gameStarted = false;
    menuActive = true;
    invincible = false;
    birdFrame = 0;
    birdAnimationTimer = 0.0f;
    resetPipes();
    particles.clear();
    clouds.clear();
    pipesPassed = 0;
}

void FlappyBird::resetPipes() {
    for (int i = 0; i < PIPE_COUNT; i++) {
        pipes[i].x = WINDOW_WIDTH + (i * (WINDOW_WIDTH / PIPE_COUNT));
        pipes[i].gapY = rand() % (PIPE_MAX_GAP_Y - PIPE_MIN_GAP_Y + 1) + PIPE_MIN_GAP_Y;
        pipes[i].scored = false;
    }
}

int main() {
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Flappy Bird: Majestic", sf::Style::Close);
    window.setFramerateLimit(60);
    window.setVerticalSyncEnabled(true);
    window.setPosition(sf::Vector2i((sf::VideoMode::getDesktopMode().width - WINDOW_WIDTH) / 2, 
                                    (sf::VideoMode::getDesktopMode().height - WINDOW_HEIGHT) / 2));
    FlappyBird game;
    sf::Clock clock;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            game.handleInput(event, window);
        }
        float deltaTime = clock.restart().asSeconds();
        game.update(deltaTime);
        game.draw(window);
    }
    return 0;
}