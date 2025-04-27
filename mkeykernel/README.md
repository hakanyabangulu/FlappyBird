Flappy Bird at mkeykernel

Text-Based Graphics: Renders bird, pipes, ground, and sky in VGA text mode.
Controls:

     X: Start/restart game.
     Space: Flap bird.
     Esc: Pause/resume.
     Q: Exit (shuts down QEMU in Game Over screen).


Mechanics: Bird navigates through moving pipes, scores points, tracks high score.
Exit: QEMU shuts down via hlt on exit.

Requirements

    GCC, NASM, QEMU, Make
    Linux/Unix-like system (or WSL on Windows)

Setup

    Clone repo:git clone https://github.com/hakanyabangulu/FlappyBird/mkeykernel

Install dependencies (Ubuntu/Debian):

    sudo apt update
    sudo apt install gcc nasm qemu-system-x86 make


Build:

    make
    Run in QEMU:qemu-system-i386 -kernel kernel.bin



Usage

    Press X to start/restart.
    Use Space to flap, avoid pipes.
    Esc to pause/resume.
    In Game Over, press Q to exit (closes QEMU).

Notes

    Code: kernel.c (game logic, VGA drawing), kernel.asm (interrupts), keyboard_map.h (key mappings).
    Customization: Adjust #define constants in kernel.c for game parameters.
    Shutdown: Uses hlt for QEMU exit.
    Limitations: 80x25 text mode, no advanced shutdown (e.g., ACPI).

Repository

    The source code is available at:
    git clone https://github.com/arjun024/mkeykernel.git

Debugging

    Ensure kernel.bin is generated (make).
    Check QEMU logs if it fails to boot.

