# lwlaim

**lwlaim** is a minimal, lightweight aim training game built for competitive gamers, focusing on low file size and high performance. The game uses **GLFW**, **OpenGL**, **STB Image**, and **STB TrueType** for rendering and font handling, providing a fast and responsive environment for improving your aim. The entire project is designed with efficiency in mind, making it easy to run even on low-spec machines.

## Features

- **Minimal File Size**: Everything in lwlaim is optimized for the smallest possible size while maintaining performance. The executable and all assets are as compact as possible, making it ideal for quick access and low overhead.
- **OpenGL Rendering**: Utilizes **OpenGL 4.6** for rendering, allowing for fast and efficient graphical output.
- **GLFW for Window Management**: Uses **GLFW** for cross-platform window creation and input handling, ensuring portability and minimal dependencies.
- **STB Image for Texture Loading**: **STB Image** is used for loading image files (e.g., textures) with a focus on minimal dependency and compact size.
- **STB TrueType for Fonts**: Uses **STB TrueType** for rendering fonts with an emphasis on simplicity and size optimization.
- **Aim Training**: The main focus of the game is to help players train their aim by providing a straightforward environment for shooting practice.

## Requirements

- **GLFW**: A library for creating windows with OpenGL contexts and managing input.
- **OpenGL 4.6**: The graphics API used for rendering.
- **STB Image**: Used to load image files for textures.
- **STB TrueType**: For loading and rendering fonts from TrueType font files.

### Dependencies

Make sure you have the following libraries available for the build:

- **GLFW** - [https://www.glfw.org/](https://www.glfw.org/)
- **GLEW** - [https://glew.sourceforge.net/](https://glew.sourceforge.net/)
- **STB Image** - [https://github.com/nothings/stb](https://github.com/nothings/stb)
- **STB TrueType** - [https://github.com/nothings/stb](https://github.com/nothings/stb)

## Building

1. Clone the repository

2. Install bun or any similarities and use `bun b` for any os, `bun bw` for windows (helps with hiding the console on release build).
	- Another option is to just copy the commands from the `packages.json` and slamming them in the terminal.

3. Run the game:
	```bash
	./dest/lwlaim
	```
	or
	```bash
	./dest/lwlaim.exe
	```

## Controls

- **Left Click**: Shoot at the targets.
- **G**: Change game mode.
- **F**: Change game mode's settings.
- **V**: Change the game's settings (targets outline/base color).
- **Escape**: Quit the game.

## Performance

- lwlaim is optimized for **low-resource usage**, meaning it runs efficiently even on older or low-spec machines.
- The game is lightweight, with a primary focus on **minimal disk usage** and **fast runtime performance**.

## Contributing

Contributions are welcome! If you have suggestions for optimization or new features, feel free to submit a pull request or open an issue.