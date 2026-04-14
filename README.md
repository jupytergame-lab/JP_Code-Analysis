# JCAT - Jupyter's Code Analysis Tool

Simple static code analizer. Supports C++ and Java with animated metric display and basic, simple UI.

**Platform**: Windows 10+ | **Language**: C++14 | **Graphics**: Raylib 5.0

## Features

- **Code Metrics**: Code density (loc/kb), Line counts, function counts, comment to code ratio
- **Animated Display**: Smooth animations for all metrics
- **Multi-Language**: C++ and Java analysis support
- **Dark UI**: Modern Raylib-based interface with 60fps rendering
- **Clipboard Support**: Direct path input with Ctrl+V

## Quick Start

### Requirements

- Windows 10/11
- Raylib 5.0+
- MinGW GCC (w64devkit)
- Visual Studio Code

### Build

```bash
code main.code-workspace
# Press F5 or Ctrl+Shift+B → "build debug"
```

### Usage

1. Select language (C++ or Java)
2. Enter file path
3. Click Analyze
4. View results

## Build Commands

**Debug:**
```bash
mingw32-make RAYLIB_PATH=C:/raylib/raylib PROJECT_NAME=main OBJS=src/*.cpp BUILD_MODE=DEBUG
```

**Release:**
```bash
mingw32-make RAYLIB_PATH=C:/raylib/raylib PROJECT_NAME=main OBJS=src/*.cpp
```

## Technical Stack

| Component | Technology |
|---|---|
| Graphics | Raylib 5.0 (OpenGL) |
| GUI | RayGUI (immediate-mode) |
| Analysis | FSM-based lexer |
| Build | MinGW Makefile |

## Versioning

| Version | Changes |
|---|---|
| V0.6 | File size and code density metrics |
| V0.5 | QoL changes |
| V0.4 | Core framework |

## Author

Created by Jupyter, 2026