file_linker=$(find ./linker/ -name *.h -o -name *.c -o -name *.txt)
file_isa=$(find ./isalib/ -name *.h -o -name *.c -o -name *.txt)
file_ldm=$(find ./ldmlib/ -name *.h -o -name *.c -o -name *.txt)
file_obj=$(find ./objlib/ -name *.h -o -name *.c -o -name *.txt)
file_sl=$(find ./sllib/ -name *.h -o -name *.c -o -name *.txt)

geany -i $file_linker $file_isa $file_ldm $file_obj $file_sl TODO CMakeLists.txt &


