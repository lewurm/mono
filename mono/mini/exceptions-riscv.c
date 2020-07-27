/*
 * Licensed to the .NET Foundation under one or more agreements.
 * The .NET Foundation licenses this file to you under the MIT license.
 */

#include "mini-runtime.h"

#include <mono/metadata/abi-details.h>
#include <mono/utils/mono-sigcontext.h>
#include "mono/utils/mono-tls-inline.h"

#ifndef DISABLE_JIT

static gpointer
nop_stub (unsigned int pattern)
{
	guint8 *code, *start;

	start = code = mono_global_codeman_reserve (0x50);

	/* hang in debugger */
	riscv_addi (code, RISCV_X0, RISCV_X0, pattern);
	riscv_ebreak (code);

	mono_arch_flush_icache (start, code - start);

	return start;
}

gpointer
mono_arch_get_restore_context (MonoTrampInfo **info, gboolean aot)
{
	guint8 *start, *code;
	MonoJumpInfo *ji = NULL;
	GSList *unwind_ops = NULL;
	int i, ctx_reg, size;
	guint8 *labels [16];

	size = 512;
	code = start = mono_global_codeman_reserve (size);

	riscv_addi (code, RISCV_T0, RISCV_A0, 0);
	ctx_reg = RISCV_T0;

	/* Restore fregs */
	// arm_ldrx (code, ARMREG_IP1, ctx_reg, MONO_STRUCT_OFFSET (MonoContext, has_fregs));
	// labels [0] = code;
	// arm_cbzx (code, ARMREG_IP1, 0);
	for (i = 0; i < RISCV_N_FREGS; ++i)
		riscv_flw (code, i, ctx_reg, MONO_STRUCT_OFFSET (MonoContext, fregs) + (i * sizeof (double)));
	// mono_arm_patch (labels [0], code, MONO_R_ARM64_CBZ);

	/* Restore gregs */
	for (i = 0; i < RISCV_N_FREGS; ++i) {
		if (i == ctx_reg)
			continue;
		riscv_ld (code, i, ctx_reg, MONO_STRUCT_OFFSET (MonoContext, gregs) + (i * sizeof (double)));
	}

	riscv_ld (code, ctx_reg, ctx_reg, MONO_STRUCT_OFFSET (MonoContext, gregs) + (RISCV_ZERO * sizeof (double)));
	riscv_jalr (code, RISCV_ZERO, ctx_reg, 0);
	/* Not reached */
	riscv_ebreak (code);


#if 0
	/* Restore gregs */
	// FIXME: Restore less registers
	// FIXME: fp should be restored later
	code = mono_arm_emit_load_regarray (code, 0xffffffff & ~(1 << ctx_reg) & ~(1 << ARMREG_SP), ctx_reg, MONO_STRUCT_OFFSET (MonoContext, regs));
	/* ip0/ip1 doesn't need to be restored */
	/* ip1 = pc */
	arm_ldrx (code, ARMREG_IP1, ctx_reg, MONO_STRUCT_OFFSET (MonoContext, pc));
	/* ip0 = sp */
	arm_ldrx (code, ARMREG_IP0, ctx_reg, MONO_STRUCT_OFFSET (MonoContext, regs) + (ARMREG_SP * 8));
	/* Restore sp, ctx is no longer valid */
	arm_movspx (code, ARMREG_SP, ARMREG_IP0); 
	/* Branch to pc */
	arm_brx (code, ARMREG_IP1);
	/* Not reached */
	arm_brk (code, 0);
#endif

	g_assert ((code - start) < size);
	mono_arch_flush_icache (start, code - start);
	MONO_PROFILER_RAISE (jit_code_buffer, (start, code - start, MONO_PROFILER_CODE_BUFFER_EXCEPTION_HANDLING, NULL));

	if (info)
		*info = mono_tramp_info_create ("restore_context", start, code - start, ji, unwind_ops);

	return start;
}

gpointer
mono_arch_get_call_filter (MonoTrampInfo **info, gboolean aot)
{
	*info = NULL;
	return nop_stub (0x37);
}

gpointer
mono_arch_get_throw_exception (MonoTrampInfo **info, gboolean aot)
{
	*info = NULL;
	return nop_stub (0x77);
}

gpointer
mono_arch_get_rethrow_exception (MonoTrampInfo **info, gboolean aot)
{
	*info = NULL;
	return nop_stub (0x88);
}

gpointer
mono_arch_get_rethrow_preserve_exception (MonoTrampInfo **info, gboolean aot)
{
	*info = NULL;
	return nop_stub (0x99);
}

gpointer
mono_arch_get_throw_corlib_exception (MonoTrampInfo **info, gboolean aot)
{
	*info = NULL;
	return nop_stub (0xaa);
}

#else

gpointer
mono_arch_get_restore_context (MonoTrampInfo **info, gboolean aot)
{
    g_assert_not_reached ();
    return NULL;
}

gpointer
mono_arch_get_call_filter (MonoTrampInfo **info, gboolean aot)
{
    g_assert_not_reached ();
    return NULL;
}

gpointer
mono_arch_get_call_filter (MonoTrampInfo **info, gboolean aot)
{
    g_assert_not_reached ();
    return NULL;
}

gpointer
mono_arch_get_throw_exception (MonoTrampInfo **info, gboolean aot)
{
    g_assert_not_reached ();
    return NULL;
}

gpointer
mono_arch_get_rethrow_exception (MonoTrampInfo **info, gboolean aot)
{
    g_assert_not_reached ();
    return NULL;
}

gpointer
mono_arch_get_throw_corlib_exception (MonoTrampInfo **info, gboolean aot)
{
	g_assert_not_reached ();
	return NULL;
}

#endif

void
mono_arch_exceptions_init (void)
{
	// NOT_IMPLEMENTED;
}

gboolean
mono_arch_unwind_frame (MonoDomain *domain, MonoJitTlsData *jit_tls, MonoJitInfo *ji,
                        MonoContext *ctx, MonoContext *new_ctx, MonoLMF **lmf,
                        host_mgreg_t **save_locations, StackFrameInfo *frame)
{
	gpointer ip = MONO_CONTEXT_GET_IP (ctx);

	memset (frame, 0, sizeof (StackFrameInfo));
	frame->ji = ji;

	*new_ctx = *ctx;

	if (ji != NULL) {
		host_mgreg_t regs [MONO_MAX_IREGS + 8 + 1];
		guint8 *cfa;
		guint32 unwind_info_len;
		guint8 *unwind_info;


		if (ji->is_trampoline)
			frame->type = FRAME_TYPE_TRAMPOLINE;
		else
			frame->type = FRAME_TYPE_MANAGED;

		unwind_info = mono_jinfo_get_unwind_info (ji, &unwind_info_len);

		memcpy (regs, &new_ctx->gregs, sizeof (host_mgreg_t) * 32);
		/* v8..v15 are callee saved */
		for (int i = 0; i < 8; i++)
			(regs + MONO_MAX_IREGS) [i] = *((host_mgreg_t*)&new_ctx->fregs [8 + i]);

		gboolean success = mono_unwind_frame (unwind_info, unwind_info_len, (guint8*)ji->code_start,
						   (guint8*)ji->code_start + ji->code_size,
						   (guint8*)ip, NULL, regs, MONO_MAX_IREGS + 8,
						   save_locations, MONO_MAX_IREGS, (guint8**)&cfa);

		if (!success)
			return FALSE;

		memcpy (&new_ctx->gregs, regs, sizeof (host_mgreg_t) * 32);
		for (int i = 0; i < 8; i++)
			*((host_mgreg_t*)&new_ctx->fregs [8 + i]) = (regs + MONO_MAX_IREGS) [i];

		// new_ctx->pc = regs [ARMREG_LR];
		new_ctx->gregs [RISCV_SP] = (host_mgreg_t)(gsize)cfa;

#if 0
		if (*lmf && (*lmf)->gregs [MONO_ARCH_LMF_REG_SP] && (MONO_CONTEXT_GET_SP (ctx) >= (gpointer)(*lmf)->gregs [MONO_ARCH_LMF_REG_SP])) {
			/* remove any unused lmf */
			*lmf = (MonoLMF*)(((gsize)(*lmf)->previous_lmf) & ~3);
		}
#endif

		/* we substract 1, so that the IP points into the call instruction */
		new_ctx->gregs [RISCV_ZERO]--;

		return TRUE;
	} else if (*lmf) {
		g_assert ((((guint64)(*lmf)->previous_lmf) & 2) == 0);

		frame->type = FRAME_TYPE_MANAGED_TO_NATIVE;

		ji = mini_jit_info_table_find (domain, (gpointer)(*lmf)->pc, NULL);
		if (!ji)
			return FALSE;

		// g_assert (MONO_ARCH_LMF_REGS == ((0x3ff << 19) | (1 << ARMREG_FP) | (1 << ARMREG_SP)));
		// memcpy (&new_ctx->regs [ARMREG_R19], &(*lmf)->gregs [0], sizeof (host_mgreg_t) * 10);
		// new_ctx->regs [ARMREG_FP] = (*lmf)->gregs [MONO_ARCH_LMF_REG_FP];
		// new_ctx->regs [ARMREG_SP] = (*lmf)->gregs [MONO_ARCH_LMF_REG_SP];
		new_ctx->gregs [RISCV_ZERO] = (*lmf)->pc;

		/* we substract 1, so that the IP points into the call instruction */
		new_ctx->gregs [RISCV_ZERO]--; 

		*lmf = (MonoLMF*)(((gsize)(*lmf)->previous_lmf) & ~3);

		return TRUE;
	}

	return FALSE;
}

static void
handle_signal_exception (gpointer obj)
{
	MonoJitTlsData *jit_tls = mono_tls_get_jit_tls ();
	MonoContext ctx = jit_tls->ex_ctx;

	mono_handle_exception (&ctx, obj);
	mono_restore_context (&ctx);
}

gboolean
mono_arch_handle_exception (void *ctx, gpointer obj)
{
	MonoJitTlsData *jit_tls = mono_tls_get_jit_tls ();

	mono_sigctx_to_monoctx (ctx, &jit_tls->ex_ctx);

	// Call handle_signal_exception () on the normal stack.
	UCONTEXT_GREGS (ctx) [RISCV_A0] = (long) obj;
	UCONTEXT_REG_PC (ctx) = (long) handle_signal_exception;

	return TRUE;
}

gpointer
mono_arch_ip_from_context (void *sigctx)
{
	return (gpointer) UCONTEXT_REG_PC (sigctx);
}

void
mono_arch_setup_async_callback (MonoContext *ctx, void (*async_cb)(void *fun), gpointer user_data)
{
	// Allocate a stack frame and redirect PC.
	MONO_CONTEXT_SET_SP (ctx, (host_mgreg_t) MONO_CONTEXT_GET_SP (ctx) - 32);

	mono_arch_setup_resume_sighandler_ctx (ctx, async_cb);
}

void
mono_arch_setup_resume_sighandler_ctx (MonoContext *ctx, gpointer func)
{
	MONO_CONTEXT_SET_IP (ctx, func);
}

void
mono_arch_undo_ip_adjustment (MonoContext *context)
{
	context->gregs[RISCV_ZERO]++;
}

void
mono_arch_do_ip_adjustment (MonoContext *context)
{
	context->gregs[RISCV_ZERO]--;
}
