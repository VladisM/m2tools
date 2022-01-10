Portable toolchain
=======================

What is this? Well, very simple toolchain for custom CPU architectures, it
contain everything for writing programs in assembler. Whole toolchain is written
in pure C99 so, once you will port C compiler for your architecture you should
be able to run this toolchain on your platform too.

Documentation
-----------------------

Documentation can be found in form of markdown files in doc folder. Following
links bring you to individual chapters.

* [introduction](./doc/introduction.md)
* [build](./doc/build.md)
* [usage](./doc/usage.md)
* [architectures](./doc/architectures.md)
* [porting](./doc/porting.md)


Build
-----------------------

As build environment CMake is used. So basically you just

```
$ cmake -S . -B build && cmake --build build/
```

and you are done. For more details please visit [build help](./doc/build.md).

Supported architectures
-----------------------

* i8080

License
-----------------------

Copyright © 2021 Vladislav Mlejnecký <v.mlejnecky@seznam.cz>

This work is free. You can redistribute it and/or modify it under the
terms of the Do What The Fuck You Want To Public License, Version 2,
as published by Sam Hocevar. See COPYING file for more details.
