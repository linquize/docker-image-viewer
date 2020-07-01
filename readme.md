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


`git clone --recursive` this repository. There is a vcpkg submodoule.


### Windows with Visual Studio

System Requirement: MSVC 2019
- Desktop development with C++
- MSVC v142 - VS 2019 C++ x64/x86 build tools
- Windows 10 SDK
- C++ CMake tools for Windows

1. In Windows Explorer, visit `vcpkg` directory within the project directory.
2. Run `bootstrap-vcpkg.bat`. When build finishes, `vcpkg.exe` should be produced.
3. Go back to the project root directory, **right** click -> **Open in Visual Studio**.
4. Visual Studio will automatically performs CMake configure. It will download and build the libraries if not yet.
5. When configure finishes, click **Build** -> **Build All** to build the project.


## Linux

System Requirement: g++ 9

```bash
sudo apt-get update && sudo apt-get install -y tar unzip curl git g++ cmake ninja-build
cd vcpkg
./bootstrap-vcpkg.sh
cd ..
mkdir -p out/build
cd out/build
cmake ../.. -G Ninja
cmake --build .
```
