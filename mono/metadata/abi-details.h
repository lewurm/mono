/**
 * \file
 * Copyright 2014 Xamarin Inc
 * Licensed under the MIT license. See LICENSE file in the project root for full license information.
 */
#ifndef __MONO_METADATA_ABI_DETAILS_H__
#define __MONO_METADATA_ABI_DETAILS_H__

#include <config.h>
#include <glib.h>

/*
 * This file defines macros to compute sizes/alignments/field offsets which depend on
 * the ABI. It is needed during cross compiling since the generated code needs to
 * contain offsets which correspond to the ABI of the target, not the host.
 * It defines the following macros:
 * - MONO_ABI_SIZEOF(type) for every basic type
 * - MONO_ABI_ALIGNOF(type) for every basic type
 * - MONO_STRUCT_OFFSET(struct, field) for various runtime structures
 * When not cross compiling, these correspond to the host ABI (i.e. sizeof/offsetof).
 * When cross compiling, these are defined in a generated header file which is
 * generated by the offsets tool in tools/offsets-tool. The name of the file
 * is given by the --with-cross-offsets= configure argument.
 */

typedef enum {
	MONO_ALIGN_gint8,
	MONO_ALIGN_gint16,
	MONO_ALIGN_gint32,
	MONO_ALIGN_gint64,
	MONO_ALIGN_float,
	MONO_ALIGN_double,
	MONO_ALIGN_gpointer,
	MONO_ALIGN_COUNT
} CoreTypeAlign;

int mono_abi_alignment (CoreTypeAlign type);

#define USE_CROSS_COMPILE_OFFSETS

#define MONO_ABI_ALIGNOF(type) mono_abi_alignment (MONO_ALIGN_ ## type)
#define MONO_ABI_SIZEOF(type) (MONO_STRUCT_SIZE (type))
#define MONO_CURRENT_ABI_SIZEOF(type) ((int)sizeof(type))

#undef DECL_OFFSET2
#define DECL_OFFSET(struct,field) MONO_OFFSET_ ## struct ## _ ## field = -1,
#define DECL_OFFSET2(struct,field,offset) MONO_OFFSET_ ## struct ## _ ## field = offset,
#define DECL_ALIGN2(type,size)
#define DECL_SIZE(type) MONO_SIZEOF_ ##type = -1,
#define DECL_SIZE2(type,size) MONO_SIZEOF_ ##type = size,


enum {
#include "object-offsets.h"
};

#ifdef USED_CROSS_COMPILER_OFFSETS
#define MONO_STRUCT_OFFSET(struct,field) MONO_OFFSET_ ## struct ## _ ## field
#define MONO_STRUCT_SIZE(struct) MONO_SIZEOF_ ## struct
#else
#if defined(HAS_CROSS_COMPILER_OFFSETS) || defined(MONO_CROSS_COMPILE)
#error "this case looks wrong and should never be reached anyway?"
#define MONO_STRUCT_OFFSET(struct,field) (MONO_OFFSET_ ## struct ## _ ## field == -1, G_STRUCT_OFFSET (struct,field))
#define MONO_STRUCT_SIZE(struct) (MONO_SIZEOF_ ## struct == -1, (int)sizeof(struct))
#else
#define MONO_STRUCT_OFFSET(struct,field) G_STRUCT_OFFSET (struct,field)
#define MONO_STRUCT_SIZE(struct) ((int)sizeof(struct))
#endif
#endif

// #define MONO_SIZEOF_MonoObject (2 * MONO_ABI_SIZEOF(gpointer))
#define MONO_SIZEOF_MonoObject (2 * MONO_SIZEOF_gpointer)
#endif
