files_h=$(find . -path ./build -prune -o -path ./junk -prune -o -path ./.git -prune -o -name *.h -print)
files_c=$(find . -path ./build -prune -o -path ./junk -prune -o -path ./.git -prune -o -name *.c -print)
files_txt=$(find . -path ./build -prune -o -path ./junk -prune -o -path ./.git -prune -o -name *.txt -print)

geany -i $files_c $files_h $files_txt TODO &
