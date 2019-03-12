files_h=$(find . -path ./build -prune -o -path ./junk -prune -o -path ./.git -prune -o -name *.h -print)
files_c=$(find . -path ./build -prune -o -path ./junk -prune -o -path ./.git -prune -o -name *.c -print)

geany -i $files_c $files_h CMakeLists.txt TODO &
