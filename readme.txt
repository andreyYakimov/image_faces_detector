# Image Faces Detector

C++ application for face detection in images using OpenCV.

The project uses CMake for building and vcpkg for dependency management.

---

## Requirements

### Windows

- Windows 10/11
- Visual Studio 2022
- CMake
- vcpkg

### Linux / WSL

- GCC or Clang
- CMake
- Ninja
- vcpkg

---

## Dependencies

Install the required dependencies:

```bash
vcpkg install
```

---

## Build

### Windows

Configure, build and install:

```powershell
cmake -S . `
  -B C:\dev\detector_build `
  -G "Visual Studio 17 2022" `
  -A x64 `
  -T v143 `
  -DCMAKE_TOOLCHAIN_FILE=C:\dev\vcpkg\scripts\buildsystems\vcpkg.cmake `
  -DVCPKG_INSTALLED_DIR=C:\dev\vcpkg_installed

cmake --build C:\dev\detector_build --config Release

cmake --install C:\dev\detector_build `
  --config Release `
  --prefix dist
```

### Linux

Set `VCPKG_ROOT` to your vcpkg installation:

```bash
export VCPKG_ROOT=/path/to/vcpkg
```

Install dependencies:

```bash
vcpkg install
```

Configure, build and install:

```bash
cmake \
  -S /mnt/c/dev/image_faces_detector/src \
  -B ~/detector/image_faces_detector/build-linux \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake

cmake --build ~/detector/image_faces_detector/build-linux

cmake --install ~/detector/image_faces_detector/build-linux --prefix dist
```

---

## Usage

Run the application with:

```text
detector
  --images_path <path>
  --result_images_path <path>
  [--cv_threads <number>]
  [--workers <number>]
  [--logging <true|false>]
```

### Command-line options

| Option | Required | Default | Description |
|---|:---:|---|---|
| `--images_path` | Yes | — | Directory containing input images |
| `--result_images_path` | Yes | — | Directory for processed images |
| `--cv_threads` | No | CPU-dependent | Number of OpenCV threads |
| `--workers` | No | CPU count | Number of detection workers |
| `--logging` | No | `false` | Enable console logging |

Both `images_path` and `result_images_path` must exist and be directories.

---

## Examples

### Linux

```bash
./detector \
  --images_path ./images \
  --result_images_path ./results \
  --cv_threads 2 \
  --workers 8 \
  --logging true
```

### Windows

```powershell
detector.exe `
  --images_path C:\data\images `
  --result_images_path C:\data\results `
  --cv_threads 2 `
  --workers 8 `
  --logging true
```

---

## Default Configuration

- `workers` defaults to the number of available CPU threads.
- `cv_threads` defaults to `2` when at least 8 CPU threads are available.
- `cv_threads` defaults to `1` otherwise.
- Logging is disabled by default.
- `workers` and `cv_threads` must be greater than `0`.

---

## Input and Output

Input images are read from `--images_path`.

Processed images are written to `--result_images_path`.

Example:

```text
images/
├── image_001.jpg
├── image_002.jpg
└── image_003.jpg

results/
├── image_001.jpg
├── image_002.jpg
└── image_003.jpg
```

---

## Graceful Shutdown

The application handles `SIGINT` and `SIGTERM`.

Press `Ctrl+C` to request a graceful shutdown.

---

## Error Handling

Invalid paths and command-line arguments are reported in the console.

Example:

```text
Error: images_path is not a valid directory
```

```text
Error: result_images_path is not a valid directory
```

```text
Error: workers should be at least 1
```

```text
Error: cv_threads should be at least 1
```
