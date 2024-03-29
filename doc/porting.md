# Porting to new architecture

Portability to new architecture is done by so called *platformlib*. It is one
core module of toolchain that contain platform dependant code. Rest of the
code is written in target independent manner.

Process of porting consist from implementation some functions that will be later
called by rest of the code. Detailed explanation can be found in header files in
lib/platformlib/targets/example/*.h.

In general, copy example folder, rename it according to your target and
implement all functionality. Then edit CMakeLists.txt for platformlib and
add your target, again, use example for inspiration.

If you are done, you can then use variable *TARGET_ARCH* to tell cmake to
configure build for your architecture.

```
cmake -S m2tools/ -B build/ -DTARGET_ARCH=my_awesome_arch
```

Architecture of this toolchain set some constrains to CPUs that can be
supported. They are:

 * All instructions have to take fixed amount of arguments.
 * CPU smallest addressable unit have to be smaller or equal than instruction length.
 * Instruction width have to be integer multiple of smallest addressable unit.

If you are unsure, you can always contact author.
