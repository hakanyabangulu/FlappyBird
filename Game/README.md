# Flappy Bird - Console Version:
* A simple Flappy Bird clone playable directly in the console (terminal).
* Developed in C for WSL Ubuntu(Linux) environment.

### Installing library ncurses:
```
sudo apt update && sudo apt install libncurses5-dev libncursesw5-dev
```

### Debugging:
```
gcc -o flappy_bird flappy_bird.c -lncurses
```

### Running the game:
```
./flappy_bird
```

### How to play:
* Press Enter to start the game.
* Press Space to make the bird jump.
* Press P to pause and unpause the game.
* When you lose:
    * Press R to restart the game.
    * Press Q to quit the game.
