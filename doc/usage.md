# Usage of toolchain

In this section usage of toolchain component is described.

## Assembler

Assembler is designed as two pass assembler, during first pass, symbols are
found and their values are assigned after first pass is completed. Input files
are plain text files with instructions and pseudo instructions written inside
of it.

Convention is that these files end with *.asm* suffix. But this is not required.

Assembler is also equipped with very simple preprocessor that can include
other files and also deal with basic macros. There is also support for condition
translation of files just like any ANSI C preprocessor do.

Although, language syntax depends on what instructions are provided by backend,
some generals rules also apply. Because this is relative simply toolchain some
functionality that usually is available is stripped down.

One example is, there is no support for mathematical operations with symbols in
assembler. Lets assume in following example *LD* is instruction that take two
arguments, one is register name and second is immediate address. Also lets say
that *FOO* is label defined somewhere in the code. Then, following construct is
not valid.

```
LD R1 FOO + 2
```

If instruction take two arguments, then there have to be two tokens after
instruction name on the line.

Another common thing is expansion of macros. You can specify only macro as
constant value that will replace other token somewhere in code. But functions
like macros aren't supported. Following code is possible, but nothing more.

```
#DEFINE FOO 1
LD R1 FOO
```

## Assembler syntax

Syntax is composed from target specific reserved words (instructions), from
preprocessor directives and also from internal pseudo instructions.

### Comments

Comment start with semicolon and as comment is counted everything till end of
line.

```
; This is comment
```

### Labels

Labels are used as pointer to specific memory location. It will translate into
value of program counter at the place where label was created. They end up with
colon.

```
FOO:    ;this is label FOO
```

Labels can be used by instructions that want address as argument. Address will
be put into instruction by linker during link step.

### Preprocessor #include

Include directive will force tokenizer to load and parse file specified as
argument event before first pass of assembler start. Path can be absolute or
relative from current file.

```
#include defs.asm
```

### Preprocessor #define

Using this directive you can define new macro that will replace all instances
by its value in rest of input files. Even in included one.

```
#define PIN_ADDRESS 10
```

When ever PIN\_ADDRESS appears, it will be replaced by value 10. You can also
replace with string not only with integers.

### Preprocessor #ifdef #ifndef #endif

There three preprocessor commands are used for conditional compilation of input.
When ever preprocessor encounter false condition it will skip parsing of input
till corresponding *#endif* command.

```
#define ENABLE_FOO  ;comment out to disable FOO

#ifdef ENABLE_FOO
; some code valid only for FOO enabled
#endif
```

### Pseudo instruction .ORG

Pseudo instruction used to change actual value of internal program counter. If
used, next item will be placed at specified address. Program counter is relative
to currently active section.

```
.ORG 0x100 ; next item will be placed on address 0x100 in current section
```

This is not absolute position in your CPU address space. It depend on setting
that you pass into linker.

### Pseudo instruction .CONS

CONS pseudo instruction will create artificial label definition that will be
fixed in value. That mean, you can for example set top of your ram as constant
and use it as any other label in instructions.

```
.CONS RAM_0_TOP 0x1000
LD R1 RAM_0_TOP
```

### Pseudo instruction .DAT

This will emit binary data into output. It is called blob inside of toolchain
and is exactly one memory element big according to your target.

```
.DAT 0xAA ; one word with value 0xAA will be emit into output
```

### Pseudo instruction .DS

DS pseudo instruction is used to create data space in output, there nothing will
be stored by assembler. Can be used for example to create statically allocated
arrays.

```
FOO:    ; this will create continuous space of 10 memory elements
.DS 10
```

### Pseudo instructions .EXPORT and .IMPORT

Symbols, or better say, labels, can be exported or imported from and into
current module by these two pseudo instructions. Each one take name of symbol
as argument.

Imagine following situation, we have two files, one file have defined routine
*FOO* that we want to call from other file. As each file will be processed
by assembler individually and connection is made in next step by liner, we have
to somehow inform assembler that *FOO* should be visible and linker that
it should look for *FOO* in global symbol table.

```
.EXPORT FOO
FOO: ;some subroutine code here

; in other file
.IMPORT FOO
CALL FOO ; will call FOO
```

Please note that if you are working with two different sections in one file, you
have to still use these two pseudo instructions as symbols and linking work on
section basis, not on files.

### Pseudo instruction .SECTION

This one is used to crate and switch sections of your code. For example, in ANSI
C you have three main sections and these are *text*, *data* and *bss*.

In section *bss* you usually have variables and in section *text* code
is placed. Lets say in section *bss* you would like to have array of ten words
and want to access it from your code. Following example deal with this.

```
.SECTION bss
.EXPORT myArray
myArray:
.DS 10

.SECTION text
.IMPORT myArray
; your code can now use myArray symbol
```

Sections are essentials for linker to deal with different memory spaces and
memory layout. Usually you will want your *text* and *data* sections to be
placed into non volatile memory and *bss* into volatile one that will be
initialized by some startup routine.

## Linker

Design of linker is as simple as possible, but nevertheless is fully functional
and does have enough features to fully support linking of programs written
in ansi C language.

The core of linker is so called cache, that hold together all symbols, their
addresses as well as their other properties. On this cache is later run linking
process itself.

Input to a linker are object files that came out from assembler. It is basically
list of code sections filled with mix of data and instructions. Another
important input is linker script.

Linker script tells linker how exactly link together sections and where to put
them in address space of target CPU. There is also possibility to create symbols
from linker script. Even dynamic one (value is calculated at link time).

Another possible input of linker is archives. Archive is basically just bunch
of object files grouped together for better manipulation by archiver program.
These libraries can be only static as dynamic linking require support by loader
wich is typically part of operating system running on target. This can make
this toolchain much less retargetable.

## Linker script syntax

Linker script is file that tells Linker mainly where to put sections in memory.
As this toolchain is written with embedded in mind, absolute addresses are
used. Another purpose for linker script is to define symbols and their values.

Symbols defined by linker can be absolute, so their value will be set by linker
script to specified number or computed one. Computed or evaluated symbols are
calculated when all other symbols are known in symbol table. To calculate
symbol value given expression is used.

Common suffix for linker script files is *.lds* but it is not mandatory.

### Entry point

Generally speaking, it is necessary to inform CPU where to start it's execution.
Fot this purpose entry point have to be set to point into one of the symbols
defined in the symbol table.

Some architectures, like ARM Cortex M CPUs have this done by vector table
that have to be placed on the beginning of the code memory. Such mechanism have
to be done manually by programmer.

In following example lets assume entry point is routine called *_startup*. In
the linker script will be written:

```
ENT _startup
```

### Memory declaration

Memories have to be declared by linker script. They are used in output LDM file.
Memory is defined by its name, start address and size.

In example, we will define two memories, *RAM_0* and *ROM_0*, our ram will have
1k words (1024) and will be placed on address 0x400, ram will be only 256 words
long and will be placed on address zero.

```
MEM RAM_0 1k 0x000400
MEM ROM_0 256 0x000000
```

### Assign sections into memories

Code is usually split into sections, some general approach is to have *text*
section that contain executable code, *bss* section that contain uninitialized
variables and *data* section that contain initialized ones. To assign them into
memories there is PUT command.

```
PUT text ROM_0
PUT bss RAM_0
PUT data RAM_0
```

This command also support asterisk to filter sections name. So, lets say we
created "subsections" in text section by using prefix *text_*. Then we can use
following to put them all into *ROM_0* memory.

```
PUT text_* ROM_0
```

If multiple sections are assigned into one memory they will be located in
same order as *PUT* commands were used.

### Create symbols

For creating symbols *SET* command in available. This command have two variants.
First variant is able to create simple, absolute symbol with defined value.

```
SET RAM_0_START 0x000400
SET ROM_0_START 0x000000
```

Second variant is computed one. For this *EVAL* and *ENDEVAL* keywords are used.

```
SET RAM_0_END EVAL RAM_0_START + 1024 ENDEVAL
```

Symbol RAM_0_END will be then filled with value 0x0007ff.

These symbols can be utilized in the code for various uses. If you use *.IMPORT*
command you can gen value of this symbol into your assembly code and use it.
In following code snipped we will set stack pointer to end of RAM. Imagine
that *SP* in stack pointer register and *MVI* will load given value into
register.

```
;set SP to the end of RAM_0
.IMPORT RAM_0_END
MVI SP RAM_0_END
```

### SET EVAL functions

For more convenient usage of eval functionality of set command, there is
multiple functions build in into evaluator. These are:

* mem_begin(mem_name)
* mem_size(mem_name)
* section_begin(sec_name)
* section_size(sec_name)

All functions will return value of address type. Argument is always only one,
and it is name of memory or name of section. This feature can be used as follows.

```
MEM RAM_0 1k 0x000400

SET RAM_0_START EVAL mem_begin(RAM_0) ENDEVAL
SET RAM_0_END EVAL RAM_0_START + mem_size(RAM_0) - 1 ENDEVAL
```

## Archiver

Archiver is verry simple utility that pack together object files generated
by assembler. Its output is one library file that can be used as input
into linker. Their purpose is to simplify transfer and manipulation with
larger code base. Produced libraries are only static one. There is no support
for dynamic ones.

Same utility can also provide information about what files are packed in
library and unpack them again. At this point, any compression algorithm is
implemented.

## Objread

Object read utility is intended to inspect object files. It can print out
defined symbols in file, it can also dump their values and data symbols (code).
