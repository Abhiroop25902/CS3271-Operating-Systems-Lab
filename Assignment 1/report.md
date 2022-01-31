# Operating Systems Laboratory Assignment 1

1.  Install Git
    -   `sudo apt-fast install git`
2.  Install qemu
    -   `sudo apt-fast install qemu`
3.  Clone xv6

    -   `git clone git://github.com/mit-pdos/xv6-public.git`
    -   error: The unauthenticated git protocol on port 9418 is no longer supported.
    -   Fix: changed url to https://... rather than git://...
    -   `git clone https://github.com/mit-pdos/xv6-public.git`

4.  Install compiler, linker, and so on.

    -   `sudo apt-fast install build-essential gdb`

5.  Build system xv6 for qemu

    -   [help link](https://www.assistedcoding.eu/2017/11/06/install-vx6-operating-system/)
    -   uncomment like 54
        -   `QEMU = qemu-system-x86_64`
    -   `make`
    -   `make qemu-nox` for open terminal inside the same window
        -   how to quit -> press `Ctrl+A` then press `x` ()
    -   else just do `make qemu` to open terminal in new windows
        -   close the window by just closing windows (might need to Ctrl + Alt + G)

6.  extra stuffs for fun -> make a code in the OS

    -   `cp wc.c bla.c`
    -   edit bla.c to make hello world program
    -   ```
        #include "types.h"
        #include "stat.h"
        #include "user.h"

        char buf[512];

        int main(int argc, char *argv[])
        {
            printf(1, "hello world\n");
            exit();
        }
        ```

    -   from the site -> "The simplest way is to open the makefile, search for wc and add bla next to it whenever it appears on the makefile"
    -   `make qemu-nox`
    -   `bla` (inside the terminal)
        -   hello world
