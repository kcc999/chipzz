# chipzz

An interpreter for the [CHIP8 programming language](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM) written in C++ (rendering done via Raylib). It's been tested on some roms but emulation correctness is not guaranteed.

Pong running on it:


![Example running pong](.media/pong_example.png)

## Building & Running

The repository contains a build.bat script for building on Windows (MSVC), you need to have raylib includes+lib file present on the same directory.

For building on UNIX systems through gcc or clang, it should be as straight forward as linking with raylib.

Run with:

```console
foo@bar:~$ chipzz.exe /path-to-rom
```


