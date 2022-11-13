* Repo to build Cbc/SYMPHONY solver libraries using github workflow
* Currently built platforms : manylinux_2_24_x86_64 and manylinux2014_x86_64
* This repo helps reduce build time of [ALIGN](https://github.com/ALIGN-analoglayout/ALIGN-public).
* Build instruction:
```
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX:PATH=<prefix dir> ..
make -j4
make install
```
