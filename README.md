# Scientific Calculator

A graphical scientific calculator written in C++ with Qt 6. You can click the buttons or type expressions directly on the keyboard.

## Features

- Basic arithmetic: `+`, `-`, `*`, `/`, `^`, parentheses
- Scientific functions: `sin`, `cos`, `tan`, `asin`, `acos`, `atan`, `ln`, `log`, `exp`, `sqrt`, `abs`
- Constants: `pi`, `e`, and `Ans` (the previous result)
- Factorial (`!`) and percent (`%`)
- DEG/RAD toggle for trigonometric functions
- Implicit multiplication, such as `2pi` or `3(4+5)`
- Clear error messages, such as division by zero or a missing `)`

## Examples

| Input | Result |
|---|---|
| `2+3*4` | 14 |
| `sin(30)` | 0.5 (in DEG mode) |
| `sqrt(16)+1` | 5 |
| `5!` | 120 |
| `2^3^2` | 512 |

## Requirements

- C++17 compiler (g++ or clang++)
- CMake 3.16 or newer
- Qt 6 (Widgets module)

Install on Ubuntu/Debian:

```bash
sudo apt update
sudo apt install qt6-base-dev cmake g++
```

On macOS, use `brew install qt cmake`. On Windows, install Qt 6 from the Qt Online Installer, along with CMake.

## Build and Run

```bash
cmake -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build
./build/calculator
```

## Keyboard Shortcuts

- **Enter** or **=**: evaluate
- **Backspace**: delete one character
- **Esc**: clear everything

## Project Structure
