/**
 * \file
 * Copyright 2018 Microsoft
 * Licensed under the MIT license. See LICENSE file in the project root for full license information.
 */
#include "config.h"
#ifdef HAVE_SGEN_GC

#include "metadata/method-builder.h"
#include "metadata/method-builder-ilgen.h"
#include "metadata/method-builder-ilgen-internals.h"

static void
emit_nursery_check (MonoMethodBuilder *mb, int *nursery_check_return_labels, gboolean is_concurrent)
{
	int shifted_nursery_start = mono_mb_add_local (mb, &mono_defaults.int_class->byval_arg);

	memset (nursery_check_return_labels, 0, sizeof (int) * 2);
	// if (ptr_in_nursery (ptr)) return;
	/*
	 * Masking out the bits might be faster, but we would have to use 64 bit
	 * immediates, which might be slower.
	 */
	mono_mb_emit_byte (mb, MONO_CUSTOM_PREFIX);
	mono_mb_emit_byte (mb, CEE_MONO_LDPTR_NURSERY_START);
	mono_mb_emit_byte (mb, MONO_CUSTOM_PREFIX);
	mono_mb_emit_byte (mb, CEE_MONO_LDPTR_NURSERY_BITS);
	mono_mb_emit_byte (mb, CEE_SHR_UN);
	mono_mb_emit_stloc (mb, shifted_nursery_start);

	mono_mb_emit_ldarg (mb, 0);
	mono_mb_emit_byte (mb, MONO_CUSTOM_PREFIX);
	mono_mb_emit_byte (mb, CEE_MONO_LDPTR_NURSERY_BITS);
	mono_mb_emit_byte (mb, CEE_SHR_UN);
	mono_mb_emit_ldloc (mb, shifted_nursery_start);
	nursery_check_return_labels [0] = mono_mb_emit_branch (mb, CEE_BEQ);

	if (!is_concurrent) {
		// if (!ptr_in_nursery (*ptr)) return;
		mono_mb_emit_ldarg (mb, 0);
		mono_mb_emit_byte (mb, CEE_LDIND_I);
		mono_mb_emit_byte (mb, MONO_CUSTOM_PREFIX);
		mono_mb_emit_byte (mb, CEE_MONO_LDPTR_NURSERY_BITS);
		mono_mb_emit_byte (mb, CEE_SHR_UN);
		mono_mb_emit_ldloc (mb, shifted_nursery_start);
		nursery_check_return_labels [1] = mono_mb_emit_branch (mb, CEE_BNE_UN);
	}
}

static void
emit_nursery_check_ilgen (MonoMethodBuilder *mb)
{
#ifdef MANAGED_WBARRIER
	int i, nursery_check_labels [2];
	emit_nursery_check (mb, nursery_check_labels, is_concurrent);
	/*
	addr = sgen_cardtable + ((address >> CARD_BITS) & CARD_MASK)
	*addr = 1;

	sgen_cardtable:
		LDC_PTR sgen_cardtable

	address >> CARD_BITS
		LDARG_0
		LDC_I4 CARD_BITS
		SHR_UN
	if (SGEN_HAVE_OVERLAPPING_CARDS) {
		LDC_PTR card_table_mask
		AND
	}
	AND
	ldc_i4_1
	stind_i1
	*/
	mono_mb_emit_byte (mb, MONO_CUSTOM_PREFIX);
	mono_mb_emit_byte (mb, CEE_MONO_LDPTR_CARD_TABLE);
	mono_mb_emit_ldarg (mb, 0);
	mono_mb_emit_icon (mb, CARD_BITS);
	mono_mb_emit_byte (mb, CEE_SHR_UN);
	mono_mb_emit_byte (mb, CEE_CONV_I);
#ifdef SGEN_HAVE_OVERLAPPING_CARDS
#if SIZEOF_VOID_P == 8
	mono_mb_emit_icon8 (mb, CARD_MASK);
#else
	mono_mb_emit_icon (mb, CARD_MASK);
#endif
	mono_mb_emit_byte (mb, CEE_CONV_I);
	mono_mb_emit_byte (mb, CEE_AND);
#endif
	mono_mb_emit_byte (mb, CEE_ADD);
	mono_mb_emit_icon (mb, 1);
	mono_mb_emit_byte (mb, CEE_STIND_I1);

	// return;
	for (i = 0; i < 2; ++i) {
		if (nursery_check_labels [i])
			mono_mb_patch_branch (mb, nursery_check_labels [i]);
	}
	mono_mb_emit_byte (mb, CEE_RET);
#else
	mono_mb_emit_ldarg (mb, 0);
	mono_mb_emit_icall (mb, mono_gc_wbarrier_generic_nostore);
	mono_mb_emit_byte (mb, CEE_RET);
#endif
}

void
mono_sgen_mono_ilgen_init (void)
{
	MonoSgenMonoCallbacks cb;
	cb.emit_nursery_check = emit_nursery_check_ilgen;
	mono_install_sgen_mono_callbacks (&cb);
}
#endif

