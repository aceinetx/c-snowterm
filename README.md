# c-snowterm
A rewrite of [python-snowterm](https://github.com/valignatev/python-snowterm) in ANSI C, with a fix regarding window size

## Compiling and running
### Linux (assuming ncurses, gcc and make are installed)
```console
$ make
...
$ ./c-snowterm
```
### Windows (mingw ucrt64, assuming gcc and make are installed)
```console
$ pacman -S mingw-w64-ucrt-x86_64-ncurses patch
...
$ patch makefile < mingw.patch
patching file makefile
$ make
...
$ ./c-snowterm
```
