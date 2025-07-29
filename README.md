# TetrixGame

A Tetris game built with C++ and Qt.

## Prerequisites

- **CMake**: Version 3.10 or higher
- **Qt5**: Widgets module
- **C++ Compiler**: Supporting C++17 (e.g., g++, clang, MSVC)
- **Operating System**: Linux (Ubuntu), Windows, or macOS

### Ubuntu
```bash
sudo apt update
sudo apt install build-essential cmake qt5-default
```

### Windows
1. Install [Qt](https://www.qt.io/download) with the Widgets module.
2. Install [CMake](https://cmake.org/download/).
3. Install a C++ compiler (e.g., MSVC via Visual Studio or MinGW).

### macOS
```bash
brew install cmake qt
```

## Building the Game

1. Clone the repository:
   ```bash
   git clone https://github.com/ian-codding-ii/TetrixGame.git
   cd TetrixGame
   ```
2. Create a build directory:
   ```bash
   mkdir build && cd build
   ```
3. Run CMake:
   ```bash
   cmake ..
   ```
4. Build the project:
   ```bash
   make
   ```
   On Windows with MSVC, use:
   ```bash
   cmake --build .
   ```

## Running the Game

1. From the build directory, run:
   ```bash
   ./TetrixGame
   ```
   On Windows:
   ```bash
   TetrixGame.exe
   ```

## Notes

- The game uses images (`island.jpg`, `gray.jpg`) in the `images/` directory.
- Ensure Qt is in your PATH or configured in CMake.
- If resizing issues occur, check the console for debug logs.

## License

MIT License.