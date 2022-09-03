# nesdoug tutorial series for llvm-mos

This is a port of the nesdoug tutorial series for developing NES games in C to
the llvm-mos toolchain. Each subdirectory contains the corresponding chapter of
the tutorial.

# Building

* Install [CMake](cmake.org) and some kind of build system (e.g.,
  [Ninja](https://github.com/ninja-build/ninja/wiki/Pre-built-Ninja-packages),
  Unix make, Windows NMake, Visual Studio/MSBuild).
* Download the latest
  [llvm-mos SDK release](https://github.com/llvm-mos/llvm-mos-sdk/releases).
* Make and open a terminal in a `build` directory under one of the chapters.
* Run `cmake -DCMAKE_PREFIX_PATH=<llvm-mos dir> -G[BuildSystem] ..`.
  * `<llvm-mos dir>` is the absolute path to the llvm-mos SDK.
  * The `-G` option gives the build system to use;
    see
    [cmake-generators](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html).
    * `-GNinja` for Ninja
    * `-G"Unix Makefiles"` for Unix makefiles
    * `-G"NMake Makefiles"` for Windows NMake makefiles
    * `-G"Visual Studio <version>"` for Visual Studio/MSBuild
    * Nothing for the system default.
* Run `ninja`, `make`, etc., depending on your chosen build system.


