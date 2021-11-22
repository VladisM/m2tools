# Introduction

Project m2tools is collection of tools that can be used as retargetable
assembler toolchain. At first it was intended as replacement of toolchain
written in Python for custom CPU called MARK-II. But as I believe that it
can be widely used by others for their architectures it was separated
from MARK-II project.

At this moment following tools are included:

 * assembler
 * linker
 * archiver
 * objread

Assembler is used to translate assembly language into object files that can
be later linked using linker. There is also archiver that can be used to create
static library that can be linked too. And for basic object files investigation
and trouble shooting there is also objread utility that can read and display
content of object files.

Whole project is made open source and you can find actual source here:
<https://github.com/VladisM/m2tools>. Feedback and contributions are welcome
but as license says, do what ever you want to do.
