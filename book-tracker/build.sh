echo "Cleaning..."
rm -f booktracker src/*.o

echo "Compiling..."
gcc -std=c11 -O2 -Wall -Wextra -Wpedantic -Isrc -c src/commands.c -o src/commands.o
gcc -std=c11 -O2 -Wall -Wextra -Wpedantic -Isrc -c src/dates.c -o src/dates.o
gcc -std=c11 -O2 -Wall -Wextra -Wpedantic -Isrc -c src/main.c -o src/main.o
gcc -std=c11 -O2 -Wall -Wextra -Wpedantic -Isrc -c src/stats.c -o src/stats.o
gcc -std=c11 -O2 -Wall -Wextra -Wpedantic -Isrc -c src/store.c -o src/store.o
gcc -std=c11 -O2 -Wall -Wextra -Wpedantic -Isrc -c src/tui.c -o src/tui.o
gcc -std=c11 -O2 -Wall -Wextra -Wpedantic -Isrc -c src/util.c -o src/util.o

echo "Linking..."
gcc -o booktracker src/*.o -lncursesw

echo "Done! Run with: ./booktracker tui"
