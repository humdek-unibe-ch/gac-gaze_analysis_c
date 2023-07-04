compilation on Windows

- use msys2.exe
- update with `pacman -Syyu`

install cmake gcc python
- `pacman -Sy mingw-w64-x86_64-cmake`
- `pacman -Sy mingw-w64-x86_64-gcc`
- `pacman -Sy mingw-w64-x86_64-python`

install syslog
- `pacman -Sy msys2-runtime-devel`

Things that are not supported on windows
- `sys/syscall.h`
- `pthread priority inheritence`
