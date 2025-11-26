# Noirify

Desktop app that showcases grayscale conversion across different processing backends (C++, placeholder ASM, Python). Drag an image into the window or open it via the menu, then run all processors to compare results and timings side by side.

## Features
- **Drag-and-drop or menu file open** for common image formats.
- **Side-by-side preview** of the original image and the processed grayscale output with smooth scaling.
- **Processor timing table** that records elapsed time and notes for each backend (C++, ASM placeholder, Python script).
- **Selectable result source** to view C++, ASM, Python, or automatically pick the fastest completed processor.
- **Qt-styled UI** with an animated spinner while processors run.

## Requirements
- CMake 3.26+ and a C++23-capable compiler.
- Qt 6 with `Core`, `Gui`, and `Widgets` components available to CMake.
- Python 3 with `numpy` and `Pillow` installed.

## Building
```bash
cmake -S . -B build
cmake --build build
```
If Qt is not discovered automatically, add `-DCMAKE_PREFIX_PATH=/path/to/qt` to the first `cmake` command.

## Running
After building, launch the app from the build directory:
```bash
./build/Noirify
```

1. **Open an image** via the File → *Open Image...* menu or drop a file into the window.
2. Click the **Noirify** button (or Run → *Run All*) to execute all processors.
3. Use the **Result Source** dropdown in the menu bar to switch between C++, ASM (currently mirrors C++), Python output, or the fastest result.

Processed outputs and timing notes are shown in the table beneath the previews. The Python processor writes its temporary output using a temp directory; if unavailable or dependencies are missing, a descriptive note appears in the table.

## Sample images
A handful of example photos live in `sample_photos/` (grouped by format) so you can quickly try the workflow.

## Project structure
- `src/` - Qt application entry point and main window/UI logic.
- `processors/cpp/` - C++ grayscale implementation.
- `processors/python/` - Python grayscale script invoked from the app.
- `processors/asm/` - Placeholder directory for a future assembly implementation.
- `resources/` - Application icon and stylesheet bundled via Qt resource system.
- `sample_photos/` - Example input images for testing.

## Status
The ASM part is currently a placeholder. The Python processor runs if Python 3, NumPy, and Pillow are available; otherwise the app reports why it could not execute.