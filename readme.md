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

System Requirement: MSVC 2019 on Windows; g++ 9 on Linux

`git clone --recursive` this repository. There is a vcpkg submodoule.

```bash
sudo apt-get update && sudo apt-get install -y tar unzip curl git g++ cmake ninja-build
cd vcpkg
./bootstrap-vcpkg.sh
./vcpkg install libarchive[core] jsoncpp
cd ..
mkdir -p out/build
cd out/build
cmake ../.. -G Ninja
cmake --build .
```
