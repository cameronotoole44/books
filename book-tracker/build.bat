@echo off
echo Cleaning...
del /q booktracker.exe src\*.o vendor\pdcurses\pdcurses\*.o vendor\pdcurses\wincon\*.o 2>nul

echo Compiling source files...
gcc -std=c11 -O2 -Wall -Wextra -Wpedantic -Isrc -Ivendor/pdcurses -c src/commands.c -o src/commands.o
gcc -std=c11 -O2 -Wall -Wextra -Wpedantic -Isrc -Ivendor/pdcurses -c src/dates.c -o src/dates.o
gcc -std=c11 -O2 -Wall -Wextra -Wpedantic -Isrc -Ivendor/pdcurses -c src/main.c -o src/main.o
gcc -std=c11 -O2 -Wall -Wextra -Wpedantic -Isrc -Ivendor/pdcurses -c src/stats.c -o src/stats.o
gcc -std=c11 -O2 -Wall -Wextra -Wpedantic -Isrc -Ivendor/pdcurses -c src/store.c -o src/store.o
gcc -std=c11 -O2 -Wall -Wextra -Wpedantic -Isrc -Ivendor/pdcurses -c src/tui.c -o src/tui.o
gcc -std=c11 -O2 -Wall -Wextra -Wpedantic -Isrc -Ivendor/pdcurses -c src/util.c -o src/util.o

echo Compiling pdcurses...
for %%f in (vendor\pdcurses\pdcurses\*.c) do gcc -std=c11 -O2 -Ivendor/pdcurses -c %%f -o %%~dpnf.o
for %%f in (vendor\pdcurses\wincon\*.c) do gcc -std=c11 -O2 -Ivendor/pdcurses -c %%f -o %%~dpnf.o

echo Linking...
gcc -o booktracker.exe src\*.o vendor\pdcurses\pdcurses\*.o vendor\pdcurses\wincon\*.o -luser32 -lgdi32

if exist booktracker.exe (
    echo Success! Run with: booktracker.exe tui
) else (
    echo Build failed - check errors above
)
