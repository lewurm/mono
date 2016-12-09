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
#include <mono/utils/mono-mmap.h>

#define UNW_LOCAL_ONLY
#undef _U /* ctype.h apparently defines this and it screws up the libunwind headers. */
#include "../../external/android-libunwind/include/libunwind.h"
#define _U 0x01

#define FUNC_NAME_LENGTH 512
#define FRAMES_TO_UNWIND 256

#define ALIGN_UP_TO(val,align)	(((val) + (align - 1)) & ~(align - 1))

static char *
mono_extension_handle_native_sigsegv_libunwind (void *ctx, MONO_SIG_HANDLER_INFO_TYPE *info)
{
	/* On Android, although mono does not register its signal handler with an
	 * altstack, ART does and then chains into our handler. The altstack size
	 * is 8k (or 16k on 64bit systems). The unw_cursor_t struct alone takes up
	 * 16k. We can't malloc because it isn't async-safe, so we use mmap. */
	int pagesize = mono_pagesize ();
	size_t cursor_len = ALIGN_UP_TO (sizeof (unw_cursor_t), pagesize);
	unw_cursor_t *cursor = mono_valloc (NULL, cursor_len, MONO_MMAP_READ|MONO_MMAP_WRITE);

	if (!cursor) {
		return g_strdup_printf ("mono_valloc failed");
	}

	size_t frames = 0;
	int unw_err;

	if ((unw_err = unw_init_local (cursor, ctx))) {
		mono_vfree (cursor, cursor_len);
		return g_strdup_printf ("unw_init_local () returned %d", unw_err);
	}

	do {
		int reg_err;

		unw_word_t ip, off;
		char name [FUNC_NAME_LENGTH];

		if ((reg_err = unw_get_reg (cursor, UNW_REG_IP, &ip))) {
			mono_runtime_printf_err ("unw_get_reg (UNW_REG_IP) returned %d", reg_err);
			break;
		}

		reg_err = unw_get_proc_name (cursor, name, FUNC_NAME_LENGTH, &off);

		if (reg_err == -UNW_ENOINFO)
			strcpy (name, "???");

		mono_runtime_printf_err (" at %s+%zu [0x%zx]", name, off, ip);

		unw_err = unw_step (cursor);
		frames++;
	} while (unw_err > 0 && frames < FRAMES_TO_UNWIND);

	if (unw_err < 0)
		mono_runtime_printf_err ("unw_step () returned %d", unw_err);

	mono_vfree (cursor, cursor_len);
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
