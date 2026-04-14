# 📊 JCAT - Jupyter's Code Analysis Tool

> **Blazing fast** code metrics analyzer with a slick UI. Powered by Raylib. No bloat, just vibes.

![C++](https://img.shields.io/badge/C%2B%2B-14-00599C?style=flat-square&logo=c%2B%2B)
![Raylib](https://img.shields.io/badge/Raylib-5.0-green?style=flat-square)
![Platform](https://img.shields.io/badge/Platform-Windows-0078D4?style=flat-square)
![Status](https://img.shields.io/badge/Status-Active-success?style=flat-square)

---

## ✨ Features

- **🎨 Gorgeous Dark UI** - Raylib-powered GUI with smooth 60fps animations
- **📈 Deep Code Metrics**
  - Line counts (total, code, comments, blank)
  - Function detection via static analysis
  - Comment density ratio
  - File size in KB
  - Code density (LOC/KB compression ratio)
- **⚡ Butter-Smooth Animations** - Easing animations for all metrics (0.6s duration)
- **🔄 Multi-Language Support** - C++ and Java analyzers
- **📋 Clipboard Integration** - Paste file paths with `Ctrl+V`

---

## 🚀 Quick Start

### Prerequisites

```prolog
OS:           Windows 10+ (or WSL2 on Linux)
Raylib:       >= 5.0
Compiler:     mingw32-make (w64devkit)
IDE:          VS Code (recommended)
C++ Standard: C++14
```

### Setup

```bash
# Clone and open in VS Code
git clone <repo>
cd raylibTest
code main.code-workspace

# Hit F5 to build & run
# Or use Ctrl+Shift+B for manual build tasks
```

---

## 🎮 Usage

1. **Pick Your Poison** → Select C++ or Java from the dropdown
2. **Drop the Path** → Paste your file path (supports full URIs)
3. **Smash Analyze** → Watch the magic happen
4. **Admire the Numbers** → Smooth counter animations + visual bars

### Example Paths

```
C:\dev\projects\main.cpp              # Windows absolute
D:\code\Algorithm.java                # Different drive
\\server\share\source.cpp             # Network paths work too
```

---

## 🔧 Build & Development

### Debug Mode (with symbols & no optimization)
```bash
mingw32-make RAYLIB_PATH=C:/raylib/raylib PROJECT_NAME=main OBJS=src/*.cpp BUILD_MODE=DEBUG
```

### Release Mode (optimized)
```bash
mingw32-make RAYLIB_PATH=C:/raylib/raylib PROJECT_NAME=main OBJS=src/*.cpp
```

### Project Structure
```
raylibTest/
├── main.code-workspace       # VS Code workspace config
├── Makefile                  # Build rules (MinGW compatible)
├── README.md                 # This file
├── lib/                      # External libraries
└── src/
    ├── main.cpp              # 480+ lines of pure C++14 goodness
    └── raygui.h              # RayGUI header-only library
```

---

## 🛠️ Tech Stack

| Layer | Tech |
|-------|------|
| **Graphics** | Raylib 5.0 (OpenGL backend) |
| **GUI** | RayGUI (immediate-mode) |
| **Analysis** | Custom handwritten lexer |
| **Language** | C++14 with STL |
| **Build** | MinGW GCC + Makefile |
| **Target** | PLATFORM_DESKTOP (Windows) |

---

## 📊 How It Works

```cpp
// Simplified flow:
1. Read file → std::ifstream
2. Scan lines → regex-free FSM-based parsing
3. Count metrics → functions, comments, blanks
4. Calculate ratios → density, comment %
5. Animate → easing function over 0.6s
6. Render → 60fps Raylib draw calls
```

**Static Analysis Features:**
- Block comment tracking (`/* */`)
- Line comment detection (`//`)
- Function heuristic: lines with `(` and ending with `{` or `)`
- Control flow exclusion (if/while/for detection)

---

## 📈 Roadmap

- [ ] Real-time file watching
- [ ] Export metrics as JSON
- [ ] Cyclomatic complexity calculation
- [ ] Function call graph visualization
- [ ] Dark/Light theme toggle
- [ ] Python & Rust language support

---

## 📝 Changelog

| Version | Highlight |
|---------|-----------|
| **V0.6** | Code density & file size metrics debut 🎉 |
| **V0.5** | Smooth counting animations (easing curves) |
| **V0.4** | Core metrics + UI framework |
| **V0.1** | First working prototype |

---

## 🎯 Design Philosophy

- **Zero Dependencies** (except Raylib) - We control the stack
- **Single-Pass Analysis** - Read file once, parse once
- **Immediate-Mode Rendering** - Raylib's approach, super responsive
- **No External Tools** - Pure C++ static analysis
- **Keyboard Shortcuts** - Power users welcome (Ctrl+V for paste)

---

## 💡 Cool Implementation Details

- **Custom MetricAnimation struct** with easing support
- **State machine** for UI (IDLE → ANALYZING → DONE/ERROR)
- **Dynamic card layout** - 2-column grid that auto-flows
- **Blinking cursor** in input field using `GetTime() % 2`
- **Clipboard support** via `GetClipboardText()` (Raylib built-in)
- **Error handling** with user-friendly messages

---

## 🤝 Contributing

Found a bug? Want a feature? Open an issue or PR!

```bash
git checkout -b feature/amazing-thing
# Make changes...
git commit -m "feat: add amazing thing"
git push origin feature/amazing-thing
```

---

## 📄 License

Built with ❤️ in 2026

**JCAT** = Jupyter's Code Analysis Tool  
*Because sometimes you just need to count those lines.*

---

<div align="center">

**Made with Raylib • Tested on Windows 10/11 • Open source** ✨

</div>

