/*
 * mini-exceptions-native-unwinder.c: libcorkscrew-based native unwinder
 *
 * Authors:
 *   Alex Rønne Petersen (alexrp@xamarin.com)
 *
 * Copyright 2015 Xamarin, Inc (http://www.xamarin.com)
 * Licensed under the MIT license. See LICENSE file in the project root for full license information.
 */
#include <config.h>

#include <mono/utils/mono-logger-internals.h>

/*
 * Attempt to handle native SIGSEGVs with libunwind or libcorkscrew.
 */

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

#include <mono/utils/mono-signal-handler.h>
#include "mini.h"

#if defined (PLATFORM_ANDROID)

#include <signal.h>
#include <sys/types.h>
#include <mono/utils/mono-dl.h>

#define UNW_LOCAL_ONLY
#undef _U /* ctype.h apparently defines this and it screws up the libunwind headers. */
#include "../../external/android-libunwind/include/libunwind.h"
#define _U 0x01

#define FUNC_NAME_LENGTH 512
#define FRAMES_TO_UNWIND 256

/* Expand the SYM argument. */
#define LOAD_SYM(DL, ERR, SYM, VAR) _LOAD_SYM(DL, ERR, SYM, VAR)
#define _LOAD_SYM(DL, ERR, SYM, VAR) \
	do { \
		if ((ERR = mono_dl_symbol (DL, #SYM, (void **) &VAR))) { \
			mono_dl_close (DL); \
			return ERR; \
		} \
	} while (0)

typedef int (*unw_init_local_t) (unw_cursor_t *, unw_context_t *);
typedef int (*unw_get_reg_t) (unw_cursor_t *, int, unw_word_t *);
typedef int (*unw_get_proc_name_t) (unw_cursor_t *, char *, size_t, unw_word_t *);
typedef int (*unw_step_t) (unw_cursor_t *);

static char *
mono_extension_handle_native_sigsegv_libunwind (void *ctx, MONO_SIG_HANDLER_INFO_TYPE *info)
{
	char *dl_err;
	int unw_err;

	unw_init_local_t unw_init_local_fn;
	unw_get_reg_t unw_get_reg_fn;
	unw_get_proc_name_t unw_get_proc_name_fn;
	unw_step_t unw_step_fn;

	unw_cursor_t cursor;

	size_t frames = 0;

	MonoDl *dl = mono_dl_open ("libunwind.so", MONO_DL_LAZY, &dl_err);

	if (!dl)
		return dl_err;

	LOAD_SYM (dl, dl_err, UNW_OBJ (init_local), unw_init_local_fn);
	LOAD_SYM (dl, dl_err, UNW_OBJ (get_reg), unw_get_reg_fn);
	LOAD_SYM (dl, dl_err, UNW_OBJ (get_proc_name), unw_get_proc_name_fn);
	LOAD_SYM (dl, dl_err, UNW_OBJ (step), unw_step_fn);

	if ((unw_err = unw_init_local_fn (&cursor, ctx))) {
		mono_dl_close (dl);

		return g_strdup_printf ("unw_init_local () returned %d", unw_err);
	}

	do {
		int reg_err;

		unw_word_t ip, off;
		char name [FUNC_NAME_LENGTH];

		if ((reg_err = unw_get_reg_fn (&cursor, UNW_REG_IP, &ip))) {
			mono_runtime_printf_err ("unw_get_reg (UNW_REG_IP) returned %d", reg_err);
			break;
		}

		reg_err = unw_get_proc_name_fn (&cursor, name, FUNC_NAME_LENGTH, &off);

		if (reg_err == -UNW_ENOINFO)
			strcpy (name, "???");

		mono_runtime_printf_err (" at %s+%zu [0x%zx]", name, off, ip);

		unw_err = unw_step_fn (&cursor);
		frames++;
	} while (unw_err > 0 && frames < FRAMES_TO_UNWIND);

	if (unw_err < 0)
		mono_runtime_printf_err ("unw_step () returned %d", unw_err);

	mono_dl_close (dl);

	return NULL;
}

void
mono_exception_native_unwind (void *ctx, MONO_SIG_HANDLER_INFO_TYPE *info)
{
	char *unwind_err;

	mono_runtime_printf_err ("\nAttempting native Android stacktrace:\n");
	unwind_err = mono_extension_handle_native_sigsegv_libunwind (ctx, info);

	if (unwind_err) {
		mono_runtime_printf_err ("\tCould not unwind with `libunwind.so`: %s", unwind_err);
		mono_runtime_printf_err ("\n\tNo options left to get a native stacktrace :-(");
		g_free (unwind_err);
	}
}

#else

void
mono_exception_native_unwind (void *ctx, MONO_SIG_HANDLER_INFO_TYPE *info)
{
}

#endif
