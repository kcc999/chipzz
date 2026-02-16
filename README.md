# chipzz

An interpreter for the [CHIP8 programming language](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM) written in C++ (rendering done via Raylib). It's been tested on some roms but emulation correctness is not guaranteed.

Pong running on it:


![Example running pong](.media/pong_example.png)

## Building & Running

This project uses CMake with Ninja.

Requirements:
- CMake 3.16+
- Ninja
- raylib (static or shared)

Build:

```console
foo@bar:~$ cmake -S . -B build -G Ninja
foo@bar:~$ cmake --build build
```

The build tries to find raylib in this order:
1. CMake package (`find_package(raylib CONFIG)`)
2. `pkg-config` (`raylib.pc`)
3. Manual system lookup (`/usr`, `/usr/local`, `/opt/homebrew`, `/opt/local`), preferring static library before shared.

Run with:

```console
foo@bar:~$ ./build/chipzz /path-to-rom
```
