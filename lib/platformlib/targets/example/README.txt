Implementing new architecture consist of adding one elseif branch into library
cmake file, one common header file, and implementing all the functions that are
spread across multiple different header files in this folder.

Each header file have doxygen style comments that describing what each function
have to do. Please read them carefully.

Architecture of this toolchain set some constrains to CPUs that can be
supported. They are:

 * All instructions have to take fixed amount of arguments.
 * CPU smallest addressable unit have to be smaller or equal than instruction length.
 * Instruction width have to be integer multiple of smallest addressable unit.

Event with these constrains many small RISC like cores can be fully supported.
