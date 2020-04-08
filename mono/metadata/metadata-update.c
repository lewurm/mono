/**
 * \file
 * Routines for publishing flying orchids
 *
 * Copyright 2020 Microsoft
 *
 * Licensed under the MIT license. See LICENSE file in the project root for full license information.
 */

#include <config.h>
#include <glib.h>
#include "mono/metadata/metadata-internals.h"
#include "mono/metadata/metadata-update.h"
#include "mono/metadata/object-internals.h"
#include "mono/metadata/tokentype.h"
#include "mono/utils/mono-coop-mutex.h"
#include "mono/utils/mono-error-internals.h"
#include "mono/utils/mono-lazy-init.h"
#include "mono/utils/mono-logger-internals.h"
#include "mono/utils/mono-path.h"

#if 1
#define UPDATE_DEBUG(stmt) do { stmt; } while (0)
#else
#define UPDATE_DEBUG(stmt) /*empty */
#endif

typedef struct _EncRecs {
	// for each table, the row in the EncMap table that has the first token for remapping it?
	uint32_t enc_recs [MONO_TABLE_NUM];
} EncRecs;


/* Maps each MonoTableInfo* to the MonoImage that it belongs to.  This is
 * mapping the base image MonoTableInfos to the base MonoImage.  We don't need
 * this for deltas.
 */
static GHashTable *table_to_image;
static MonoCoopMutex table_to_image_mutex;

static void
table_to_image_lock (void)
{
	mono_coop_mutex_lock (&table_to_image_mutex);
}

static void
table_to_image_unlock (void)
{
	mono_coop_mutex_unlock (&table_to_image_mutex);
}

static void
table_to_image_init (void)
{
	mono_coop_mutex_init (&table_to_image_mutex);
	table_to_image = g_hash_table_new (NULL, NULL);
}

static gboolean
remove_base_image (gpointer key, gpointer value, gpointer user_data)
{
	MonoImage *base_image = (MonoImage*)user_data;
	MonoImage *value_image = (MonoImage*)value;
	return (value_image == base_image);
}

void
mono_metadata_update_cleanup_on_close (MonoImage *base_image)
{
	/* remove all keys that map to the given image */
	table_to_image_lock ();
	g_hash_table_foreach_remove (table_to_image, remove_base_image, (gpointer)base_image);
	table_to_image_unlock ();
}

static void
table_to_image_add (MonoImage *base_image)
{
	/* If at least one table from this image is already here, they all are */
	if (g_hash_table_contains (table_to_image, &base_image->tables[MONO_TABLE_MODULE]))
		return;
	table_to_image_lock ();
	if (g_hash_table_contains (table_to_image, &base_image->tables[MONO_TABLE_MODULE])) {
	        table_to_image_unlock ();
		return;
	}
	for (int idx = 0; idx < MONO_TABLE_NUM; ++idx) {
		MonoTableInfo *table = &base_image->tables[idx];
		g_hash_table_insert (table_to_image, table, base_image);
	}
	table_to_image_unlock ();
}

MonoImage *
mono_table_info_get_base_image (const MonoTableInfo *t)
{
	MonoImage *image = (MonoImage *) g_hash_table_lookup (table_to_image, t);
	return image;
}

MonoImage*
mono_image_open_dmeta_from_data (MonoImage *base_image, uint32_t generation, const char *dmeta_name, gconstpointer dmeta_bytes, uint32_t dmeta_len, MonoImageOpenStatus *status);

MonoDilFile *
mono_dil_file_open (const char *dil_path);

void
mono_image_append_delta (MonoImage *base, MonoImage *delta);


void
mono_metadata_update_init (void)
{
	table_to_image_init ();
}


static
void
mono_metadata_update_invoke_hook (MonoDomain *domain, MonoAssemblyLoadContext *alc, uint32_t generation)
{
	if (mono_get_runtime_callbacks ()->metadata_update_published)
		mono_get_runtime_callbacks ()->metadata_update_published (domain, alc, generation);
}

static uint32_t update_published, update_alloc_frontier;
static MonoCoopMutex publish_mutex;

static void
publish_lock (void)
{
	mono_coop_mutex_lock (&publish_mutex);
}

static void
publish_unlock (void)
{
	mono_coop_mutex_unlock (&publish_mutex);
}

static mono_lazy_init_t metadata_update_lazy_init;

static void
initialize (void)
{
	mono_coop_mutex_init (&publish_mutex);
}

uint32_t
mono_metadata_update_prepare (MonoDomain *domain) {
	mono_lazy_initialize (&metadata_update_lazy_init, initialize);
	/*
	 * TODO: assert that the updater isn't depending on current metadata, else publishing might block.
	 */
	publish_lock ();
	return ++update_alloc_frontier;
}

gboolean
mono_metadata_update_available (void) {
	return update_published < update_alloc_frontier;
}

gboolean
mono_metadata_wait_for_update (uint32_t timeout_ms)
{
	/* TODO: give threads a way to voluntarily wait for an update to be published. */
	g_assert_not_reached ();
}

void
mono_metadata_update_publish (MonoDomain *domain, MonoAssemblyLoadContext *alc, uint32_t generation) {
	g_assert (update_published < generation && generation <= update_alloc_frontier);
	/* TODO: wait for all threads that are using old metadata to update. */
	mono_metadata_update_invoke_hook (domain, alc, generation);
	update_published = update_alloc_frontier;
	publish_unlock ();
}

void
mono_metadata_update_cancel (uint32_t generation)
{
	g_assert (update_alloc_frontier == generation);
	g_assert (update_alloc_frontier > 0);
	g_assert (update_alloc_frontier - 1 >= update_published);
	--update_alloc_frontier;
	publish_unlock ();
}

struct _MonoDilFile {
	MonoFileMap *filed;
	gpointer handle;
	char *il;
};

void
mono_image_append_delta (MonoImage *base, MonoImage *delta)
{
	/* FIXME: needs locking. Assumes one updater at a time */
	if (!base->delta_image) {
		base->delta_image = base->delta_image_last = g_slist_prepend (NULL, delta);
		return;
	}
	g_assert (((MonoImage*)base->delta_image_last->data)->generation < delta->generation);
	base->delta_image_last = g_slist_append (base->delta_image_last, delta);
}

MonoImage*
mono_image_open_dmeta_from_data (MonoImage *base_image, uint32_t generation, const char *dmeta_name, gconstpointer dmeta_bytes, uint32_t dmeta_len, MonoImageOpenStatus *status)
{
	MonoAssemblyLoadContext *alc = mono_image_get_alc (base_image);
	MonoImage *dmeta_image = mono_image_open_from_data_internal (alc, (char*)dmeta_bytes, dmeta_len, TRUE, status, FALSE, TRUE, dmeta_name);

	dmeta_image->generation = generation;

	/* base_image takes ownership of 1 refcount ref of dmeta_image */
	if (dmeta_image->minimal_delta) {
		/* tables will be patched and heaps will be merged */
	} else {
		mono_image_append_delta (base_image, dmeta_image);
	}

	return dmeta_image;
}

MonoDilFile *
mono_dil_file_open (const char *dil_path)
{
	MonoDilFile *dil = g_new0 (MonoDilFile, 1);

	dil->filed = mono_file_map_open (dil_path);
	g_assert (dil->filed);
	dil->il = (char *) mono_file_map_fileio (
			mono_file_map_size (dil->filed),
			MONO_MMAP_READ | MONO_MMAP_PRIVATE,
			mono_file_map_fd (dil->filed),
			0,
			&dil->handle);
	return dil;
}

void
mono_dil_file_close (MonoDilFile *dil)
{
	mono_file_map_close (dil->filed);
}

void
mono_dil_file_destroy (MonoDilFile *dil)
{
	g_free (dil);
}

static void
dump_update_summary (MonoImage *image_base, MonoImage *image_dmeta, EncRecs *enc_recs)
{
	int rows;

	mono_trace (G_LOG_LEVEL_DEBUG, MONO_TRACE_METADATA_UPDATE, "dmeta tables:");
	for (int idx = 0; idx < MONO_TABLE_NUM; ++idx) {
		if (image_dmeta->tables [idx].base)
			mono_trace (G_LOG_LEVEL_DEBUG, MONO_TRACE_METADATA_UPDATE, "\t0x%02x \"%s\"", idx, mono_meta_table_name (idx));
	}
	mono_trace (G_LOG_LEVEL_DEBUG, MONO_TRACE_METADATA_UPDATE, "================================");

	rows = mono_image_get_table_rows (image_base, MONO_TABLE_TYPEREF);
	for (int i = 1; i <= rows; ++i) {
		guint32 cols [MONO_TYPEREF_SIZE];
		mono_metadata_decode_row (&image_base->tables [MONO_TABLE_TYPEREF], i - 1, cols, MONO_TYPEREF_SIZE);
		const char *scope = mono_metadata_string_heap (image_base, cols [MONO_TYPEREF_SCOPE]);
		const char *name = mono_metadata_string_heap (image_base, cols [MONO_TYPEREF_NAME]);
		const char *namespace = mono_metadata_string_heap (image_base, cols [MONO_TYPEREF_NAMESPACE]);
		mono_trace (G_LOG_LEVEL_DEBUG, MONO_TRACE_METADATA_UPDATE, "base  typeref i=%d (token=0x%08x) -> scope=%s, namespace=%s, name=%s", i, MONO_TOKEN_TYPE_REF | i, scope, namespace, name);
	}
	if (!image_dmeta->minimal_delta) {
		mono_trace (G_LOG_LEVEL_DEBUG, MONO_TRACE_METADATA_UPDATE, "--------------------------------");

		rows = mono_image_get_table_rows (image_dmeta, MONO_TABLE_TYPEREF);
		for (int i = 1; i <= rows; ++i) {
			guint32 cols [MONO_TYPEREF_SIZE];
			mono_metadata_decode_row (&image_dmeta->tables [MONO_TABLE_TYPEREF], i - 1, cols, MONO_TYPEREF_SIZE);
			const char *scope = mono_metadata_string_heap (image_base, cols [MONO_TYPEREF_SCOPE]);
			const char *name = mono_metadata_string_heap (image_base, cols [MONO_TYPEREF_NAME]);
			const char *namespace = mono_metadata_string_heap (image_base, cols [MONO_TYPEREF_NAMESPACE]);
			mono_trace (G_LOG_LEVEL_DEBUG, MONO_TRACE_METADATA_UPDATE, "dmeta typeref i=%d (token=0x%08x) -> scope=%s, namespace=%s, name=%s", i, MONO_TOKEN_TYPE_REF | i, scope, namespace, name);
		}
	}
	mono_trace (G_LOG_LEVEL_DEBUG, MONO_TRACE_METADATA_UPDATE, "================================");

	rows = mono_image_get_table_rows (image_base, MONO_TABLE_METHOD);
	for (int i = 1; i <= rows ; ++i) {
		guint32 cols [MONO_METHOD_SIZE];
		mono_metadata_decode_row_raw (&image_base->tables [MONO_TABLE_METHOD], i - 1, cols, MONO_METHOD_SIZE);
		const char *name = mono_metadata_string_heap (image_base, cols [MONO_METHOD_NAME]);
		guint32 rva = cols [MONO_METHOD_RVA];
		mono_trace (G_LOG_LEVEL_DEBUG, MONO_TRACE_METADATA_UPDATE, "base  method i=%d (token=0x%08x), rva=%d/0x%04x, name=%s", i, MONO_TOKEN_METHOD_DEF | i, rva, rva, name);
	}
	if (!image_dmeta->minimal_delta) {
		mono_trace (G_LOG_LEVEL_DEBUG, MONO_TRACE_METADATA_UPDATE, "--------------------------------");

		rows = mono_image_get_table_rows (image_dmeta, MONO_TABLE_METHOD);
		for (int i = 1; i <= rows ; ++i) {
			guint32 cols [MONO_METHOD_SIZE];
			mono_metadata_decode_row_raw (&image_dmeta->tables [MONO_TABLE_METHOD], i - 1, cols, MONO_METHOD_SIZE);
			const char *name = mono_metadata_string_heap (image_base, cols [MONO_METHOD_NAME]);
			guint32 rva = cols [MONO_METHOD_RVA];
			int new_token = enc_recs->enc_recs [MONO_TABLE_METHOD] + i;
			mono_trace (G_LOG_LEVEL_DEBUG, MONO_TRACE_METADATA_UPDATE, "dmeta method i=%d (token=0x%08x => new_token=0x%08x), rva=%d/0x%04x, name=%s", i, MONO_TOKEN_METHOD_DEF | i, MONO_TOKEN_METHOD_DEF | new_token, rva, rva, name);
		}
	}
	mono_trace (G_LOG_LEVEL_DEBUG, MONO_TRACE_METADATA_UPDATE, "================================");

	rows = mono_image_get_table_rows (image_base, MONO_TABLE_STANDALONESIG);
	for (int i = 1; i <= rows; ++i) {
		guint32 cols [MONO_STAND_ALONE_SIGNATURE_SIZE];
		mono_metadata_decode_row (&image_base->tables [MONO_TABLE_STANDALONESIG], i - 1, cols, MONO_STAND_ALONE_SIGNATURE_SIZE);
		mono_trace (G_LOG_LEVEL_DEBUG, MONO_TRACE_METADATA_UPDATE, "base  standalonesig i=%d (token=0x%08x) -> 0x%08x", i, MONO_TOKEN_SIGNATURE | i, cols [MONO_STAND_ALONE_SIGNATURE]);
	}

	if (!image_dmeta->minimal_delta) {
		mono_trace (G_LOG_LEVEL_DEBUG, MONO_TRACE_METADATA_UPDATE, "--------------------------------");

		rows = mono_image_get_table_rows (image_dmeta, MONO_TABLE_STANDALONESIG);
		for (int i = 1; i <= rows; ++i) {
			guint32 cols [MONO_STAND_ALONE_SIGNATURE_SIZE];
			mono_metadata_decode_row_raw (&image_dmeta->tables [MONO_TABLE_STANDALONESIG], i - 1, cols, MONO_STAND_ALONE_SIGNATURE_SIZE);
			mono_trace (G_LOG_LEVEL_DEBUG, MONO_TRACE_METADATA_UPDATE, "dmeta standalonesig i=%d (token=0x%08x) -> 0x%08x", i, MONO_TOKEN_SIGNATURE | i, cols [MONO_STAND_ALONE_SIGNATURE]);
		}
	}
	mono_trace (G_LOG_LEVEL_DEBUG, MONO_TRACE_METADATA_UPDATE, "================================");

}

static int
relative_delta_index (MonoImage *image_dmeta, EncRecs *enc_recs, int token)
{
	MonoTableInfo *encmap = &image_dmeta->tables [MONO_TABLE_ENCMAP];
	if (!encmap->rows || !image_dmeta->minimal_delta)
		return token;

	int table = mono_metadata_token_table (token);
	int index = mono_metadata_token_index (token);

	int index_map = enc_recs->enc_recs [table];
	guint32 cols[MONO_ENCMAP_SIZE];
	mono_metadata_decode_row (encmap, index_map - 1, cols, MONO_ENCMAP_SIZE);
	int map_entry = cols [MONO_ENCMAP_TOKEN];

	while (mono_metadata_token_table (map_entry) == table && mono_metadata_token_index (map_entry) < index) {
		mono_metadata_decode_row (encmap, ++index_map - 1, cols, MONO_ENCMAP_SIZE);
		map_entry = cols [MONO_ENCMAP_TOKEN];
	}

	/* this is the index into the minimal delta image */
	return index_map - enc_recs->enc_recs [table] + 1;
}

static void
start_encmap (MonoImage *image_dmeta, EncRecs *enc_recs)
{
	memset (enc_recs, 0, sizeof (*enc_recs));
	MonoTableInfo *encmap = &image_dmeta->tables [MONO_TABLE_ENCMAP];
	int table;
	int prev_table = -1;
	if (!encmap->rows)
		return;
	int idx;
	for (idx = 1; idx <= encmap->rows; ++idx) {
		guint32 cols[MONO_ENCMAP_SIZE];
		mono_metadata_decode_row (encmap, idx - 1, cols, MONO_ENCMAP_SIZE);
		uint32_t tok = cols [MONO_ENCMAP_TOKEN];
		table = mono_metadata_token_table (tok);
		g_assert (table >= 0 && table < MONO_TABLE_NUM);
		g_assert (table != MONO_TABLE_ENCLOG);
		g_assert (table != MONO_TABLE_ENCMAP);
		/* FIXME: this function is cribbed from CMiniMdRW::StartENCMap
		 * https://github.com/dotnet/runtime/blob/4f9ae42d861fcb4be2fcd5d3d55d5f227d30e723/src/coreclr/src/md/enc/metamodelenc.cpp#L389
		 * but for some reason the following assertion table >= prev_table fails.
		 */
		g_assert (table >= prev_table);
		if (table == prev_table)
			continue;
		while (prev_table < table) {
			prev_table++;
			enc_recs->enc_recs [prev_table] = idx;
		}
	}
	while (prev_table < MONO_TABLE_NUM) {
		prev_table++;
		enc_recs->enc_recs [prev_table] = idx;
	}

	for (int i = 0 ; i < MONO_TABLE_NUM; ++i) {
		if (image_dmeta->tables [i].base)
			mono_trace (G_LOG_LEVEL_DEBUG, MONO_TRACE_METADATA_UPDATE, "enc_recs [%02x] / %s = 0x%02x", i, mono_meta_table_name (i), enc_recs->enc_recs[i]);
	}
}

static gboolean
apply_enclog (MonoImage *image_base, MonoImage *image_dmeta, MonoDilFile *dil, MonoError *error)
{
	MonoTableInfo *table_enclog = &image_dmeta->tables [MONO_TABLE_ENCLOG];
	int rows = table_enclog->rows;

	for (int i = 0; i < rows ; ++i) {
		guint32 cols [MONO_ENCLOG_SIZE];
		mono_metadata_decode_row (table_enclog, i, cols, MONO_ENCLOG_SIZE);

		int log_token = cols [MONO_ENCLOG_TOKEN];
		int func_code = cols [MONO_ENCLOG_FUNC_CODE];

		int table_index = mono_metadata_token_table (log_token);
		mono_trace (G_LOG_LEVEL_DEBUG, MONO_TRACE_METADATA_UPDATE, "enclog i=%d: token=0x%08x (table=%s): %d", i, log_token, mono_meta_table_name (table_index), func_code);

		switch (func_code) {
			case 0: /* default */
				break;
			default:
				g_error ("EnC: FuncCode %d not supported (token=0x%08x)", func_code, log_token);
				break;
		}

		if (table_index == MONO_TABLE_METHOD) {
			if (!image_base->delta_index)
				image_base->delta_index = g_hash_table_new (g_direct_hash, g_direct_equal);

			int token_idx = mono_metadata_token_index (log_token);
			int rva = mono_metadata_decode_row_col (&image_dmeta->tables [MONO_TABLE_METHOD], token_idx - 1, MONO_METHOD_RVA);

			g_hash_table_insert (image_base->delta_index, GUINT_TO_POINTER (token_idx), (gpointer) (dil->il + rva));
		} else {
			g_print ("todo: do something about this table index: 0x%02x\n", table_index);
		}
	}
	return TRUE;
}

static void
append_heap (MonoStreamHeader *base, MonoStreamHeader *appendix)
{
	int size = base->size + appendix->size;
	char *data = (char *) g_malloc (size * sizeof (char));
	memcpy (data, base->data, base->size);
	memcpy (data + base->size, appendix->data, appendix->size);
	base->data = data;
	base->size = size;
}

void
mono_image_load_enc_delta (MonoDomain *domain, MonoImage *image_base, const char *dmeta_name, gconstpointer dmeta_bytes, uint32_t dmeta_len, const char *dil_path)
{
	const char *basename = image_base->filename;

	mono_trace (G_LOG_LEVEL_INFO, MONO_TRACE_METADATA_UPDATE, "LOADING basename=%s, dmeta=%s, dil=%s", basename, dmeta_name, dil_path);

	uint32_t generation = mono_metadata_update_prepare (domain);

	mono_trace (G_LOG_LEVEL_DEBUG, MONO_TRACE_METADATA_UPDATE, "base image string size: 0x%08x", image_base->heap_strings.size);
	mono_trace (G_LOG_LEVEL_DEBUG, MONO_TRACE_METADATA_UPDATE, "base image user string size: 0x%08x", image_base->heap_us.size);
	mono_trace (G_LOG_LEVEL_DEBUG, MONO_TRACE_METADATA_UPDATE, "base image blob heap addr: %p", image_base->heap_blob.data);
	mono_trace (G_LOG_LEVEL_DEBUG, MONO_TRACE_METADATA_UPDATE, "base image blob heap size: 0x%08x", image_base->heap_blob.size);

	MonoImageOpenStatus status;
	MonoImage *image_dmeta = mono_image_open_dmeta_from_data (image_base, generation, dmeta_name, dmeta_bytes, dmeta_len, &status);
	mono_trace (G_LOG_LEVEL_DEBUG, MONO_TRACE_METADATA_UPDATE, "delta image string size: 0x%08x", image_dmeta->heap_strings.size);
	mono_trace (G_LOG_LEVEL_DEBUG, MONO_TRACE_METADATA_UPDATE, "delta image user string size: 0x%08x", image_dmeta->heap_us.size);
	mono_trace (G_LOG_LEVEL_DEBUG, MONO_TRACE_METADATA_UPDATE, "delta image blob heap addr: %p", image_dmeta->heap_blob.data);
	mono_trace (G_LOG_LEVEL_DEBUG, MONO_TRACE_METADATA_UPDATE, "delta image blob heap size: 0x%08x", image_dmeta->heap_blob.size);
	g_assert (image_dmeta);
	g_assert (status == MONO_IMAGE_OK);

	if (image_dmeta->minimal_delta) {
		guint32 idx = mono_metadata_decode_row_col (&image_dmeta->tables [MONO_TABLE_MODULE],
							    0, MONO_MODULE_NAME);

		/* FIXME: for the 2nd update to the same image, we should decrement idx by the sum of all the previous
		 * heap segments.
		 */
		g_assert (idx >= image_base->heap_strings.size);
		const char *module_name = mono_metadata_string_heap (image_dmeta, idx - image_base->heap_strings.size);

		/* Set the module name now that we know the base String heap size */
		g_assert (!image_dmeta->module_name);
		image_dmeta->module_name = module_name;
		mono_trace (G_LOG_LEVEL_DEBUG, MONO_TRACE_METADATA_UPDATE, "applied dmeta name: '%s'\n", module_name);
	}

	MonoTableInfo *table_enclog = &image_dmeta->tables [MONO_TABLE_ENCLOG];
	MonoTableInfo *table_encmap = &image_dmeta->tables [MONO_TABLE_ENCMAP];

	/* if there are updates, start tracking the tables of the base image, if we weren't already. */
	if (table_enclog->rows)
		table_to_image_add (image_base);

	EncRecs enc_recs;
	start_encmap (image_dmeta, &enc_recs);

	mono_trace (G_LOG_LEVEL_DEBUG, MONO_TRACE_METADATA_UPDATE, "base  guid: %s", image_base->guid);
	mono_trace (G_LOG_LEVEL_DEBUG, MONO_TRACE_METADATA_UPDATE, "dmeta guid: %s", image_dmeta->guid);

	MonoDilFile *dil = mono_dil_file_open (dil_path);
	image_dmeta->delta_il = dil;

	if (image_dmeta->minimal_delta) {
		/* (1) patch cols in base table */
		
		/* TODO: patch me! */

		/* (2) append heaps (blob heap. any others?) */
		append_heap (&image_base->heap_blob, &image_dmeta->heap_blob);
		mono_trace (G_LOG_LEVEL_DEBUG, MONO_TRACE_METADATA_UPDATE, "base image blob heap addr (merged): %p", image_base->heap_blob.data);
		mono_trace (G_LOG_LEVEL_DEBUG, MONO_TRACE_METADATA_UPDATE, "base image blob heap size (merged): 0x%08x", image_base->heap_blob.size);
		
		append_heap (&image_base->heap_strings, &image_dmeta->heap_strings);
		mono_trace (G_LOG_LEVEL_DEBUG, MONO_TRACE_METADATA_UPDATE, "base image strings heap addr (merged): %p", image_base->heap_strings.data);
		mono_trace (G_LOG_LEVEL_DEBUG, MONO_TRACE_METADATA_UPDATE, "base image strings heap size (merged): 0x%08x", image_base->heap_strings.size);
	}

	if (mono_trace_is_traced (G_LOG_LEVEL_DEBUG, MONO_TRACE_METADATA_UPDATE))
		dump_update_summary (image_base, image_dmeta, &enc_recs);

#if 0
	printf ("table_enclog->rows: %d\n", table_enclog->rows);
	printf ("table_encmap->rows: %d\n", table_encmap->rows);
#endif
	if (!table_enclog->rows && !table_encmap->rows) {
		mono_trace (G_LOG_LEVEL_INFO, MONO_TRACE_METADATA_UPDATE, "No enclog or encmap in delta image %s, nothing to do", dmeta_name);
		mono_metadata_update_cancel (generation);
		return;
	}

	ERROR_DECL (error);
	if (!apply_enclog (image_base, image_dmeta, dil, error)) {
		mono_trace (G_LOG_LEVEL_INFO, MONO_TRACE_METADATA_UPDATE, "Error applying delta image %s, due to: %s", dmeta_name, mono_error_get_message (error));
		mono_error_cleanup (error);
		mono_metadata_update_cancel (generation);
		return;
	}
	mono_error_assert_ok (error);

	MonoAssemblyLoadContext *alc = mono_image_get_alc (image_base);
	mono_metadata_update_publish (domain, alc, generation);

	mono_trace (G_LOG_LEVEL_INFO, MONO_TRACE_METADATA_UPDATE, ">>> EnC delta %s (generation %d) applied", dmeta_name, generation);
}
