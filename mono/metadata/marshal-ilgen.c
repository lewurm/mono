/*
 * mono_mb_emit_exception_marshal_directive:
 *
 *   This function assumes ownership of MSG, which should be malloc-ed.
 */
static void
mono_mb_emit_exception_marshal_directive (MonoMethodBuilder *mb, char *msg)
{
	char *s;

	if (!mb->dynamic) {
		s = mono_image_strdup (mb->method->klass->image, msg);
		g_free (msg);
	} else {
		s = g_strdup (msg);
	}
	mono_mb_emit_exception_full (mb, "System.Runtime.InteropServices", "MarshalDirectiveException", s);
}

static void
emit_ptr_to_object_conv (MonoMethodBuilder *mb, MonoType *type, MonoMarshalConv conv, MonoMarshalSpec *mspec)
{
	switch (conv) {
	case MONO_MARSHAL_CONV_BOOL_I4:
		mono_mb_emit_ldloc (mb, 1);
		mono_mb_emit_ldloc (mb, 0);
		mono_mb_emit_byte (mb, CEE_LDIND_I4);
		mono_mb_emit_byte (mb, CEE_BRFALSE_S);
		mono_mb_emit_byte (mb, 3);
		mono_mb_emit_byte (mb, CEE_LDC_I4_1);
		mono_mb_emit_byte (mb, CEE_BR_S);
		mono_mb_emit_byte (mb, 1);
		mono_mb_emit_byte (mb, CEE_LDC_I4_0);
		mono_mb_emit_byte (mb, CEE_STIND_I1);
		break;
	case MONO_MARSHAL_CONV_BOOL_VARIANTBOOL:
		mono_mb_emit_ldloc (mb, 1);
		mono_mb_emit_ldloc (mb, 0);
		mono_mb_emit_byte (mb, CEE_LDIND_I2);
		mono_mb_emit_byte (mb, CEE_BRFALSE_S);
		mono_mb_emit_byte (mb, 3);
		mono_mb_emit_byte (mb, CEE_LDC_I4_1);
		mono_mb_emit_byte (mb, CEE_BR_S);
		mono_mb_emit_byte (mb, 1);
		mono_mb_emit_byte (mb, CEE_LDC_I4_0);
		mono_mb_emit_byte (mb, CEE_STIND_I1);
		break;
	case MONO_MARSHAL_CONV_ARRAY_BYVALARRAY: {
		MonoClass *eklass = NULL;
		int esize;

		if (type->type == MONO_TYPE_SZARRAY) {
			eklass = type->data.klass;
		} else {
			g_assert_not_reached ();
		}

		esize = mono_class_native_size (eklass, NULL);

		/* create a new array */
		mono_mb_emit_ldloc (mb, 1);
		mono_mb_emit_icon (mb, mspec->data.array_data.num_elem);
		mono_mb_emit_op (mb, CEE_NEWARR, eklass);	
		mono_mb_emit_byte (mb, CEE_STIND_REF);

		if (eklass->blittable) {
			/* copy the elements */
			mono_mb_emit_ldloc (mb, 1);
			mono_mb_emit_byte (mb, CEE_LDIND_I);
			mono_mb_emit_icon (mb, MONO_STRUCT_OFFSET (MonoArray, vector));
			mono_mb_emit_byte (mb, CEE_ADD);
			mono_mb_emit_ldloc (mb, 0);
			mono_mb_emit_icon (mb, mspec->data.array_data.num_elem * esize);
			mono_mb_emit_byte (mb, CEE_PREFIX1);
			mono_mb_emit_byte (mb, CEE_CPBLK);			
		}
		else {
			int array_var, src_var, dst_var, index_var;
			guint32 label2, label3;

			array_var = mono_mb_add_local (mb, &mono_defaults.object_class->byval_arg);
			src_var = mono_mb_add_local (mb, &mono_defaults.int_class->byval_arg);
			dst_var = mono_mb_add_local (mb, &mono_defaults.int_class->byval_arg);

			/* set array_var */
			mono_mb_emit_ldloc (mb, 1);
			mono_mb_emit_byte (mb, CEE_LDIND_REF);
			mono_mb_emit_stloc (mb, array_var);
		
			/* save the old src pointer */
			mono_mb_emit_ldloc (mb, 0);
			mono_mb_emit_stloc (mb, src_var);
			/* save the old dst pointer */
			mono_mb_emit_ldloc (mb, 1);
			mono_mb_emit_stloc (mb, dst_var);

			/* Emit marshalling loop */
			index_var = mono_mb_add_local (mb, &mono_defaults.int_class->byval_arg);
			mono_mb_emit_byte (mb, CEE_LDC_I4_0);
			mono_mb_emit_stloc (mb, index_var);

			/* Loop header */
			label2 = mono_mb_get_label (mb);
			mono_mb_emit_ldloc (mb, index_var);
			mono_mb_emit_ldloc (mb, array_var);
			mono_mb_emit_byte (mb, CEE_LDLEN);
			label3 = mono_mb_emit_branch (mb, CEE_BGE);

			/* src is already set */

			/* Set dst */
			mono_mb_emit_ldloc (mb, array_var);
			mono_mb_emit_ldloc (mb, index_var);
			mono_mb_emit_op (mb, CEE_LDELEMA, eklass);
			mono_mb_emit_stloc (mb, 1);

			/* Do the conversion */
			emit_struct_conv (mb, eklass, TRUE);

			/* Loop footer */
			mono_mb_emit_add_to_local (mb, index_var, 1);

			mono_mb_emit_branch_label (mb, CEE_BR, label2);

			mono_mb_patch_branch (mb, label3);
		
			/* restore the old src pointer */
			mono_mb_emit_ldloc (mb, src_var);
			mono_mb_emit_stloc (mb, 0);
			/* restore the old dst pointer */
			mono_mb_emit_ldloc (mb, dst_var);
			mono_mb_emit_stloc (mb, 1);
		}
		break;
	}
	case MONO_MARSHAL_CONV_ARRAY_BYVALCHARARRAY: {
		MonoClass *eclass = mono_defaults.char_class;

		/* create a new array */
		mono_mb_emit_ldloc (mb, 1);
		mono_mb_emit_icon (mb, mspec->data.array_data.num_elem);
		mono_mb_emit_op (mb, CEE_NEWARR, eclass);	
		mono_mb_emit_byte (mb, CEE_STIND_REF);

		mono_mb_emit_ldloc (mb, 1);
		mono_mb_emit_byte (mb, CEE_LDIND_REF);
		mono_mb_emit_ldloc (mb, 0);
		mono_mb_emit_icon (mb, mspec->data.array_data.num_elem);
		mono_mb_emit_icall (mb, mono_byvalarray_to_byte_array);
		break;
	}
	case MONO_MARSHAL_CONV_STR_BYVALSTR: 
		if (mspec && mspec->native == MONO_NATIVE_BYVALTSTR && mspec->data.array_data.num_elem) {
			mono_mb_emit_ldloc (mb, 1);
			mono_mb_emit_ldloc (mb, 0);
			mono_mb_emit_icon (mb, mspec->data.array_data.num_elem);
			mono_mb_emit_icall (mb, mono_string_from_byvalstr);
		} else {
			mono_mb_emit_ldloc (mb, 1);
			mono_mb_emit_ldloc (mb, 0);
			mono_mb_emit_icall (mb, ves_icall_string_new_wrapper);
		}
		mono_mb_emit_byte (mb, CEE_STIND_REF);		
		break;
	case MONO_MARSHAL_CONV_STR_BYVALWSTR:
		if (mspec && mspec->native == MONO_NATIVE_BYVALTSTR && mspec->data.array_data.num_elem) {
			mono_mb_emit_ldloc (mb, 1);
			mono_mb_emit_ldloc (mb, 0);
			mono_mb_emit_icon (mb, mspec->data.array_data.num_elem);
			mono_mb_emit_icall (mb, mono_string_from_byvalwstr);
		} else {
			mono_mb_emit_ldloc (mb, 1);
			mono_mb_emit_ldloc (mb, 0);
			mono_mb_emit_icall (mb, ves_icall_mono_string_from_utf16);
		}
		mono_mb_emit_byte (mb, CEE_STIND_REF);		
		break;		
	case MONO_MARSHAL_CONV_STR_LPTSTR:
		mono_mb_emit_ldloc (mb, 1);
		mono_mb_emit_ldloc (mb, 0);
		mono_mb_emit_byte (mb, CEE_LDIND_I);
#ifdef TARGET_WIN32
		mono_mb_emit_icall (mb, ves_icall_mono_string_from_utf16);
#else
		mono_mb_emit_icall (mb, ves_icall_string_new_wrapper);
#endif
		mono_mb_emit_byte (mb, CEE_STIND_REF);	
		break;

		// In Mono historically LPSTR was treated as a UTF8STR
	case MONO_MARSHAL_CONV_STR_LPSTR:
	case MONO_MARSHAL_CONV_STR_UTF8STR:
		mono_mb_emit_ldloc (mb, 1);
		mono_mb_emit_ldloc (mb, 0);
		mono_mb_emit_byte (mb, CEE_LDIND_I);
		mono_mb_emit_icall (mb, ves_icall_string_new_wrapper);
		mono_mb_emit_byte (mb, CEE_STIND_REF);		
		break;
	case MONO_MARSHAL_CONV_STR_LPWSTR:
		mono_mb_emit_ldloc (mb, 1);
		mono_mb_emit_ldloc (mb, 0);
		mono_mb_emit_byte (mb, CEE_LDIND_I);
		mono_mb_emit_icall (mb, ves_icall_mono_string_from_utf16);
		mono_mb_emit_byte (mb, CEE_STIND_REF);
		break;
	case MONO_MARSHAL_CONV_OBJECT_STRUCT: {
		MonoClass *klass = mono_class_from_mono_type (type);
		int src_var, dst_var;

		src_var = mono_mb_add_local (mb, &mono_defaults.int_class->byval_arg);
		dst_var = mono_mb_add_local (mb, &mono_defaults.int_class->byval_arg);
		
		/* *dst = new object */
		mono_mb_emit_ldloc (mb, 1);
		mono_mb_emit_byte (mb, MONO_CUSTOM_PREFIX);
		mono_mb_emit_op (mb, CEE_MONO_NEWOBJ, klass);	
		mono_mb_emit_byte (mb, CEE_STIND_REF);
	
		/* save the old src pointer */
		mono_mb_emit_ldloc (mb, 0);
		mono_mb_emit_stloc (mb, src_var);
		/* save the old dst pointer */
		mono_mb_emit_ldloc (mb, 1);
		mono_mb_emit_stloc (mb, dst_var);

		/* dst = pointer to newly created object data */
		mono_mb_emit_ldloc (mb, 1);
		mono_mb_emit_byte (mb, CEE_LDIND_I);
		mono_mb_emit_icon (mb, sizeof (MonoObject));
		mono_mb_emit_byte (mb, CEE_ADD);
		mono_mb_emit_stloc (mb, 1); 

		emit_struct_conv (mb, klass, TRUE);
		
		/* restore the old src pointer */
		mono_mb_emit_ldloc (mb, src_var);
		mono_mb_emit_stloc (mb, 0);
		/* restore the old dst pointer */
		mono_mb_emit_ldloc (mb, dst_var);
		mono_mb_emit_stloc (mb, 1);
		break;
	}
	case MONO_MARSHAL_CONV_DEL_FTN: {
		MonoClass *klass = mono_class_from_mono_type (type);

		mono_mb_emit_ldloc (mb, 1);
		mono_mb_emit_byte (mb, MONO_CUSTOM_PREFIX);
		mono_mb_emit_op (mb, CEE_MONO_CLASSCONST, klass);
		mono_mb_emit_ldloc (mb, 0);
		mono_mb_emit_byte (mb, CEE_LDIND_I);
		mono_mb_emit_icall (mb, mono_ftnptr_to_delegate);
		mono_mb_emit_byte (mb, CEE_STIND_REF);
		break;
	}
	case MONO_MARSHAL_CONV_ARRAY_LPARRAY:
		g_error ("Structure field of type %s can't be marshalled as LPArray", mono_class_from_mono_type (type)->name);
		break;

#ifndef DISABLE_COM
	case MONO_MARSHAL_CONV_OBJECT_INTERFACE:
	case MONO_MARSHAL_CONV_OBJECT_IUNKNOWN:
	case MONO_MARSHAL_CONV_OBJECT_IDISPATCH:
		mono_cominterop_emit_ptr_to_object_conv (mb, type, conv, mspec);
		break;
#endif /* DISABLE_COM */

	case MONO_MARSHAL_CONV_SAFEHANDLE: {
		/*
		 * Passing SafeHandles as ref does not allow the unmanaged code
		 * to change the SafeHandle value.   If the value is changed,
		 * we should issue a diagnostic exception (NotSupportedException)
		 * that informs the user that changes to handles in unmanaged code
		 * is not supported. 
		 *
		 * Since we currently have no access to the original
		 * SafeHandle that was used during the marshalling,
		 * for now we just ignore this, and ignore/discard any
		 * changes that might have happened to the handle.
		 */
		break;
	}
		
	case MONO_MARSHAL_CONV_HANDLEREF: {
		/*
		 * Passing HandleRefs in a struct that is ref()ed does not 
		 * copy the values back to the HandleRef
		 */
		break;
	}
		
	case MONO_MARSHAL_CONV_STR_BSTR:
	case MONO_MARSHAL_CONV_STR_ANSIBSTR:
	case MONO_MARSHAL_CONV_STR_TBSTR:
	case MONO_MARSHAL_CONV_ARRAY_SAVEARRAY:
	default: {
		char *msg = g_strdup_printf ("marshaling conversion %d not implemented", conv);

		mono_mb_emit_exception_marshal_directive (mb, msg);
		break;
	}
	}
}

static gpointer
conv_to_icall (MonoMarshalConv conv, int *ind_store_type)
{
	int dummy;
	if (!ind_store_type)
		ind_store_type = &dummy;
	*ind_store_type = CEE_STIND_I;
	switch (conv) {
	case MONO_MARSHAL_CONV_STR_LPWSTR:
		return mono_marshal_string_to_utf16;		
	case MONO_MARSHAL_CONV_LPWSTR_STR:
		*ind_store_type = CEE_STIND_REF;
		return ves_icall_mono_string_from_utf16;
	case MONO_MARSHAL_CONV_LPTSTR_STR:
		*ind_store_type = CEE_STIND_REF;
		return ves_icall_string_new_wrapper;
	case MONO_MARSHAL_CONV_UTF8STR_STR:
	case MONO_MARSHAL_CONV_LPSTR_STR:
		*ind_store_type = CEE_STIND_REF;
		return ves_icall_string_new_wrapper;
	case MONO_MARSHAL_CONV_STR_LPTSTR:
#ifdef TARGET_WIN32
		return mono_marshal_string_to_utf16;
#else
		return mono_string_to_utf8str;
#endif
		// In Mono historically LPSTR was treated as a UTF8STR
	case MONO_MARSHAL_CONV_STR_UTF8STR:
	case MONO_MARSHAL_CONV_STR_LPSTR:
		return mono_string_to_utf8str;
	case MONO_MARSHAL_CONV_STR_BSTR:
		return mono_string_to_bstr;
	case MONO_MARSHAL_CONV_BSTR_STR:
		*ind_store_type = CEE_STIND_REF;
		return mono_string_from_bstr_icall;
	case MONO_MARSHAL_CONV_STR_TBSTR:
	case MONO_MARSHAL_CONV_STR_ANSIBSTR:
		return mono_string_to_ansibstr;
	case MONO_MARSHAL_CONV_SB_UTF8STR:
	case MONO_MARSHAL_CONV_SB_LPSTR:
		return mono_string_builder_to_utf8;
	case MONO_MARSHAL_CONV_SB_LPTSTR:
#ifdef TARGET_WIN32
		return mono_string_builder_to_utf16;
#else
		return mono_string_builder_to_utf8;
#endif
	case MONO_MARSHAL_CONV_SB_LPWSTR:
		return mono_string_builder_to_utf16;
	case MONO_MARSHAL_CONV_ARRAY_SAVEARRAY:
		return mono_array_to_savearray;
	case MONO_MARSHAL_CONV_ARRAY_LPARRAY:
		return mono_array_to_lparray;
	case MONO_MARSHAL_FREE_LPARRAY:
		return mono_free_lparray;
	case MONO_MARSHAL_CONV_DEL_FTN:
		return mono_delegate_to_ftnptr;
	case MONO_MARSHAL_CONV_FTN_DEL:
		*ind_store_type = CEE_STIND_REF;
		return mono_ftnptr_to_delegate;
	case MONO_MARSHAL_CONV_UTF8STR_SB:
	case MONO_MARSHAL_CONV_LPSTR_SB:
		*ind_store_type = CEE_STIND_REF;
		return mono_string_utf8_to_builder;
	case MONO_MARSHAL_CONV_LPTSTR_SB:
		*ind_store_type = CEE_STIND_REF;
#ifdef TARGET_WIN32
		return mono_string_utf16_to_builder;
#else
		return mono_string_utf8_to_builder;
#endif
	case MONO_MARSHAL_CONV_LPWSTR_SB:
		*ind_store_type = CEE_STIND_REF;
		return mono_string_utf16_to_builder;
	case MONO_MARSHAL_FREE_ARRAY:
		return mono_marshal_free_array;
	case MONO_MARSHAL_CONV_STR_BYVALSTR:
		return mono_string_to_byvalstr;
	case MONO_MARSHAL_CONV_STR_BYVALWSTR:
		return mono_string_to_byvalwstr;
	default:
		g_assert_not_reached ();
	}

	return NULL;
}

static void
emit_object_to_ptr_conv (MonoMethodBuilder *mb, MonoType *type, MonoMarshalConv conv, MonoMarshalSpec *mspec)
{
	int pos;
	int stind_op;

	switch (conv) {
	case MONO_MARSHAL_CONV_BOOL_I4:
		mono_mb_emit_ldloc (mb, 1);
		mono_mb_emit_ldloc (mb, 0);
		mono_mb_emit_byte (mb, CEE_LDIND_U1);
		mono_mb_emit_byte (mb, CEE_STIND_I4);
		break;
	case MONO_MARSHAL_CONV_BOOL_VARIANTBOOL:
		mono_mb_emit_ldloc (mb, 1);
		mono_mb_emit_ldloc (mb, 0);
		mono_mb_emit_byte (mb, CEE_LDIND_U1);
		mono_mb_emit_byte (mb, CEE_NEG);
		mono_mb_emit_byte (mb, CEE_STIND_I2);
		break;
	// In Mono historically LPSTR was treated as a UTF8STR
	case MONO_MARSHAL_CONV_STR_UTF8STR:
	case MONO_MARSHAL_CONV_STR_LPWSTR:
	case MONO_MARSHAL_CONV_STR_LPSTR:
	case MONO_MARSHAL_CONV_STR_LPTSTR:
	case MONO_MARSHAL_CONV_STR_BSTR:
	case MONO_MARSHAL_CONV_STR_ANSIBSTR:
	case MONO_MARSHAL_CONV_STR_TBSTR: {
		int pos;

		/* free space if free == true */
		mono_mb_emit_ldloc (mb, 2);
		pos = mono_mb_emit_short_branch (mb, CEE_BRFALSE_S);
		mono_mb_emit_ldloc (mb, 1);
		mono_mb_emit_byte (mb, CEE_LDIND_I);
		mono_mb_emit_icall (mb, g_free);
		mono_mb_patch_short_branch (mb, pos);

		mono_mb_emit_ldloc (mb, 1);
		mono_mb_emit_ldloc (mb, 0);
		mono_mb_emit_byte (mb, CEE_LDIND_REF);
		mono_mb_emit_icall (mb, conv_to_icall (conv, &stind_op));
		mono_mb_emit_byte (mb, stind_op);
		break;
	}
	case MONO_MARSHAL_CONV_ARRAY_SAVEARRAY:
	case MONO_MARSHAL_CONV_ARRAY_LPARRAY:
	case MONO_MARSHAL_CONV_DEL_FTN:
		mono_mb_emit_ldloc (mb, 1);
		mono_mb_emit_ldloc (mb, 0);
		mono_mb_emit_byte (mb, CEE_LDIND_REF);
		mono_mb_emit_icall (mb, conv_to_icall (conv, &stind_op));
		mono_mb_emit_byte (mb, stind_op);
		break;
	case MONO_MARSHAL_CONV_STR_BYVALSTR: 
	case MONO_MARSHAL_CONV_STR_BYVALWSTR: {
		g_assert (mspec);

		mono_mb_emit_ldloc (mb, 1); /* dst */
		mono_mb_emit_ldloc (mb, 0);	
		mono_mb_emit_byte (mb, CEE_LDIND_REF); /* src String */
		mono_mb_emit_icon (mb, mspec->data.array_data.num_elem);
		mono_mb_emit_icall (mb, conv_to_icall (conv, NULL));
		break;
	}
	case MONO_MARSHAL_CONV_ARRAY_BYVALARRAY: {
		MonoClass *eklass = NULL;
		int esize;

		if (type->type == MONO_TYPE_SZARRAY) {
			eklass = type->data.klass;
		} else {
			g_assert_not_reached ();
		}

		if (eklass->valuetype)
			esize = mono_class_native_size (eklass, NULL);
		else
			esize = sizeof (gpointer);

		mono_mb_emit_ldloc (mb, 0);
		mono_mb_emit_byte (mb, CEE_LDIND_REF);
		pos = mono_mb_emit_branch (mb, CEE_BRFALSE);

		if (eklass->blittable) {
			mono_mb_emit_ldloc (mb, 1);
			mono_mb_emit_ldloc (mb, 0);	
			mono_mb_emit_byte (mb, CEE_LDIND_REF);	
			mono_mb_emit_ldflda (mb, MONO_STRUCT_OFFSET (MonoArray, vector));
			mono_mb_emit_icon (mb, mspec->data.array_data.num_elem * esize);
			mono_mb_emit_byte (mb, CEE_PREFIX1);
			mono_mb_emit_byte (mb, CEE_CPBLK);			
		} else {
			int array_var, src_var, dst_var, index_var;
			guint32 label2, label3;

			array_var = mono_mb_add_local (mb, &mono_defaults.object_class->byval_arg);
			src_var = mono_mb_add_local (mb, &mono_defaults.int_class->byval_arg);
			dst_var = mono_mb_add_local (mb, &mono_defaults.int_class->byval_arg);

			/* set array_var */
			mono_mb_emit_ldloc (mb, 0);	
			mono_mb_emit_byte (mb, CEE_LDIND_REF);
			mono_mb_emit_stloc (mb, array_var);

			/* save the old src pointer */
			mono_mb_emit_ldloc (mb, 0);
			mono_mb_emit_stloc (mb, src_var);
			/* save the old dst pointer */
			mono_mb_emit_ldloc (mb, 1);
			mono_mb_emit_stloc (mb, dst_var);

			/* Emit marshalling loop */
			index_var = mono_mb_add_local (mb, &mono_defaults.int_class->byval_arg);
			mono_mb_emit_byte (mb, CEE_LDC_I4_0);
			mono_mb_emit_stloc (mb, index_var);

			/* Loop header */
			label2 = mono_mb_get_label (mb);
			mono_mb_emit_ldloc (mb, index_var);
			mono_mb_emit_ldloc (mb, array_var);
			mono_mb_emit_byte (mb, CEE_LDLEN);
			label3 = mono_mb_emit_branch (mb, CEE_BGE);

			/* Set src */
			mono_mb_emit_ldloc (mb, array_var);
			mono_mb_emit_ldloc (mb, index_var);
			mono_mb_emit_op (mb, CEE_LDELEMA, eklass);
			mono_mb_emit_stloc (mb, 0);

			/* dst is already set */

			/* Do the conversion */
			emit_struct_conv (mb, eklass, FALSE);

			/* Loop footer */
			mono_mb_emit_add_to_local (mb, index_var, 1);

			mono_mb_emit_branch_label (mb, CEE_BR, label2);

			mono_mb_patch_branch (mb, label3);
		
			/* restore the old src pointer */
			mono_mb_emit_ldloc (mb, src_var);
			mono_mb_emit_stloc (mb, 0);
			/* restore the old dst pointer */
			mono_mb_emit_ldloc (mb, dst_var);
			mono_mb_emit_stloc (mb, 1);
		}

		mono_mb_patch_branch (mb, pos);
		break;
	}
	case MONO_MARSHAL_CONV_ARRAY_BYVALCHARARRAY: {
		mono_mb_emit_ldloc (mb, 0);
		mono_mb_emit_byte (mb, CEE_LDIND_REF);
		pos = mono_mb_emit_short_branch (mb, CEE_BRFALSE_S);

		mono_mb_emit_ldloc (mb, 1);
		mono_mb_emit_ldloc (mb, 0);	
		mono_mb_emit_byte (mb, CEE_LDIND_REF);
		mono_mb_emit_icon (mb, mspec->data.array_data.num_elem);
		mono_mb_emit_icall (mb, mono_array_to_byte_byvalarray);
		mono_mb_patch_short_branch (mb, pos);
		break;
	}
	case MONO_MARSHAL_CONV_OBJECT_STRUCT: {
		int src_var, dst_var;

		src_var = mono_mb_add_local (mb, &mono_defaults.int_class->byval_arg);
		dst_var = mono_mb_add_local (mb, &mono_defaults.int_class->byval_arg);
		
		mono_mb_emit_ldloc (mb, 0);
		mono_mb_emit_byte (mb, CEE_LDIND_I);
		pos = mono_mb_emit_branch (mb, CEE_BRFALSE);
		
		/* save the old src pointer */
		mono_mb_emit_ldloc (mb, 0);
		mono_mb_emit_stloc (mb, src_var);
		/* save the old dst pointer */
		mono_mb_emit_ldloc (mb, 1);
		mono_mb_emit_stloc (mb, dst_var);

		/* src = pointer to object data */
		mono_mb_emit_ldloc (mb, 0);
		mono_mb_emit_byte (mb, CEE_LDIND_I);		
		mono_mb_emit_icon (mb, sizeof (MonoObject));
		mono_mb_emit_byte (mb, CEE_ADD);
		mono_mb_emit_stloc (mb, 0); 

		emit_struct_conv (mb, mono_class_from_mono_type (type), FALSE);
		
		/* restore the old src pointer */
		mono_mb_emit_ldloc (mb, src_var);
		mono_mb_emit_stloc (mb, 0);
		/* restore the old dst pointer */
		mono_mb_emit_ldloc (mb, dst_var);
		mono_mb_emit_stloc (mb, 1);

		mono_mb_patch_branch (mb, pos);
		break;
	}

#ifndef DISABLE_COM
	case MONO_MARSHAL_CONV_OBJECT_INTERFACE:
	case MONO_MARSHAL_CONV_OBJECT_IDISPATCH:
	case MONO_MARSHAL_CONV_OBJECT_IUNKNOWN:
		mono_cominterop_emit_object_to_ptr_conv (mb, type, conv, mspec);
		break;
#endif /* DISABLE_COM */

	case MONO_MARSHAL_CONV_SAFEHANDLE: {
		int pos;
		
		mono_mb_emit_ldloc (mb, 0);
		mono_mb_emit_byte (mb, CEE_LDIND_I);
		pos = mono_mb_emit_branch (mb, CEE_BRTRUE);
		mono_mb_emit_exception (mb, "ArgumentNullException", NULL);
		mono_mb_patch_branch (mb, pos);
		
		/* Pull the handle field from SafeHandle */
		mono_mb_emit_ldloc (mb, 1);
		mono_mb_emit_ldloc (mb, 0);
		mono_mb_emit_byte (mb, CEE_LDIND_I);
		mono_mb_emit_ldflda (mb, MONO_STRUCT_OFFSET (MonoSafeHandle, handle));
		mono_mb_emit_byte (mb, CEE_LDIND_I);
		mono_mb_emit_byte (mb, CEE_STIND_I);
		break;
	}

	case MONO_MARSHAL_CONV_HANDLEREF: {
		mono_mb_emit_ldloc (mb, 1);
		mono_mb_emit_ldloc (mb, 0);
		mono_mb_emit_icon (mb, MONO_STRUCT_OFFSET (MonoHandleRef, handle));
		mono_mb_emit_byte (mb, CEE_ADD);
		mono_mb_emit_byte (mb, CEE_LDIND_I);
		mono_mb_emit_byte (mb, CEE_STIND_I);
		break;
	}
		
	default: {
		g_error ("marshalling conversion %d not implemented", conv);
	}
	}
}

static int
offset_of_first_nonstatic_field (MonoClass *klass)
{
	int i;
	int fcount = mono_class_get_field_count (klass);
	mono_class_setup_fields (klass);
	for (i = 0; i < fcount; i++) {
		if (!(klass->fields[i].type->attrs & FIELD_ATTRIBUTE_STATIC) && !mono_field_is_deleted (&klass->fields[i]))
			return klass->fields[i].offset - sizeof (MonoObject);
	}

	return 0;
}

static gboolean
get_fixed_buffer_attr (MonoClassField *field, MonoType **out_etype, int *out_len)
{
	ERROR_DECL (error);
	MonoCustomAttrInfo *cinfo;
	MonoCustomAttrEntry *attr;
	int aindex;

	cinfo = mono_custom_attrs_from_field_checked (field->parent, field, error);
	if (!is_ok (error))
		return FALSE;
	attr = NULL;
	if (cinfo) {
		for (aindex = 0; aindex < cinfo->num_attrs; ++aindex) {
			MonoClass *ctor_class = cinfo->attrs [aindex].ctor->klass;
			if (mono_class_has_parent (ctor_class, mono_class_get_fixed_buffer_attribute_class ())) {
				attr = &cinfo->attrs [aindex];
				break;
			}
		}
	}
	if (attr) {
		MonoArray *typed_args, *named_args;
		CattrNamedArg *arginfo;
		MonoObject *o;

		mono_reflection_create_custom_attr_data_args (mono_defaults.corlib, attr->ctor, attr->data, attr->data_size, &typed_args, &named_args, &arginfo, error);
		if (!is_ok (error))
			return FALSE;
		g_assert (mono_array_length (typed_args) == 2);

		/* typed args */
		o = mono_array_get (typed_args, MonoObject*, 0);
		*out_etype = monotype_cast (o)->type;
		o = mono_array_get (typed_args, MonoObject*, 1);
		g_assert (o->vtable->klass == mono_defaults.int32_class);
		*out_len = *(gint32*)mono_object_unbox (o);
		g_free (arginfo);
	}
	if (cinfo && !cinfo->cached)
		mono_custom_attrs_free (cinfo);
	return attr != NULL;
}

static void
emit_fixed_buf_conv (MonoMethodBuilder *mb, MonoType *type, MonoType *etype, int len, gboolean to_object, int *out_usize)
{
	MonoClass *klass = mono_class_from_mono_type (type);
	MonoClass *eklass = mono_class_from_mono_type (etype);
	int esize;

	esize = mono_class_native_size (eklass, NULL);

	MonoMarshalNative string_encoding = klass->unicode ? MONO_NATIVE_LPWSTR : MONO_NATIVE_LPSTR;
	int usize = mono_class_value_size (eklass, NULL);
	int msize = mono_class_value_size (eklass, NULL);

	//printf ("FIXED: %s %d %d\n", mono_type_full_name (type), eklass->blittable, string_encoding);

	if (eklass->blittable) {
		/* copy the elements */
		mono_mb_emit_ldloc (mb, 1);
		mono_mb_emit_ldloc (mb, 0);
		mono_mb_emit_icon (mb, len * esize);
		mono_mb_emit_byte (mb, CEE_PREFIX1);
		mono_mb_emit_byte (mb, CEE_CPBLK);
	} else {
		int index_var;
		guint32 label2, label3;

		/* Emit marshalling loop */
		index_var = mono_mb_add_local (mb, &mono_defaults.int_class->byval_arg);
		mono_mb_emit_byte (mb, CEE_LDC_I4_0);
		mono_mb_emit_stloc (mb, index_var);

		/* Loop header */
		label2 = mono_mb_get_label (mb);
		mono_mb_emit_ldloc (mb, index_var);
		mono_mb_emit_icon (mb, len);
		label3 = mono_mb_emit_branch (mb, CEE_BGE);

		/* src/dst is already set */

		/* Do the conversion */
		MonoTypeEnum t = etype->type;
		switch (t) {
		case MONO_TYPE_I4:
		case MONO_TYPE_U4:
		case MONO_TYPE_I1:
		case MONO_TYPE_U1:
		case MONO_TYPE_BOOLEAN:
		case MONO_TYPE_I2:
		case MONO_TYPE_U2:
		case MONO_TYPE_CHAR:
		case MONO_TYPE_I8:
		case MONO_TYPE_U8:
		case MONO_TYPE_PTR:
		case MONO_TYPE_R4:
		case MONO_TYPE_R8:
			mono_mb_emit_ldloc (mb, 1);
			mono_mb_emit_ldloc (mb, 0);
			if (t == MONO_TYPE_CHAR && string_encoding != MONO_NATIVE_LPWSTR) {
				if (to_object) {
					mono_mb_emit_byte (mb, CEE_LDIND_U1);
					mono_mb_emit_byte (mb, CEE_STIND_I2);
				} else {
					mono_mb_emit_byte (mb, CEE_LDIND_U2);
					mono_mb_emit_byte (mb, CEE_STIND_I1);
				}
				usize = 1;
			} else {
				mono_mb_emit_byte (mb, mono_type_to_ldind (etype));
				mono_mb_emit_byte (mb, mono_type_to_stind (etype));
			}
			break;
		default:
			g_assert_not_reached ();
			break;
		}

		if (to_object) {
			mono_mb_emit_add_to_local (mb, 0, usize);
			mono_mb_emit_add_to_local (mb, 1, msize);
		} else {
			mono_mb_emit_add_to_local (mb, 0, msize);
			mono_mb_emit_add_to_local (mb, 1, usize);
		}

		/* Loop footer */
		mono_mb_emit_add_to_local (mb, index_var, 1);

		mono_mb_emit_branch_label (mb, CEE_BR, label2);

		mono_mb_patch_branch (mb, label3);
	}

	*out_usize = usize * len;
}

static void
emit_struct_conv_full (MonoMethodBuilder *mb, MonoClass *klass, gboolean to_object,
						int offset_of_first_child_field, MonoMarshalNative string_encoding)
{
	MonoMarshalType *info;
	int i;

	if (klass->parent)
		emit_struct_conv_full (mb, klass->parent, to_object, offset_of_first_nonstatic_field (klass), string_encoding);

	info = mono_marshal_load_type_info (klass);

	if (info->native_size == 0)
		return;

	if (klass->blittable) {
		int usize = mono_class_value_size (klass, NULL);
		g_assert (usize == info->native_size);
		mono_mb_emit_ldloc (mb, 1);
		mono_mb_emit_ldloc (mb, 0);
		mono_mb_emit_icon (mb, usize);
		mono_mb_emit_byte (mb, CEE_PREFIX1);
		mono_mb_emit_byte (mb, CEE_CPBLK);

		if (to_object) {
			mono_mb_emit_add_to_local (mb, 0, usize);
			mono_mb_emit_add_to_local (mb, 1, offset_of_first_child_field);
		} else {
			mono_mb_emit_add_to_local (mb, 0, offset_of_first_child_field);
			mono_mb_emit_add_to_local (mb, 1, usize);
		}
		return;
	}

	if (klass != mono_class_try_get_safehandle_class ()) {
		if (mono_class_is_auto_layout (klass)) {
			char *msg = g_strdup_printf ("Type %s which is passed to unmanaged code must have a StructLayout attribute.",
										 mono_type_full_name (&klass->byval_arg));
			mono_mb_emit_exception_marshal_directive (mb, msg);
			return;
		}
	}

	for (i = 0; i < info->num_fields; i++) {
		MonoMarshalNative ntype;
		MonoMarshalConv conv;
		MonoType *ftype = info->fields [i].field->type;
		int msize = 0;
		int usize = 0;
		gboolean last_field = i < (info->num_fields -1) ? 0 : 1;

		if (ftype->attrs & FIELD_ATTRIBUTE_STATIC)
			continue;

		ntype = (MonoMarshalNative)mono_type_to_unmanaged (ftype, info->fields [i].mspec, TRUE, klass->unicode, &conv);

		if (last_field) {
			msize = klass->instance_size - info->fields [i].field->offset;
			usize = info->native_size - info->fields [i].offset;
		} else {
			msize = info->fields [i + 1].field->offset - info->fields [i].field->offset;
			usize = info->fields [i + 1].offset - info->fields [i].offset;
		}

		if (klass != mono_class_try_get_safehandle_class ()){
			/* 
			 * FIXME: Should really check for usize==0 and msize>0, but we apply 
			 * the layout to the managed structure as well.
			 */
			
			if (mono_class_is_explicit_layout (klass) && (usize == 0)) {
				if (MONO_TYPE_IS_REFERENCE (info->fields [i].field->type) ||
				    ((!last_field && MONO_TYPE_IS_REFERENCE (info->fields [i + 1].field->type))))
					g_error ("Type %s which has an [ExplicitLayout] attribute cannot have a "
						 "reference field at the same offset as another field.",
						 mono_type_full_name (&klass->byval_arg));
			}
		}
		
		switch (conv) {
		case MONO_MARSHAL_CONV_NONE: {
			int t;

			//XXX a byref field!?!? that's not allowed! and worse, it might miss a WB
			g_assert (!ftype->byref);
			if (ftype->type == MONO_TYPE_I || ftype->type == MONO_TYPE_U) {
				mono_mb_emit_ldloc (mb, 1);
				mono_mb_emit_ldloc (mb, 0);
				mono_mb_emit_byte (mb, CEE_LDIND_I);
				mono_mb_emit_byte (mb, CEE_STIND_I);
				break;
			}

		handle_enum:
			t = ftype->type;
			switch (t) {
			case MONO_TYPE_I4:
			case MONO_TYPE_U4:
			case MONO_TYPE_I1:
			case MONO_TYPE_U1:
			case MONO_TYPE_BOOLEAN:
			case MONO_TYPE_I2:
			case MONO_TYPE_U2:
			case MONO_TYPE_CHAR:
			case MONO_TYPE_I8:
			case MONO_TYPE_U8:
			case MONO_TYPE_PTR:
			case MONO_TYPE_R4:
			case MONO_TYPE_R8:
				mono_mb_emit_ldloc (mb, 1);
				mono_mb_emit_ldloc (mb, 0);
				if (t == MONO_TYPE_CHAR && ntype == MONO_NATIVE_U1 && string_encoding != MONO_NATIVE_LPWSTR) {
					if (to_object) {
						mono_mb_emit_byte (mb, CEE_LDIND_U1);
						mono_mb_emit_byte (mb, CEE_STIND_I2);
					} else {
						mono_mb_emit_byte (mb, CEE_LDIND_U2);
						mono_mb_emit_byte (mb, CEE_STIND_I1);
					}
				} else {
					mono_mb_emit_byte (mb, mono_type_to_ldind (ftype));
					mono_mb_emit_byte (mb, mono_type_to_stind (ftype));
				}
				break;
			case MONO_TYPE_VALUETYPE: {
				int src_var, dst_var;
				MonoType *etype;
				int len;

				if (ftype->data.klass->enumtype) {
					ftype = mono_class_enum_basetype (ftype->data.klass);
					goto handle_enum;
				}

				src_var = mono_mb_add_local (mb, &mono_defaults.int_class->byval_arg);
				dst_var = mono_mb_add_local (mb, &mono_defaults.int_class->byval_arg);
	
				/* save the old src pointer */
				mono_mb_emit_ldloc (mb, 0);
				mono_mb_emit_stloc (mb, src_var);
				/* save the old dst pointer */
				mono_mb_emit_ldloc (mb, 1);
				mono_mb_emit_stloc (mb, dst_var);

				if (get_fixed_buffer_attr (info->fields [i].field, &etype, &len)) {
					emit_fixed_buf_conv (mb, ftype, etype, len, to_object, &usize);
				} else {
					emit_struct_conv (mb, ftype->data.klass, to_object);
				}

				/* restore the old src pointer */
				mono_mb_emit_ldloc (mb, src_var);
				mono_mb_emit_stloc (mb, 0);
				/* restore the old dst pointer */
				mono_mb_emit_ldloc (mb, dst_var);
				mono_mb_emit_stloc (mb, 1);
				break;
			}
			case MONO_TYPE_OBJECT: {
#ifndef DISABLE_COM
				if (to_object) {
					static MonoMethod *variant_clear = NULL;
					static MonoMethod *get_object_for_native_variant = NULL;

					if (!variant_clear)
						variant_clear = mono_class_get_method_from_name (mono_class_get_variant_class (), "Clear", 0);
					if (!get_object_for_native_variant)
						get_object_for_native_variant = mono_class_get_method_from_name (mono_defaults.marshal_class, "GetObjectForNativeVariant", 1);
					mono_mb_emit_ldloc (mb, 1);
					mono_mb_emit_ldloc (mb, 0);
					mono_mb_emit_managed_call (mb, get_object_for_native_variant, NULL);
					mono_mb_emit_byte (mb, CEE_STIND_REF);

					mono_mb_emit_ldloc (mb, 0);
					mono_mb_emit_managed_call (mb, variant_clear, NULL);
				}
				else {
					static MonoMethod *get_native_variant_for_object = NULL;

					if (!get_native_variant_for_object)
						get_native_variant_for_object = mono_class_get_method_from_name (mono_defaults.marshal_class, "GetNativeVariantForObject", 2);

					mono_mb_emit_ldloc (mb, 0);
					mono_mb_emit_byte(mb, CEE_LDIND_REF);
					mono_mb_emit_ldloc (mb, 1);
					mono_mb_emit_managed_call (mb, get_native_variant_for_object, NULL);
				}
#else
				char *msg = g_strdup_printf ("COM support was disabled at compilation time.");
				mono_mb_emit_exception_marshal_directive (mb, msg);
#endif
				break;
			}

			default: 
				g_warning ("marshaling type %02x not implemented", ftype->type);
				g_assert_not_reached ();
			}
			break;
		}
		default: {
			int src_var, dst_var;

			src_var = mono_mb_add_local (mb, &mono_defaults.int_class->byval_arg);
			dst_var = mono_mb_add_local (mb, &mono_defaults.int_class->byval_arg);

			/* save the old src pointer */
			mono_mb_emit_ldloc (mb, 0);
			mono_mb_emit_stloc (mb, src_var);
			/* save the old dst pointer */
			mono_mb_emit_ldloc (mb, 1);
			mono_mb_emit_stloc (mb, dst_var);

			if (to_object) 
				emit_ptr_to_object_conv (mb, ftype, conv, info->fields [i].mspec);
			else
				emit_object_to_ptr_conv (mb, ftype, conv, info->fields [i].mspec);

			/* restore the old src pointer */
			mono_mb_emit_ldloc (mb, src_var);
			mono_mb_emit_stloc (mb, 0);
			/* restore the old dst pointer */
			mono_mb_emit_ldloc (mb, dst_var);
			mono_mb_emit_stloc (mb, 1);
		}
		}

		if (to_object) {
			mono_mb_emit_add_to_local (mb, 0, usize);
			mono_mb_emit_add_to_local (mb, 1, msize);
		} else {
			mono_mb_emit_add_to_local (mb, 0, msize);
			mono_mb_emit_add_to_local (mb, 1, usize);
		}				
	}
}

static void
emit_struct_conv (MonoMethodBuilder *mb, MonoClass *klass, gboolean to_object)
{
	emit_struct_conv_full (mb, klass, to_object, 0, (MonoMarshalNative)-1);
}

static void
emit_struct_free (MonoMethodBuilder *mb, MonoClass *klass, int struct_var)
{
	/* Call DestroyStructure */
	/* FIXME: Only do this if needed */
	mono_mb_emit_byte (mb, MONO_CUSTOM_PREFIX);
	mono_mb_emit_op (mb, CEE_MONO_CLASSCONST, klass);
	mono_mb_emit_ldloc (mb, struct_var);
	mono_mb_emit_icall (mb, mono_struct_delete_old);
}

static void
emit_thread_interrupt_checkpoint_call (MonoMethodBuilder *mb, gpointer checkpoint_func)
{
	int pos_noabort, pos_noex;

	mono_mb_emit_byte (mb, MONO_CUSTOM_PREFIX);
	mono_mb_emit_byte (mb, CEE_MONO_LDPTR_INT_REQ_FLAG);
	mono_mb_emit_byte (mb, CEE_LDIND_U4);
	pos_noabort = mono_mb_emit_branch (mb, CEE_BRFALSE);

	mono_mb_emit_byte (mb, MONO_CUSTOM_PREFIX);
	mono_mb_emit_byte (mb, CEE_MONO_NOT_TAKEN);

	mono_mb_emit_icall (mb, checkpoint_func);
	/* Throw the exception returned by the checkpoint function, if any */
	mono_mb_emit_byte (mb, CEE_DUP);
	pos_noex = mono_mb_emit_branch (mb, CEE_BRFALSE);
	mono_mb_emit_byte (mb, CEE_THROW);
	mono_mb_patch_branch (mb, pos_noex);
	mono_mb_emit_byte (mb, CEE_POP);
	
	mono_mb_patch_branch (mb, pos_noabort);
}

static void
emit_thread_interrupt_checkpoint (MonoMethodBuilder *mb)
{
	if (strstr (mb->name, "mono_thread_interruption_checkpoint"))
		return;
	
	emit_thread_interrupt_checkpoint_call (mb, mono_thread_interruption_checkpoint);
}

static void
emit_thread_force_interrupt_checkpoint (MonoMethodBuilder *mb)
{
	emit_thread_interrupt_checkpoint_call (mb, mono_thread_force_interruption_checkpoint_noraise);
}

void
mono_marshal_emit_thread_interrupt_checkpoint (MonoMethodBuilder *mb)
{
	emit_thread_interrupt_checkpoint (mb);
}

void
mono_marshal_emit_thread_force_interrupt_checkpoint (MonoMethodBuilder *mb)
{
	emit_thread_force_interrupt_checkpoint (mb);
}

int
mono_mb_emit_save_args (MonoMethodBuilder *mb, MonoMethodSignature *sig, gboolean save_this)
{
	int i, params_var, tmp_var;

	/* allocate local (pointer) *params[] */
	params_var = mono_mb_add_local (mb, &mono_defaults.int_class->byval_arg);
	/* allocate local (pointer) tmp */
	tmp_var = mono_mb_add_local (mb, &mono_defaults.int_class->byval_arg);

	/* alloate space on stack to store an array of pointers to the arguments */
	mono_mb_emit_icon (mb, sizeof (gpointer) * (sig->param_count + 1));
	mono_mb_emit_byte (mb, CEE_PREFIX1);
	mono_mb_emit_byte (mb, CEE_LOCALLOC);
	mono_mb_emit_stloc (mb, params_var);

	/* tmp = params */
	mono_mb_emit_ldloc (mb, params_var);
	mono_mb_emit_stloc (mb, tmp_var);

	if (save_this && sig->hasthis) {
		mono_mb_emit_ldloc (mb, tmp_var);
		mono_mb_emit_ldarg_addr (mb, 0);
		mono_mb_emit_byte (mb, CEE_STIND_I);
		/* tmp = tmp + sizeof (gpointer) */
		if (sig->param_count)
			mono_mb_emit_add_to_local (mb, tmp_var, sizeof (gpointer));

	}

	for (i = 0; i < sig->param_count; i++) {
		mono_mb_emit_ldloc (mb, tmp_var);
		mono_mb_emit_ldarg_addr (mb, i + sig->hasthis);
		mono_mb_emit_byte (mb, CEE_STIND_I);
		/* tmp = tmp + sizeof (gpointer) */
		if (i < (sig->param_count - 1))
			mono_mb_emit_add_to_local (mb, tmp_var, sizeof (gpointer));
	}

	return params_var;
}


void
mono_mb_emit_restore_result (MonoMethodBuilder *mb, MonoType *return_type)
{
	MonoType *t = mono_type_get_underlying_type (return_type);

	if (return_type->byref)
		return_type = &mono_defaults.int_class->byval_arg;

	switch (t->type) {
	case MONO_TYPE_VOID:
		g_assert_not_reached ();
		break;
	case MONO_TYPE_PTR:
	case MONO_TYPE_STRING:
	case MONO_TYPE_CLASS: 
	case MONO_TYPE_OBJECT: 
	case MONO_TYPE_ARRAY: 
	case MONO_TYPE_SZARRAY: 
		/* nothing to do */
		break;
	case MONO_TYPE_U1:
	case MONO_TYPE_BOOLEAN:
	case MONO_TYPE_I1:
	case MONO_TYPE_U2:
	case MONO_TYPE_CHAR:
	case MONO_TYPE_I2:
	case MONO_TYPE_I:
	case MONO_TYPE_U:
	case MONO_TYPE_I4:
	case MONO_TYPE_U4:
	case MONO_TYPE_U8:
	case MONO_TYPE_I8:
	case MONO_TYPE_R4:
	case MONO_TYPE_R8:
		mono_mb_emit_op (mb, CEE_UNBOX, mono_class_from_mono_type (return_type));
		mono_mb_emit_byte (mb, mono_type_to_ldind (return_type));
		break;
	case MONO_TYPE_GENERICINST:
		if (!mono_type_generic_inst_is_valuetype (t))
			break;
		/* fall through */
	case MONO_TYPE_VALUETYPE: {
		MonoClass *klass = mono_class_from_mono_type (return_type);
		mono_mb_emit_op (mb, CEE_UNBOX, klass);
		mono_mb_emit_op (mb, CEE_LDOBJ, klass);
		break;
	}
	case MONO_TYPE_VAR:
	case MONO_TYPE_MVAR: {
		MonoClass *klass = mono_class_from_mono_type (return_type);
		mono_mb_emit_op (mb, CEE_UNBOX_ANY, klass);
		break;
	}
	default:
		g_warning ("type 0x%x not handled", return_type->type);
		g_assert_not_reached ();
	}

	mono_mb_emit_byte (mb, CEE_RET);
}

/*
 * emit_invoke_call:
 *
 *   Emit the call to the wrapper method from a runtime invoke wrapper.
 */
static void
emit_invoke_call (MonoMethodBuilder *mb, MonoMethod *method,
				  MonoMethodSignature *sig, MonoMethodSignature *callsig,
				  int loc_res,
				  gboolean virtual_, gboolean need_direct_wrapper)
{
	static MonoString *string_dummy = NULL;
	int i;
	int *tmp_nullable_locals;
	gboolean void_ret = FALSE;
	gboolean string_ctor = method && method->string_ctor;

	/* to make it work with our special string constructors */
	if (!string_dummy) {
		ERROR_DECL (error);
		MONO_GC_REGISTER_ROOT_SINGLE (string_dummy, MONO_ROOT_SOURCE_MARSHAL, NULL, "Marshal Dummy String");
		string_dummy = mono_string_new_checked (mono_get_root_domain (), "dummy", error);
		mono_error_assert_ok (error);
	}

	if (virtual_) {
		g_assert (sig->hasthis);
		g_assert (method->flags & METHOD_ATTRIBUTE_VIRTUAL);
	}

	if (sig->hasthis) {
		if (string_ctor) {
			if (mono_gc_is_moving ()) {
				mono_mb_emit_ptr (mb, &string_dummy);
				mono_mb_emit_byte (mb, CEE_LDIND_REF);
			} else {
				mono_mb_emit_ptr (mb, string_dummy);
			}
		} else {
			mono_mb_emit_ldarg (mb, 0);
		}
	}

	tmp_nullable_locals = g_new0 (int, sig->param_count);

	for (i = 0; i < sig->param_count; i++) {
		MonoType *t = sig->params [i];
		int type;

		mono_mb_emit_ldarg (mb, 1);
		if (i) {
			mono_mb_emit_icon (mb, sizeof (gpointer) * i);
			mono_mb_emit_byte (mb, CEE_ADD);
		}

		if (t->byref) {
			mono_mb_emit_byte (mb, CEE_LDIND_I);
			/* A Nullable<T> type don't have a boxed form, it's either null or a boxed T.
			 * So to make this work we unbox it to a local variablee and push a reference to that.
			 */
			if (t->type == MONO_TYPE_GENERICINST && mono_class_is_nullable (mono_class_from_mono_type (t))) {
				tmp_nullable_locals [i] = mono_mb_add_local (mb, &mono_class_from_mono_type (t)->byval_arg);

				mono_mb_emit_op (mb, CEE_UNBOX_ANY, mono_class_from_mono_type (t));
				mono_mb_emit_stloc (mb, tmp_nullable_locals [i]);
				mono_mb_emit_ldloc_addr (mb, tmp_nullable_locals [i]);
			}
			continue;
		}

		/*FIXME 'this doesn't handle generic enums. Shouldn't we?*/
		type = sig->params [i]->type;
handle_enum:
		switch (type) {
		case MONO_TYPE_I1:
		case MONO_TYPE_BOOLEAN:
		case MONO_TYPE_U1:
		case MONO_TYPE_I2:
		case MONO_TYPE_U2:
		case MONO_TYPE_CHAR:
		case MONO_TYPE_I:
		case MONO_TYPE_U:
		case MONO_TYPE_I4:
		case MONO_TYPE_U4:
		case MONO_TYPE_R4:
		case MONO_TYPE_R8:
		case MONO_TYPE_I8:
		case MONO_TYPE_U8:
			mono_mb_emit_byte (mb, CEE_LDIND_I);
			mono_mb_emit_byte (mb, mono_type_to_ldind (sig->params [i]));
			break;
		case MONO_TYPE_STRING:
		case MONO_TYPE_CLASS:  
		case MONO_TYPE_ARRAY:
		case MONO_TYPE_PTR:
		case MONO_TYPE_SZARRAY:
		case MONO_TYPE_OBJECT:
			mono_mb_emit_byte (mb, mono_type_to_ldind (sig->params [i]));
			break;
		case MONO_TYPE_GENERICINST:
			if (!mono_type_generic_inst_is_valuetype (sig->params [i])) {
				mono_mb_emit_byte (mb, mono_type_to_ldind (sig->params [i]));
				break;
			}

			/* fall through */
		case MONO_TYPE_VALUETYPE:
			if (type == MONO_TYPE_VALUETYPE && t->data.klass->enumtype) {
				type = mono_class_enum_basetype (t->data.klass)->type;
				goto handle_enum;
			}
			mono_mb_emit_byte (mb, CEE_LDIND_I);
			if (mono_class_is_nullable (mono_class_from_mono_type (sig->params [i]))) {
				/* Need to convert a boxed vtype to an mp to a Nullable struct */
				mono_mb_emit_op (mb, CEE_UNBOX, mono_class_from_mono_type (sig->params [i]));
				mono_mb_emit_op (mb, CEE_LDOBJ, mono_class_from_mono_type (sig->params [i]));
			} else {
				mono_mb_emit_op (mb, CEE_LDOBJ, mono_class_from_mono_type (sig->params [i]));
			}
			break;
		default:
			g_assert_not_reached ();
		}
	}
	
	if (virtual_) {
		mono_mb_emit_op (mb, CEE_CALLVIRT, method);
	} else if (need_direct_wrapper) {
		mono_mb_emit_op (mb, CEE_CALL, method);
	} else {
		mono_mb_emit_ldarg (mb, 3);
		mono_mb_emit_calli (mb, callsig);
	}

	if (sig->ret->byref) {
		/* fixme: */
		g_assert_not_reached ();
	}

	switch (sig->ret->type) {
	case MONO_TYPE_VOID:
		if (!string_ctor)
			void_ret = TRUE;
		break;
	case MONO_TYPE_BOOLEAN:
	case MONO_TYPE_CHAR:
	case MONO_TYPE_I1:
	case MONO_TYPE_U1:
	case MONO_TYPE_I2:
	case MONO_TYPE_U2:
	case MONO_TYPE_I4:
	case MONO_TYPE_U4:
	case MONO_TYPE_I:
	case MONO_TYPE_U:
	case MONO_TYPE_R4:
	case MONO_TYPE_R8:
	case MONO_TYPE_I8:
	case MONO_TYPE_U8:
	case MONO_TYPE_VALUETYPE:
	case MONO_TYPE_TYPEDBYREF:
	case MONO_TYPE_GENERICINST:
		/* box value types */
		mono_mb_emit_op (mb, CEE_BOX, mono_class_from_mono_type (sig->ret));
		break;
	case MONO_TYPE_STRING:
	case MONO_TYPE_CLASS:  
	case MONO_TYPE_ARRAY:
	case MONO_TYPE_SZARRAY:
	case MONO_TYPE_OBJECT:
		/* nothing to do */
		break;
	case MONO_TYPE_PTR:
		/* The result is an IntPtr */
		mono_mb_emit_op (mb, CEE_BOX, mono_defaults.int_class);
		break;
	default:
		g_assert_not_reached ();
	}

	if (!void_ret)
		mono_mb_emit_stloc (mb, loc_res);

	/* Convert back nullable-byref arguments */
	for (i = 0; i < sig->param_count; i++) {
		MonoType *t = sig->params [i];

		/* 
		 * Box the result and put it back into the array, the caller will have
		 * to obtain it from there.
		 */
		if (t->byref && t->type == MONO_TYPE_GENERICINST && mono_class_is_nullable (mono_class_from_mono_type (t))) {
			mono_mb_emit_ldarg (mb, 1);			
			mono_mb_emit_icon (mb, sizeof (gpointer) * i);
			mono_mb_emit_byte (mb, CEE_ADD);

			mono_mb_emit_ldloc (mb, tmp_nullable_locals [i]);
			mono_mb_emit_op (mb, CEE_BOX, mono_class_from_mono_type (t));

			mono_mb_emit_byte (mb, CEE_STIND_REF);
		}
	}

	g_free (tmp_nullable_locals);
}

static void
emit_runtime_invoke_body (MonoMethodBuilder *mb, MonoImage *image, MonoMethod *method,
						  MonoMethodSignature *sig, MonoMethodSignature *callsig,
						  gboolean virtual_, gboolean need_direct_wrapper)
{
	gint32 labels [16];
	MonoExceptionClause *clause;
	int loc_res, loc_exc;

	/* The wrapper looks like this:
	 *
	 * <interrupt check>
	 * if (exc) {
	 *	 try {
	 *	   return <call>
	 *	 } catch (Exception e) {
	 *     *exc = e;
	 *   }
	 * } else {
	 *     return <call>
	 * }
	 */

	/* allocate local 0 (object) tmp */
	loc_res = mono_mb_add_local (mb, &mono_defaults.object_class->byval_arg);
	/* allocate local 1 (object) exc */
	loc_exc = mono_mb_add_local (mb, &mono_defaults.object_class->byval_arg);

	/* *exc is assumed to be initialized to NULL by the caller */

	mono_mb_emit_byte (mb, CEE_LDARG_2);
	labels [0] = mono_mb_emit_branch (mb, CEE_BRFALSE);

	/*
	 * if (exc) case
	 */
	labels [1] = mono_mb_get_label (mb);
	emit_thread_force_interrupt_checkpoint (mb);
	emit_invoke_call (mb, method, sig, callsig, loc_res, virtual_, need_direct_wrapper);

	labels [2] = mono_mb_emit_branch (mb, CEE_LEAVE);

	/* Add a try clause around the call */
	clause = (MonoExceptionClause *)mono_image_alloc0 (image, sizeof (MonoExceptionClause));
	clause->flags = MONO_EXCEPTION_CLAUSE_NONE;
	clause->data.catch_class = mono_defaults.exception_class;
	clause->try_offset = labels [1];
	clause->try_len = mono_mb_get_label (mb) - labels [1];

	clause->handler_offset = mono_mb_get_label (mb);

	/* handler code */
	mono_mb_emit_stloc (mb, loc_exc);	
	mono_mb_emit_byte (mb, CEE_LDARG_2);
	mono_mb_emit_ldloc (mb, loc_exc);
	mono_mb_emit_byte (mb, CEE_STIND_REF);

	mono_mb_emit_branch (mb, CEE_LEAVE);

	clause->handler_len = mono_mb_get_pos (mb) - clause->handler_offset;

	mono_mb_set_clauses (mb, 1, clause);

	mono_mb_patch_branch (mb, labels [2]);
	mono_mb_emit_ldloc (mb, loc_res);
	mono_mb_emit_byte (mb, CEE_RET);

	/*
	 * if (!exc) case
	 */
	mono_mb_patch_branch (mb, labels [0]);
	emit_thread_force_interrupt_checkpoint (mb);
	emit_invoke_call (mb, method, sig, callsig, loc_res, virtual_, need_direct_wrapper);

	mono_mb_emit_ldloc (mb, 0);
	mono_mb_emit_byte (mb, CEE_RET);
}

static void
mono_mb_emit_auto_layout_exception (MonoMethodBuilder *mb, MonoClass *klass)
{
	char *msg = g_strdup_printf ("The type `%s.%s' layout needs to be Sequential or Explicit",
				     klass->name_space, klass->name);

	mono_mb_emit_exception_marshal_directive (mb, msg);
}
