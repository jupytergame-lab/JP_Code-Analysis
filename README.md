# JP Code Analysis Tool

A visual code analysis application built with Raylib that provides detailed metrics about your source code files. Analyze C++ and Java files to understand code structure, quality, and composition.

## Features

- **Clean UI** - Clean, dark themed UI with animations
- **Code Metrics** - Analysis including:
  - Total lines, code lines, comment lines, blank lines
  - Function count
  - Comment ratio (%)
  - File size (KB)
  - Code density (lines of code per KB)
- **Animations** - Metric values have a smooth animation after analysis
- **Language Selection** - Choose between C++ and Java analysis
- **File Path Input** - Enter file paths directly with clipboard support (Ctrl+V)

## Requirements

- Windows 10 / 11 (OS doesn't really matter if the requirements below are supported)
- Raylib 5.0+
- MingW Compiler
- Visual Studio Code

## Setup & Build

1. Open `main.code-workspace` in VS Code
2. Press `F5` to compile and run, or use the build task:
   - **Debug Build**: `Ctrl+Shift+B` → select "build debug"
   - **Release Build**: `Ctrl+Shift+B` → select "build release"

## Usage

1. **Select Language** - Click the dropdown and choose C++ or Java
2. **Enter File Path** - Type or paste the full path to your file
3. **Click Analyze** - The tool will scan the file and display some metrics about basic information
4. **View Results** - The metrics will pop up after a quick second of calculation with some smooth animations.

### Example Paths
- Windows: `C:\Users\YourName\Projects\main.cpp`
- WSL: `/home/user/project/file.java`


## Version History

- **V0.6** - Implemented new metrics for code density per KB & added file size metrics
- **V0.5** - Added file size and code density metrics & implemented typewriter effect
- **V0.4** - Initial animated metrics implementation
- Earlier - Base code analysis structure

## Build Commands

```bash
# Debug build (with debug symbols and no optimizations)
mingw32-make RAYLIB_PATH=C:/raylib/raylib PROJECT_NAME=main OBJS=src/*.cpp BUILD_MODE=DEBUG

# Release build (optimized)
mingw32-make RAYLIB_PATH=C:/raylib/raylib PROJECT_NAME=main OBJS=src/*.cpp
```

## Technical Details

- **Language**: C++17
- **Graphics Library**: Raylib 5.0
- **GUI Framework**: RayGUI
- **Platform**: Windows (PLATFORM_DESKTOP)
- **Target FPS**: 60

## Author

Created by JP (Jupyter), 2026

