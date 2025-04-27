# Flappy Bird - OpenGL Version
This is a implementation of the classic "Flappy Bird" game using OpenGL, GLFW, GLEW, and GLM libraries. The game consists of a bird that the player controls by pressing the spacebar to make the bird "flap" and avoid obstacles (pipes). The goal is to keep the bird in the air while navigating through the pipes, earning points for each pipe the bird successfully passes.

### Needed Installations:
* Before compiling and running the game, make sure to have the necessary libraries installed on your system. The following instructions are for Linux-based systems (Ubuntu). I made and tested the game on WSL-Ubuntu.

```
sudo apt install -y build-essential cmake pkg-config
sudo apt install -y mesa-utils libglu1-mesa-dev freeglut3-dev mesa-common-dev
sudo apt install -y libglew-dev libglfw3-dev libglm-dev
```

* On terminal, by using command:
```
glxgears 
```
you can test and verify that you successfully  downloaded the OpenGL library. This will display a simple OpenGL rendering window with rotating gears. If the window appears, OpenGL is working correctly

### Debugging:
* In directory /home/.../FlappyBirdOpenGL)
```
g++ -o flappy_bird src/main.cpp -lglfw -lGLEW -lGL -lGLU -lglut
```

### Running the game:
* Once the program is compiled, you can run the game by executing the following command:
```
./flappy_bird 
```

### Game states:
* The game has three possible states:
    * START: The game is in the starting state. Press the spacebar to start playing.
    * PLAYING: The bird is flapping, and you need to avoid the pipes. The game is in progress.
    * GAME_OVER: The game has ended. You can press "R" to restart or "Q" to quit.

### How to play:
* SPACE: Make the bird flap (starts the game in the START state and continues while the game is in PLAYING state).
* R: Restart the game after a game over (in the GAME_OVER state).
* Q: Quit the game.
