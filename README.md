# Software Ray Tracing

A lightweight, CPU-based software ray tracer implemented in modern C++, featuring the use of **C++20 Modules** for seamless library integration. 

## Features

- **Pure CPU Rendering**: Implementation of core ray tracing concepts (ray generation, ray-sphere/ray-triangle intersection, and simple shading).
- **C++20 Modules Integration**: Traditional headers are wrapped into modern C++ modules (`.cppm`) including `glm`, `minifb`, and `tinyobjloader` to achieve cleaner interfaces and faster compile times.
- **Declarative Dependency Management**: Built-in support for `vcpkg` in manifest mode.
- **Modern VS Solution Format**: Uses the lightweight Visual Studio Solution (`.slnx`) structure.

## Tech Stack

- **Language Standard**: C++20 / C++23
- **Mathematics**: [GLM (OpenGL Mathematics)](https://github.com/g-truc/glm) — via `glm.cppm`
- **Window & Framebuffer**: [MiniFB](https://github.com/emoon/minifb) — via `minifb.cppm`
- **3D Model Loader**: [tinyobjloader](https://github.com/tinyobj/tinyobjloader) — via `tinyobjloader.cppm`
- **Package Manager**: [vcpkg](https://github.com/microsoft/vcpkg)
