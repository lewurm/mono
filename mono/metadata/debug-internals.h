#ifndef __DEBUG_INTERNALS_H__
#define __DEBUG_INTERNALS_H__

#include <glib.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/mono-debug.h>
#include <mono/utils/mono-compiler.h>

struct _MonoDebugMethodInfo {
	MonoMethod *method;
	MonoDebugHandle *handle;
	uint32_t index;
	uint32_t data_offset;
	uint32_t lnt_offset;
};

typedef struct {
	int parent;
	int type;
	/* IL offsets */
	int start_offset, end_offset;
} MonoDebugCodeBlock;

typedef struct {
	char *name;
	int index;
	/* Might be null for the main scope */
	MonoDebugCodeBlock *block;
} MonoDebugLocalVar;

/*
 * Information about local variables retrieved from a symbol file.
 */
struct _MonoDebugLocalsInfo {
	int num_locals;
	MonoDebugLocalVar *locals;
	int num_blocks;
	MonoDebugCodeBlock *code_blocks;
};

struct _MonoDebugLineNumberEntry {
	uint32_t il_offset;
	uint32_t native_offset;
};

/*
 * Information about a source file retrieved from a symbol file.
 */
typedef struct {
	char *source_file;
	/* 16 byte long */
	guint8 *guid, *hash;
} MonoDebugSourceInfo;

typedef struct {
	int il_offset;
	int line, column;
	int end_line, end_column;
} MonoSymSeqPoint;

void            mono_debugger_lock                          (void);
void            mono_debugger_unlock                        (void);

void
mono_debug_get_seq_points (MonoDebugMethodInfo *minfo, char **source_file, GPtrArray **source_file_list, int **source_files, MonoSymSeqPoint **seq_points, int *n_seq_points);

MONO_API void
mono_debug_free_locals (MonoDebugLocalsInfo *info);


gboolean
mono_debug_image_has_debug_info (MonoImage *image);

MonoDebugSourceLocation *
mono_debug_lookup_source_location_by_il (MonoMethod *method, guint32 il_offset, MonoDomain *domain);

#endif /* __DEBUG_INTERNALS_H__ */
