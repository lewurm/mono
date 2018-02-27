/**
 * \file
 * Copyright 2018 Microsoft
 * Licensed under the MIT license. See LICENSE file in the project root for full license information.
 */
#include <mono/metadata/method-builder-ilgen.h>

/**
 * mono_mb_add_local:
 */
int
mono_mb_add_local (MonoMethodBuilder *mb, MonoType *type)
{
	int res;
	MonoType *t;

	/*
	 * Have to make a copy early since type might be sig->ret,
	 * which is transient, see mono_metadata_signature_dup_internal_with_padding ().
	 */
	t = mono_metadata_type_dup (NULL, type);

	g_assert (mb != NULL);
	g_assert (type != NULL);

	res = mb->locals;
	mb->locals_list = g_list_append (mb->locals_list, t);
	mb->locals++;

	return res;
}

/**
 * mono_mb_patch_addr:
 */
void
mono_mb_patch_addr (MonoMethodBuilder *mb, int pos, int value)
{
	mb->code [pos] = value & 0xff;
	mb->code [pos + 1] = (value >> 8) & 0xff;
	mb->code [pos + 2] = (value >> 16) & 0xff;
	mb->code [pos + 3] = (value >> 24) & 0xff;
}

/**
 * mono_mb_patch_addr_s:
 */
void
mono_mb_patch_addr_s (MonoMethodBuilder *mb, int pos, gint8 value)
{
	*((gint8 *)(&mb->code [pos])) = value;
}

/**
 * mono_mb_emit_byte:
 */
void
mono_mb_emit_byte (MonoMethodBuilder *mb, guint8 op)
{
	if (mb->pos >= mb->code_size) {
		mb->code_size += mb->code_size >> 1;
		mb->code = (unsigned char *)g_realloc (mb->code, mb->code_size);
	}

	mb->code [mb->pos++] = op;
}

/**
 * mono_mb_emit_ldflda:
 */
void
mono_mb_emit_ldflda (MonoMethodBuilder *mb, gint32 offset)
{
        mono_mb_emit_byte (mb, MONO_CUSTOM_PREFIX);
        mono_mb_emit_byte (mb, CEE_MONO_OBJADDR);

	if (offset) {
		mono_mb_emit_icon (mb, offset);
		mono_mb_emit_byte (mb, CEE_ADD);
	}
}

/**
 * mono_mb_emit_i4:
 */
void
mono_mb_emit_i4 (MonoMethodBuilder *mb, gint32 data)
{
	if ((mb->pos + 4) >= mb->code_size) {
		mb->code_size += mb->code_size >> 1;
		mb->code = (unsigned char *)g_realloc (mb->code, mb->code_size);
	}

	mono_mb_patch_addr (mb, mb->pos, data);
	mb->pos += 4;
}

void
mono_mb_emit_i8 (MonoMethodBuilder *mb, gint64 data)
{
	if ((mb->pos + 8) >= mb->code_size) {
		mb->code_size += mb->code_size >> 1;
		mb->code = (unsigned char *)g_realloc (mb->code, mb->code_size);
	}

	mono_mb_patch_addr (mb, mb->pos, data);
	mono_mb_patch_addr (mb, mb->pos + 4, data >> 32);
	mb->pos += 8;
}

/**
 * mono_mb_emit_i2:
 */
void
mono_mb_emit_i2 (MonoMethodBuilder *mb, gint16 data)
{
	if ((mb->pos + 2) >= mb->code_size) {
		mb->code_size += mb->code_size >> 1;
		mb->code = (unsigned char *)g_realloc (mb->code, mb->code_size);
	}

	mb->code [mb->pos] = data & 0xff;
	mb->code [mb->pos + 1] = (data >> 8) & 0xff;
	mb->pos += 2;
}

void
mono_mb_emit_op (MonoMethodBuilder *mb, guint8 op, gpointer data)
{
	mono_mb_emit_byte (mb, op);
	mono_mb_emit_i4 (mb, mono_mb_add_data (mb, data));
}

/**
 * mono_mb_emit_ldstr:
 */
void
mono_mb_emit_ldstr (MonoMethodBuilder *mb, char *str)
{
	mono_mb_emit_op (mb, CEE_LDSTR, str);
}

/**
 * mono_mb_emit_ldarg:
 */
void
mono_mb_emit_ldarg (MonoMethodBuilder *mb, guint argnum)
{
	if (argnum < 4) {
 		mono_mb_emit_byte (mb, CEE_LDARG_0 + argnum);
	} else if (argnum < 256) {
		mono_mb_emit_byte (mb, CEE_LDARG_S);
		mono_mb_emit_byte (mb, argnum);
	} else {
		mono_mb_emit_byte (mb, CEE_PREFIX1);
		mono_mb_emit_byte (mb, CEE_LDARG);
		mono_mb_emit_i2 (mb, argnum);
	}
}

/**
 * mono_mb_emit_ldarg_addr:
 */
void
mono_mb_emit_ldarg_addr (MonoMethodBuilder *mb, guint argnum)
{
	if (argnum < 256) {
		mono_mb_emit_byte (mb, CEE_LDARGA_S);
		mono_mb_emit_byte (mb, argnum);
	} else {
		mono_mb_emit_byte (mb, CEE_PREFIX1);
		mono_mb_emit_byte (mb, CEE_LDARGA);
		mono_mb_emit_i2 (mb, argnum);
	}
}

/**
 * mono_mb_emit_ldloc_addr:
 */
void
mono_mb_emit_ldloc_addr (MonoMethodBuilder *mb, guint locnum)
{
	if (locnum < 256) {
		mono_mb_emit_byte (mb, CEE_LDLOCA_S);
		mono_mb_emit_byte (mb, locnum);
	} else {
		mono_mb_emit_byte (mb, CEE_PREFIX1);
		mono_mb_emit_byte (mb, CEE_LDLOCA);
		mono_mb_emit_i2 (mb, locnum);
	}
}

/**
 * mono_mb_emit_ldloc:
 */
void
mono_mb_emit_ldloc (MonoMethodBuilder *mb, guint num)
{
	if (num < 4) {
 		mono_mb_emit_byte (mb, CEE_LDLOC_0 + num);
	} else if (num < 256) {
		mono_mb_emit_byte (mb, CEE_LDLOC_S);
		mono_mb_emit_byte (mb, num);
	} else {
		mono_mb_emit_byte (mb, CEE_PREFIX1);
		mono_mb_emit_byte (mb, CEE_LDLOC);
		mono_mb_emit_i2 (mb, num);
	}
}

/**
 * mono_mb_emit_stloc:
 */
void
mono_mb_emit_stloc (MonoMethodBuilder *mb, guint num)
{
	if (num < 4) {
 		mono_mb_emit_byte (mb, CEE_STLOC_0 + num);
	} else if (num < 256) {
		mono_mb_emit_byte (mb, CEE_STLOC_S);
		mono_mb_emit_byte (mb, num);
	} else {
		mono_mb_emit_byte (mb, CEE_PREFIX1);
		mono_mb_emit_byte (mb, CEE_STLOC);
		mono_mb_emit_i2 (mb, num);
	}
}

/**
 * mono_mb_emit_icon:
 */
void
mono_mb_emit_icon (MonoMethodBuilder *mb, gint32 value)
{
	if (value >= -1 && value < 8) {
		mono_mb_emit_byte (mb, CEE_LDC_I4_0 + value);
	} else if (value >= -128 && value <= 127) {
		mono_mb_emit_byte (mb, CEE_LDC_I4_S);
		mono_mb_emit_byte (mb, value);
	} else {
		mono_mb_emit_byte (mb, CEE_LDC_I4);
		mono_mb_emit_i4 (mb, value);
	}
}

void
mono_mb_emit_icon8 (MonoMethodBuilder *mb, gint64 value)
{
	mono_mb_emit_byte (mb, CEE_LDC_I8);
	mono_mb_emit_i8 (mb, value);
}

int
mono_mb_get_label (MonoMethodBuilder *mb)
{
	return mb->pos;
}

int
mono_mb_get_pos (MonoMethodBuilder *mb)
{
	return mb->pos;
}

/**
 * mono_mb_emit_branch:
 */
guint32
mono_mb_emit_branch (MonoMethodBuilder *mb, guint8 op)
{
	guint32 res;
	mono_mb_emit_byte (mb, op);
	res = mb->pos;
	mono_mb_emit_i4 (mb, 0);
	return res;
}

guint32
mono_mb_emit_short_branch (MonoMethodBuilder *mb, guint8 op)
{
	guint32 res;
	mono_mb_emit_byte (mb, op);
	res = mb->pos;
	mono_mb_emit_byte (mb, 0);

	return res;
}

void
mono_mb_emit_branch_label (MonoMethodBuilder *mb, guint8 op, guint32 label)
{
	mono_mb_emit_byte (mb, op);
	mono_mb_emit_i4 (mb, label - (mb->pos + 4));
}

void
mono_mb_patch_branch (MonoMethodBuilder *mb, guint32 pos)
{
	mono_mb_patch_addr (mb, pos, mb->pos - (pos + 4));
}

void
mono_mb_patch_short_branch (MonoMethodBuilder *mb, guint32 pos)
{
	mono_mb_patch_addr_s (mb, pos, mb->pos - (pos + 1));
}

void
mono_mb_emit_ptr (MonoMethodBuilder *mb, gpointer ptr)
{
	mono_mb_emit_byte (mb, MONO_CUSTOM_PREFIX);
	mono_mb_emit_op (mb, CEE_MONO_LDPTR, ptr);
}

void
mono_mb_emit_calli (MonoMethodBuilder *mb, MonoMethodSignature *sig)
{
	mono_mb_emit_op (mb, CEE_CALLI, sig);
}

/**
 * mono_mb_emit_managed_call:
 */
void
mono_mb_emit_managed_call (MonoMethodBuilder *mb, MonoMethod *method, MonoMethodSignature *opt_sig)
{
	mono_mb_emit_op (mb, CEE_CALL, method);
}

/**
 * mono_mb_emit_native_call:
 */
void
mono_mb_emit_native_call (MonoMethodBuilder *mb, MonoMethodSignature *sig, gpointer func)
{
	mono_mb_emit_ptr (mb, func);
	mono_mb_emit_calli (mb, sig);
}

void
mono_mb_emit_icall (MonoMethodBuilder *mb, gpointer func)
{
	mono_mb_emit_byte (mb, MONO_CUSTOM_PREFIX);
	mono_mb_emit_op (mb, CEE_MONO_ICALL, func);
}

void
mono_mb_emit_exception_full (MonoMethodBuilder *mb, const char *exc_nspace, const char *exc_name, const char *msg)
{
	MonoMethod *ctor = NULL;

	MonoClass *mme = mono_class_load_from_name (mono_defaults.corlib, exc_nspace, exc_name);
	mono_class_init (mme);
	ctor = mono_class_get_method_from_name (mme, ".ctor", 0);
	g_assert (ctor);
	mono_mb_emit_op (mb, CEE_NEWOBJ, ctor);
	if (msg != NULL) {
		mono_mb_emit_byte (mb, CEE_DUP);
		mono_mb_emit_ldflda (mb, MONO_STRUCT_OFFSET (MonoException, message));
		mono_mb_emit_ldstr (mb, (char*)msg);
		mono_mb_emit_byte (mb, CEE_STIND_REF);
	}
	mono_mb_emit_byte (mb, CEE_THROW);
}

/**
 * mono_mb_emit_exception:
 */
void
mono_mb_emit_exception (MonoMethodBuilder *mb, const char *exc_name, const char *msg)
{
	mono_mb_emit_exception_full (mb, "System", exc_name, msg);
}

/**
 * mono_mb_emit_exception_for_error:
 */
void
mono_mb_emit_exception_for_error (MonoMethodBuilder *mb, MonoError *error)
{
	/*
	 * If at some point there is need to support other types of errors,
	 * the behaviour should conform with mono_error_prepare_exception().
	 */
	g_assert (mono_error_get_error_code (error) == MONO_ERROR_GENERIC && "Unsupported error code.");
	mono_mb_emit_exception_full (mb, "System", mono_error_get_exception_name (error), mono_error_get_message (error));
}

/**
 * mono_mb_emit_add_to_local:
 */
void
mono_mb_emit_add_to_local (MonoMethodBuilder *mb, guint16 local, gint32 incr)
{
	mono_mb_emit_ldloc (mb, local); 
	mono_mb_emit_icon (mb, incr);
	mono_mb_emit_byte (mb, CEE_ADD);
	mono_mb_emit_stloc (mb, local); 
}

void
mono_mb_set_clauses (MonoMethodBuilder *mb, int num_clauses, MonoExceptionClause *clauses)
{
	mb->num_clauses = num_clauses;
	mb->clauses = clauses;
}

/*
 * mono_mb_set_param_names:
 *
 *   PARAM_NAMES should have length equal to the sig->param_count, the caller retains
 * ownership of the array, and its entries.
 */
void
mono_mb_set_param_names (MonoMethodBuilder *mb, const char **param_names)
{
	mb->param_names = param_names;
}

#endif /* DISABLE_JIT */
