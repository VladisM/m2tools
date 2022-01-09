# Building from source

Toolchain is distributed only in source form. Same is valid for documentation,
if you received this document already in pdf form, then good for you. To build
it you will need some prerequisites.

 * git
 * cmake *At least version 3.13.0 is required.*
 * C99 compiler *Use anything you want.*

You have to start with downloading source codes, this can be done by git as
follow. This will also recursively clone all submodules.

```
$ git clone --recurse-submodules git@github.com:VladisM/m2tools.git
```

After successful clone, you can then use cmake to configure toolchain and build
it. For configuration there is some variables in cmake to be set. At now, there
are following variables used.

 * **TARGET_ARCH** Target architecture name to build toolchain for. Empty by
 default.

Individual targets can have another options defined, see their documentation
for this.

For example, cmake command that will compile everything into build folder next
to source one can look like this.

```
$ cmake -S m2tools/ -B build/ -DTARGET_ARCH=example
```

To build everything you can also use cmake.

```
$ cmake --build build/
```

And you are all done. Your binaries can be located inside root of build
directory, you can find pdf file with documentation in doc subfolder. Note
that binaries will have prefix for your specified platform. This way you
can have multiple configurations installed on your system.
