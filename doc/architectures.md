# Supported architectures

In this section you can find some information to each supported architecture.

## Example

This is dummy target intended only for demonstration purposes. It can be
successfully compiled and linked, but will throw errors when you try to run it.
You should use this target to see what should be implemented when you are
adding new one. To compile this architecture please set target variable to
"example" as follows.

```
-DTARGET_ARCH=example
```

## i8080

Backend for Intel 8080 CPU. This architecture was selected as easy enough to
demonstrate function of toolchain. i8080 was launched at 1974, but some hobbyist
is still using this CPU for nostalgic reasons in many 8bit computers. To compile
toolchain for this architecture one have to set following variable.

```
-DTARGET_ARCH=i8080
```
