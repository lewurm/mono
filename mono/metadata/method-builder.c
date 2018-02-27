/**
 * \file
 * Functions for creating IL methods at runtime.
 * 
 * Author:
 *   Paolo Molaro (lupus@ximian.com)
 *
 * Copyright 2002-2003 Ximian, Inc (http://www.ximian.com)
 * Copyright 2004-2009 Novell, Inc (http://www.novell.com)
 * Licensed under the MIT license. See LICENSE file in the project root for full license information.
 */

#include "config.h"
#include "loader.h"
#include "mono/metadata/abi-details.h"
#include "mono/metadata/method-builder.h"
#include "mono/metadata/tabledefs.h"
#include "mono/metadata/exception.h"
#include "mono/metadata/appdomain.h"
#include "mono/metadata/debug-helpers.h"
#include "mono/metadata/metadata-internals.h"
#include "mono/metadata/domain-internals.h"
#include <string.h>
#include <errno.h>

/* #define DEBUG_RUNTIME_CODE */

#define OPDEF(a,b,c,d,e,f,g,h,i,j) \
	a = i,

enum {
#include "mono/cil/opcode.def"
	LAST = 0xff
};
#undef OPDEF

#ifdef DEBUG_RUNTIME_CODE
static char*
indenter (MonoDisHelper *dh, MonoMethod *method, guint32 ip_offset)
{
	return g_strdup (" ");
}

static MonoDisHelper marshal_dh = {
	"\n",
	"IL_%04x: ",
	"IL_%04x",
	indenter, 
	NULL,
	NULL
};
#endif 

static MonoMethodBuilder *
mono_mb_new_base (MonoClass *klass, MonoWrapperType type)
{
	MonoMethodBuilder *mb;
	MonoMethod *m;

	g_assert (klass != NULL);

	mb = g_new0 (MonoMethodBuilder, 1);

	mb->method = m = (MonoMethod *)g_new0 (MonoMethodWrapper, 1);

	m->klass = klass;
	m->inline_info = 1;
	m->wrapper_type = type;

#ifdef ENABLE_ILGEN
	mb->code_size = 40;
	mb->code = (unsigned char *)g_malloc (mb->code_size);
	mb->init_locals = TRUE;
#endif
	/* placeholder for the wrapper always at index 1 */
	mono_mb_add_data (mb, NULL);

	return mb;
}

MonoMethodBuilder *
mono_mb_new_no_dup_name (MonoClass *klass, const char *name, MonoWrapperType type)
{
	MonoMethodBuilder *mb = mono_mb_new_base (klass, type);
	mb->name = (char*)name;
	mb->no_dup_name = TRUE;
	return mb;
}

/**
 * mono_mb_new:
 */
MonoMethodBuilder *
mono_mb_new (MonoClass *klass, const char *name, MonoWrapperType type)
{
	MonoMethodBuilder *mb = mono_mb_new_base (klass, type);
	mb->name = g_strdup (name);
	return mb;
}

/**
 * mono_mb_free:
 */
void
mono_mb_free (MonoMethodBuilder *mb)
{
#ifdef ENABLE_ILGEN
	GList *l;

	for (l = mb->locals_list; l; l = l->next) {
		/* Allocated in mono_mb_add_local () */
		g_free (l->data);
	}
	g_list_free (mb->locals_list);
	if (!mb->dynamic) {
		g_free (mb->method);
		if (!mb->no_dup_name)
			g_free (mb->name);
		g_free (mb->code);
	}
#else
	g_free (mb->method);
	if (!mb->no_dup_name)
		g_free (mb->name);
#endif
	g_free (mb);
}

/**
 * mono_mb_create_method:
 * Create a \c MonoMethod from this method builder.
 * \returns the newly created method.
 */
MonoMethod *
mono_mb_create_method (MonoMethodBuilder *mb, MonoMethodSignature *signature, int max_stack)
{
#ifdef ENABLE_ILGEN
	MonoMethodHeader *header;
#endif
	MonoMethodWrapper *mw;
	MonoImage *image;
	MonoMethod *method;
	GList *l;
	int i;

	g_assert (mb != NULL);

	image = mb->method->klass->image;

#ifdef ENABLE_ILGEN
	if (mb->dynamic) {
		method = mb->method;
		mw = (MonoMethodWrapper*)method;

		method->name = mb->name;
		method->dynamic = TRUE;

		mw->header = header = (MonoMethodHeader *) 
			g_malloc0 (MONO_SIZEOF_METHOD_HEADER + mb->locals * sizeof (MonoType *));

		header->code = mb->code;

		for (i = 0, l = mb->locals_list; l; l = l->next, i++) {
			header->locals [i] = (MonoType*)l->data;
		}
	} else
#endif
	{
		/* Realloc the method info into a mempool */

		method = (MonoMethod *)mono_image_alloc0 (image, sizeof (MonoMethodWrapper));
		memcpy (method, mb->method, sizeof (MonoMethodWrapper));
		mw = (MonoMethodWrapper*) method;

		if (mb->no_dup_name)
			method->name = mb->name;
		else
			method->name = mono_image_strdup (image, mb->name);

#ifdef ENABLE_ILGEN
		mw->header = header = (MonoMethodHeader *) 
			mono_image_alloc0 (image, MONO_SIZEOF_METHOD_HEADER + mb->locals * sizeof (MonoType *));

		header->code = (const unsigned char *)mono_image_alloc (image, mb->pos);
		memcpy ((char*)header->code, mb->code, mb->pos);

		for (i = 0, l = mb->locals_list; l; l = l->next, i++) {
			header->locals [i] = (MonoType*)l->data;
		}
#endif
	}

#ifdef ENABLE_ILGEN
	/* Free the locals list so mono_mb_free () doesn't free the types twice */
	g_list_free (mb->locals_list);
	mb->locals_list = NULL;
#endif

	method->signature = signature;
	if (!signature->hasthis)
		method->flags |= METHOD_ATTRIBUTE_STATIC;

#ifdef ENABLE_ILGEN
	if (max_stack < 8)
		max_stack = 8;

	header->max_stack = max_stack;

	header->code_size = mb->pos;
	header->num_locals = mb->locals;
	header->init_locals = mb->init_locals;

	header->num_clauses = mb->num_clauses;
	header->clauses = mb->clauses;

	method->skip_visibility = mb->skip_visibility;
#endif

	i = g_list_length ((GList *)mw->method_data);
	if (i) {
		GList *tmp;
		void **data;
		l = g_list_reverse ((GList *)mw->method_data);
		if (method_is_dynamic (method))
			data = (void **)g_malloc (sizeof (gpointer) * (i + 1));
		else
			data = (void **)mono_image_alloc (image, sizeof (gpointer) * (i + 1));
		/* store the size in the first element */
		data [0] = GUINT_TO_POINTER (i);
		i = 1;
		for (tmp = l; tmp; tmp = tmp->next) {
			data [i++] = tmp->data;
		}
		g_list_free (l);

		mw->method_data = data;
	}

#ifdef ENABLE_ILGEN
	/*{
		static int total_code = 0;
		static int total_alloc = 0;
		total_code += mb->pos;
		total_alloc += mb->code_size;
		g_print ("code size: %d of %d (allocated: %d)\n", mb->pos, total_code, total_alloc);
	}*/

#ifdef DEBUG_RUNTIME_CODE
	printf ("RUNTIME CODE FOR %s\n", mono_method_full_name (method, TRUE));
	printf ("%s\n", mono_disasm_code (&marshal_dh, method, mb->code, mb->code + mb->pos));
#endif

	if (mb->param_names) {
		char **param_names = (char **)mono_image_alloc0 (image, signature->param_count * sizeof (gpointer));
		for (i = 0; i < signature->param_count; ++i)
			param_names [i] = mono_image_strdup (image, mb->param_names [i]);

		mono_image_lock (image);
		if (!image->wrapper_param_names)
			image->wrapper_param_names = g_hash_table_new (NULL, NULL);
		g_hash_table_insert (image->wrapper_param_names, method, param_names);
		mono_image_unlock (image);
	}
#endif

	return method;
}

/**
 * mono_mb_add_data:
 */
guint32
mono_mb_add_data (MonoMethodBuilder *mb, gpointer data)
{
	MonoMethodWrapper *mw;

	g_assert (mb != NULL);

	mw = (MonoMethodWrapper *)mb->method;

	/* one O(n) is enough */
	mw->method_data = g_list_prepend ((GList *)mw->method_data, data);

	return g_list_length ((GList *)mw->method_data);
}

