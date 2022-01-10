/**
 * @file platformlib_target_specific.h
 * @brief This file pack together target headers to be included by platformlib
 * and its dependencies.
 *
 * When porting, copy this file with its folder to your backend and write down
 * paths to all header files required when working with your architecture.
 */

#ifndef PLATFORMLIB_TARGET_SPECIFIC_h_included
#define PLATFORMLIB_TARGET_SPECIFIC_h_included

/**
 * @brief This macro define name for your architecture. It will be stored in
 * all output files.
 */
#define TARGET_ARCH_NAME "example_target"

#include "../assemble.h"
#include "../datatypes.h"
#include "../instructions_description.h"
#include "../miscellaneous.h"

#endif
