# 🧱 3D Maze Navigation Game

A 3D first-person maze navigation game created using Modern OpenGL and C/C++ in Xcode (macOS). Use keyboard controls to navigate the maze, and reach the red goal cube as fast as possible while a timer tracks your progress.

---

## 🎮 Features

- First-person camera movement with smooth controls
- Simple 3D maze layout using a grid system
- Timer display using GLUT fonts
- Basic lighting and shading effects
- Red goal cube marks the end of the maze

---

## 🛠️ Built With

- **Xcode** (macOS)
- **OpenGL** (`<OpenGL/gl.h>`)
- **GLUT** (`<GLUT/glut.h>`)
- **GLFW** (`<GLFW/glfw3.h>`)
- **C / C++**

---

## 🔧 Controls

| Key | Action         |
|-----|----------------|
| `W` | Move Forward   |
| `S` | Move Backward  |
| `A` | Rotate Left    |
| `D` | Rotate Right   |

---

## 🚀 How to Run

1. Clone this repository or copy the files into a new Xcode project.
2. Link the following frameworks:
   - OpenGL.framework
   - GLUT.framework
3. Install GLFW if needed:
   ```bash
   brew install glfw

