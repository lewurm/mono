/**
 * \file
 * Functions for creating IL methods at runtime.
 * 
 * Author:
 *   Paolo Molaro (lupus@ximian.com)
 *
 * (C) 2002 Ximian, Inc.  http://www.ximian.com
 *
 */

#ifndef __MONO_METHOD_BUILDER_H__
#define __MONO_METHOD_BUILDER_H__

#include "config.h"
#include <mono/metadata/class.h>
#include <mono/metadata/object-internals.h>
#include <mono/metadata/class-internals.h>
#include <mono/metadata/opcodes.h>
#include <mono/metadata/reflection.h>

G_BEGIN_DECLS

typedef struct _MonoMethodBuilder {
	MonoMethod *method;
	char *name;
	gboolean no_dup_name;
#ifdef ENABLE_ILGEN
	GList *locals_list;
	int locals;
	gboolean dynamic;
	gboolean skip_visibility, init_locals;
	guint32 code_size, pos;
	unsigned char *code;
	int num_clauses;
	MonoExceptionClause *clauses;
	const char **param_names;
#endif
} MonoMethodBuilder;

#define MONO_METHOD_BUILDER_CALLBACKS_VERSION 1

typedef struct {
	int version;
	MonoMethodBuilder* (*new_base) (MonoClass *klass, MonoWrapperType type);
	void (*free) (MonoMethodBuilder *mb);
	MonoMethod (*create_method) (MonoMethodBuilder *mb, MonoMethodSignature *signature, int max_stack);
} MonoMethodBuilderCallbacks;

MonoMethodBuilder *
mono_mb_new (MonoClass *klass, const char *name, MonoWrapperType type);

MonoMethodBuilder *
mono_mb_new_no_dup_name (MonoClass *klass, const char *name, MonoWrapperType type);

void
mono_mb_free (MonoMethodBuilder *mb);

MonoMethod *
mono_mb_create_method (MonoMethodBuilder *mb, MonoMethodSignature *signature, int max_stack);

guint32
mono_mb_add_data (MonoMethodBuilder *mb, gpointer data);

G_END_DECLS

#endif /* __MONO_METHOD_BUILDER_H__ */

