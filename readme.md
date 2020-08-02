docker-image-viewer
===================

Docker Image Viewer

Usage:
```bash
docker save your-image > image.tar
./docker-image-viewer image.tar
```


Build
--------


`git clone --recursive` this repository. There is a `vcpkg` submodoule.


### Windows with Visual Studio

System Requirement: MSVC 2019
- Desktop development with C++
- MSVC v142 - VS 2019 C++ x64/x86 build tools
- Windows 10 SDK
- C++ CMake tools for Windows

1. In Windows Explorer, visit `vcpkg` directory within the project directory.
2. **Right** click -> **Open in Visual Studio**.
3. Visual Studio will automatically performs CMake configure. It will download and build the libraries if not yet.
4. When configure finishes, click **Build** -> **Build All** to build the project.


## Linux

System Requirement:
- g++ 9+
- cmake 3.13+

Ubuntu
```bash
sudo apt-get update && sudo apt-get install -y tar zip unzip curl git g++ cmake ninja-build
```

OpenSUSE
```bash
zypper install -y tar gzip bzip2 xz zip unzip curl git gcc-c++ cmake ninja
```

Alpine
```bash
apk add tar zip unzip curl git g++ cmake ninja
export VCPKG_FORCE_SYSTEM_BINARIES=1
cd vcpkg
./bootstrap-vcpkg.sh -useSystemBinaries
```

Then use cmake to build
```bash
cmake -S . -B out/build -G Ninja
cmake --build out/build
```
