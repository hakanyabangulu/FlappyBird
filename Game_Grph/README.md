Flappy Bird:

Overview

    This is a C++ implementation of the classic Flappy Bird game using the SFML (Simple and Fast Multimedia Library) framework. The game features a bird navigating through a series of pipes, with added visual enhancements like clouds, a sun with rays, and particle effects. The game includes a scoring system, lives, invincibility, and a dynamic difficulty that increases pipe speed as the score rises.
    Features

Gameplay Mechanics:

    Control the bird using the Space key to jump.
    Avoid pipes and the ground to survive.
    Earn points by passing pipes.
    Lives system: Start with 2 lives, lose one on collision, game over when no lives remain.
    Invincibility mode after losing a life (temporary).
    

Visuals:

    Gradient sky background.
    Animated bird with flapping wings, beak, and eye details.
    Moving clouds for a dynamic background.
    Sun with evenly spaced rays for aesthetic enhancement.
    Particle effects on collisions.
    Ground with dirt layer.


UI:

    Displays current score, high score, and remaining lives.
    Menu screen with instructions to start (Space).
    Game over screen with options to restart (R) or quit (Q).
    Pause functionality using the P key.
    Fullscreen toggle with F11.


Dynamic Difficulty:

    Pipe speed increases with score (capped at 7.0f).
    Randomly positioned pipe gaps for varied gameplay.



Prerequisites

    SFML Library: Ensure SFML is installed and configured in your development environment.
    C++ Compiler: A C++ compiler supporting C++11 or later (e.g., g++).
    Font: The game uses DejaVuSans.ttf, expected at /usr/share/fonts/truetype/dejavu/DejaVuSans.ttf. Adjust the path in the code if necessary or provide an alternative font.

Compilation and Running

Install SFML:

    On Ubuntu: sudo apt-get install libsfml-dev
    On macOS: brew install sfml
    On Windows: Follow SFML setup instructions for your IDE (e.g., Visual Studio).


Compile the Code: make


Run the Game:./flappy_bird



Controls

    Space: Jump (start the game from the menu).
    P: Pause/unpause the game.
    R: Restart the game (when game over).
    Q: Quit the game (when game over).
    F11: Toggle fullscreen mode.

Code Structure

    Main File: Contains the FlappyBird class and main function.
    FlappyBird Class:
    Manages game state (bird position, velocity, score, lives, etc.).
    Handles input, updates game logic, and renders graphics.
    Includes private helper functions for pipe management, collision detection, and cloud updates.


Key Components:

    Bird: Composed of sf::RectangleShape (body), sf::ConvexShape (wings, beak), and sf::CircleShape (eye, pupil) with animation.
    Pipes: Array of Pipe structs with position, gap, and scoring status.
    Clouds: Vector of Cloud structs for background animation.
    UI: Uses sf::Text for score, high score, lives, and menu displays.
    Visuals: Gradient background, sun with rays, and particles for effects.



Known Issues

    Font Path: Hardcoded font path may not work on all systems. Update the font.loadFromFile path in initGame if needed.
    Collision Detection: May occasionally feel slightly off due to the bird’s hitbox size; adjust hitbox parameters for finer tuning.
    Performance: High particle counts or fullscreen mode may impact performance on low-end systems.

Future Improvements

    Add sound effects for jumps, collisions, and scoring.
    Implement a high score save system using file I/O.
    Add more visual effects (e.g., animated clouds, varying pipe colors).
    Introduce power-ups for varied gameplay.
    Support customizable key bindings.



