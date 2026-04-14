# JP Code Analysis Tool

A visual code analysis application built with Raylib that provides detailed metrics about your source code files. Analyze C++ and Java files to understand code structure, quality, and composition.

## Features

- **Interactive GUI** - Clean, dark-themed interface with smooth animations
- **Code Metrics** - Comprehensive analysis including:
  - Total lines, code lines, comment lines, blank lines
  - Function count detection
  - Comment ratio (%)
  - File size (KB)
  - Code density (lines of code per KB)
- **Smooth Animations** - Metric values count up smoothly when analysis completes
- **Language Selection** - Choose between C++ and Java analysis
- **Real-time Feedback** - Live status updates (analyzing, done, error states)
- **File Path Input** - Enter file paths directly with clipboard support (Ctrl+V)

## Requirements

- Windows 10 or Windows 11
- Raylib 5.0+
- w64devkit compiler (MinGW)
- Visual Studio Code

## Setup & Build

1. Open `main.code-workspace` in VS Code
2. Press `F5` to compile and run, or use the build task:
   - **Debug Build**: `Ctrl+Shift+B` → select "build debug"
   - **Release Build**: `Ctrl+Shift+B` → select "build release"

## Usage

1. **Select Language** - Click the dropdown and choose C++ or Java
2. **Enter File Path** - Type the full path to your source file
3. **Click Analyze** - The tool will scan the file and display metrics
4. **View Results** - Watch metrics animate and fade in with visual feedback

### Example Paths
- Windows: `C:\Users\YourName\Projects\main.cpp`
- WSL: `/home/user/project/file.java`

## Project Structure

```
raylibTest/
├── main.code-workspace    # Workspace configuration
├── Makefile              # Build configuration
├── README.md             # This file
├── lib/                  # Library files
└── src/
    ├── main.cpp          # Main application code
    └── raygui.h          # Ray-GUI header
```

## Version History

- **V0.5** - Added file size and code density metrics, smooth animations
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

- **Language**: C++14
- **Graphics Library**: Raylib 5.0
- **GUI Framework**: RayGUI
- **Platform**: Windows (PLATFORM_DESKTOP)
- **Target FPS**: 60

## Author

Created by JP (Jupyter), 2026

