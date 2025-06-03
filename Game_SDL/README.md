Flappy Bird: 

Overview

    This is a C++ implementation of the classic Flappy Bird game using the SDL2 (Simple DirectMedia Layer) library with SDL_ttf for text rendering. The game features a bird navigating through pipes with enhanced visuals like a gradient sky, animated clouds, a glowing sun, and particle effects for scoring and collisions. It includes a dynamic difficulty system, score tracking, and a high score system.
    Features

Gameplay Mechanics:

    Control the bird with the Space key to jump.
    Navigate through randomly positioned pipe gaps.
    Score points by passing pipes.
    Dynamic pipe speed increases with score (capped at 8.0f).
    Game over on collision with pipes or ground.


Visuals:

    Gradient sky background for depth.
    Animated bird with three-frame wing flapping (up, mid, down), beak, and eye details.
    Moving clouds with sinusoidal motion for a lively background.
    Sun with halo effect using layered rectangles.
    Particle effects: yellow stars for scoring, gray smoke for collisions.
    Detailed ground with grass, dirt, and pink flower decorations.
    Pipes with texture lines, caps, and shadow effects.


UI:

    Displays score and high score with shadowed text.
    Start screen with instructions ("Press Space to Start").
    Game over screen with restart (R) or quit (Q) options.
    Pause functionality using the P key.
    Smooth transitions for game over and start screens.


Dynamic Difficulty:

    Pipe speed scales with score for increased challenge.
    Randomized pipe gap positions for varied gameplay.



Prerequisites

    SDL2 Library: Install SDL2 and SDL2_ttf.
    On Ubuntu: sudo apt-get install libsdl2-dev libsdl2-ttf-dev
    On macOS: brew install sdl2 sdl2_ttf
    On Windows: Follow SDL2 setup instructions for your IDE (e.g., Visual Studio).


C++ Compiler: A compiler supporting C++11 or later (e.g., g++).
Font: Uses DejaVuSans.ttf, expected at /usr/share/fonts/truetype/dejavu/DejaVuSans.ttf. Update the path in initGame if necessary or provide an alternative font.

Compilation and Running

    Install SDL2 and SDL2_ttf:
    Ensure SDL2 and SDL2_ttf are installed and configured.


Compile the Code:make


Run the Game:./flappy_bird



Controls

    Space: Jump (starts the game from the menu).
    P: Pause/unpause the game.
    R: Restart the game (when game over, after a short delay).
    Q: Quit the game (when game over, after a short delay).

Code Structure

    Main File: Contains the FlappyBird class and main function.
    FlappyBird Class:
    Manages game state (bird position, velocity, score, pipes, etc.).
    Handles input, updates game logic, and renders graphics using SDL2.
    Includes particle system for visual effects and cloud animations.


Key Components:

    Bird: Composed of SDL_Rect objects for body, wings (three frames), beak, eye, pupil, and shadow.
    Pipes: Array of Pipe structs with position, gap, and scoring status.
    Particles: Vector of Particle structs for score and crash effects.
    UI: Text rendering with shadows using SDL_ttf for score, high score, and menus.
    Visuals: Gradient sky, animated clouds, sun with halo, and detailed ground.



Known Issues

    Font Path: Hardcoded font path may not work on all systems. Update the TTF_OpenFont path in initGame if needed.
    Collision Detection: Bird hitbox may feel slightly large; adjust BIRD_X ± 18 and birdY ± 12 in checkCollisions for precision.
    Performance: High particle counts may impact performance on low-end systems.
    Fullscreen: No fullscreen toggle; window is fixed at 1000x700.

Future Improvements

    Add sound effects for jumps, collisions, and scoring.
    Implement high score persistence using file I/O.
    Add more particle effects or animations (e.g., dynamic cloud shapes).
    Introduce power-ups or additional obstacles.
    Support customizable controls or window resizing.
    Optimize rendering for better performance on low-end devices.


