# libtre
libtre is a personal C++ library for game engine fundamentals.

## Important notice ##

If you plan to use this library for any reason, it is important to stress that it is currently in unstable development and the API may change on a whim until I deem it stable. There may also be bugs I haven't fixed yet.

## Documentation ##

Documentation can be built with Doxygen, or can be viewed [here](https://trdario.github.io/libtre/).

## Examples ##

TO-DO.

## Dependencies ##
libtre depends on the following external libraries:
- [libtr](https://github.com/TRDario/libtr)

libtref depends on the following external libraries:
- [lz4](https://github.com/lz4/lz4)

trefc depends on the following external libraries:
- [qoi](https://github.com/phoboslab/qoi) (vendored)
- [stb_image](https://github.com/nothings/stb) (vendored)

## Building ##
The following is required to build libtre:
- A C++20 compiler.
- CMake 3.24.0 or higher.
- glslang
- xxd

libtre can be easily integrated into a project using CMake FetchContent:
```
include(FetchContent)
FetchContent_Declare(tre GIT_REPOSITORY https://github.com/TRDario/libtre.git GIT_TAG main FIND_PACKAGE_ARGS NAMES tre)
FetchContent_MakeAvailable(tre)
```