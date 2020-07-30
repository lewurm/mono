/*
 * Licensed to the .NET Foundation under one or more agreements.
 * The .NET Foundation licenses this file to you under the MIT license.
 */

#include <mono/utils/mono-hwcap.h>

#include "mini-runtime.h"
#include "ir-emit.h"

#ifdef TARGET_RISCV64
#include "cpu-riscv64.h"
#else
#include "cpu-riscv32.h"
#endif

static gboolean riscv_stdext_a, riscv_stdext_b, riscv_stdext_c,
                riscv_stdext_d, riscv_stdext_f, riscv_stdext_j,
                riscv_stdext_l, riscv_stdext_m, riscv_stdext_n,
                riscv_stdext_p, riscv_stdext_q, riscv_stdext_t,
                riscv_stdext_v;

/* The single step trampoline */
static gpointer ss_trampoline;

/* The breakpoint trampoline */
static gpointer bp_trampoline;

void
mono_arch_cpu_init (void)
{
}

void
mono_arch_init (void)
{
	riscv_stdext_a = mono_hwcap_riscv_has_stdext_a;
	riscv_stdext_c = mono_hwcap_riscv_has_stdext_c;
	riscv_stdext_d = mono_hwcap_riscv_has_stdext_d;
	riscv_stdext_f = mono_hwcap_riscv_has_stdext_f;
	riscv_stdext_m = mono_hwcap_riscv_has_stdext_m;

#if 0
	if (!mono_aot_only)
		bp_trampoline = mini_get_breakpoint_trampoline ();
#endif
}

void
mono_arch_finish_init (void)
{
}

void
mono_arch_register_lowlevel_calls (void)
{
}

void
mono_arch_cleanup (void)
{
}

void
mono_arch_set_target (char *mtriple)
{
	// riscv{32,64}[extensions]-[<vendor>-]<system>-<abi>

	size_t len = strlen (MONO_RISCV_ARCHITECTURE);

	if (!strncmp (mtriple, MONO_RISCV_ARCHITECTURE, len)) {
		mtriple += len;

		for (;;) {
			char c = *mtriple;

			if (!c || c == '-')
				break;

			// ISA manual says upper and lower case are both OK.
			switch (c) {
			case 'A':
			case 'a':
				riscv_stdext_a = TRUE;
				break;
			case 'B':
			case 'b':
				riscv_stdext_b = TRUE;
				break;
			case 'C':
			case 'c':
				riscv_stdext_c = TRUE;
				break;
			case 'D':
			case 'd':
				riscv_stdext_d = TRUE;
				break;
			case 'F':
			case 'f':
				riscv_stdext_f = TRUE;
				break;
			case 'J':
			case 'j':
				riscv_stdext_j = TRUE;
				break;
			case 'L':
			case 'l':
				riscv_stdext_l = TRUE;
				break;
			case 'M':
			case 'm':
				riscv_stdext_m = TRUE;
				break;
			case 'N':
			case 'n':
				riscv_stdext_n = TRUE;
				break;
			case 'P':
			case 'p':
				riscv_stdext_p = TRUE;
				break;
			case 'Q':
			case 'q':
				riscv_stdext_q = TRUE;
				break;
			case 'T':
			case 't':
				riscv_stdext_t = TRUE;
				break;
			case 'V':
			case 'v':
				riscv_stdext_v = TRUE;
				break;
			default:
				break;
			}

			mtriple++;
		}
	}
}

guint32
mono_arch_cpu_optimizations (guint32 *exclude_mask)
{
	*exclude_mask = 0;
	return 0;
}

gboolean
mono_arch_have_fast_tls (void)
{
	return TRUE;
}

gboolean
mono_arch_opcode_supported (int opcode)
{
	switch (opcode) {
	case OP_ATOMIC_ADD_I4:
	case OP_ATOMIC_EXCHANGE_I4:
	case OP_ATOMIC_CAS_I4:
	case OP_ATOMIC_LOAD_I1:
	case OP_ATOMIC_LOAD_I2:
	case OP_ATOMIC_LOAD_I4:
	case OP_ATOMIC_LOAD_U1:
	case OP_ATOMIC_LOAD_U2:
	case OP_ATOMIC_LOAD_U4:
	case OP_ATOMIC_STORE_I1:
	case OP_ATOMIC_STORE_I2:
	case OP_ATOMIC_STORE_I4:
	case OP_ATOMIC_STORE_U1:
	case OP_ATOMIC_STORE_U2:
	case OP_ATOMIC_STORE_U4:
#ifdef TARGET_RISCV64
	case OP_ATOMIC_ADD_I8:
	case OP_ATOMIC_EXCHANGE_I8:
	case OP_ATOMIC_CAS_I8:
	case OP_ATOMIC_LOAD_I8:
	case OP_ATOMIC_LOAD_U8:
	case OP_ATOMIC_STORE_I8:
	case OP_ATOMIC_STORE_U8:
#endif
		return riscv_stdext_a;
	case OP_ATOMIC_LOAD_R4:
	case OP_ATOMIC_STORE_R4:
#ifdef TARGET_RISCV64
	case OP_ATOMIC_LOAD_R8:
	case OP_ATOMIC_STORE_R8:
#endif
		return riscv_stdext_a && riscv_stdext_d;
	default:
		return FALSE;
	}
}

const char *
mono_arch_regname (int reg)
{
    static const char *names [RISCV_N_GREGS] = {
		"zero", "ra", "sp",  "gp",  "tp", "t0", "t1", "t2",
		"s0",   "s1", "a0",  "a1",  "a2", "a3", "a4", "a5",
		"a6",   "a7", "s2",  "s3",  "s4", "s5", "s6", "s7",
		"s8",   "s9", "s10", "s11", "t3", "t4", "t5", "t6",
    };

    if (reg >= 0 && reg < G_N_ELEMENTS (names))
        return names [reg];

    return "x?";
}

const char*
mono_arch_fregname (int reg)
{
    static const char *names [RISCV_N_FREGS] = {
		"ft0", "ft1", "ft2",  "ft3",  "ft4", "ft5", "ft6",  "ft7",
		"fs0", "fs1", "fa0",  "fa1",  "fa2", "fa3", "fa4",  "fa5",
		"fa6", "fa7", "fs2",  "fs3",  "fs4", "fs5", "fs6",  "fs7",
		"fs8", "fs9", "fs10", "fs11", "ft8", "ft9", "ft10", "ft11",
    };

    if (reg >= 0 && reg < G_N_ELEMENTS (names))
        return names [reg];

    return "f?";
}

gpointer
mono_arch_get_this_arg_from_call (host_mgreg_t *regs, guint8 *code)
{
	return (gpointer) regs [RISCV_A0];
}

MonoMethod *
mono_arch_find_imt_method (host_mgreg_t *regs, guint8 *code)
{
	return (MonoMethod *) regs [MONO_ARCH_IMT_REG];
}

MonoVTable *
mono_arch_find_static_call_vtable (host_mgreg_t *regs, guint8 *code)
{
	return (MonoVTable *) regs [MONO_ARCH_VTABLE_REG];
}

GSList*
mono_arch_get_cie_program (void)
{
	GSList *l = NULL;

	mono_add_unwind_op_def_cfa (l, (guint8*)NULL, (guint8*)NULL, RISCV_SP, 0);

	return l;
}

host_mgreg_t
mono_arch_context_get_int_reg (MonoContext *ctx, int reg)
{
	return ctx->gregs [reg];
}

void
mono_arch_context_set_int_reg (MonoContext *ctx, int reg, host_mgreg_t val)
{
	ctx->gregs [reg] = val;
}

void
mono_arch_flush_register_windows (void)
{
}

void
mono_arch_flush_icache (guint8 *code, gint size)
{
#ifndef MONO_CROSS_COMPILE
	__builtin___clear_cache (code, code + size);
#endif
}

MonoDynCallInfo *
mono_arch_dyn_call_prepare (MonoMethodSignature *sig)
{
	NOT_IMPLEMENTED;
	return NULL;
}

void
mono_arch_dyn_call_free (MonoDynCallInfo *info)
{
	NOT_IMPLEMENTED;
}

int
mono_arch_dyn_call_get_buf_size (MonoDynCallInfo *info)
{
	NOT_IMPLEMENTED;
	return 0;
}

void
mono_arch_start_dyn_call (MonoDynCallInfo *info, gpointer **args, guint8 *ret,
                          guint8 *buf)
{
	NOT_IMPLEMENTED;
}

void
mono_arch_finish_dyn_call (MonoDynCallInfo *info, guint8 *buf)
{
	NOT_IMPLEMENTED;
}

int
mono_arch_get_argument_info (MonoMethodSignature *csig, int param_count,
                             MonoJitArgumentInfo *arg_info)
{
    NOT_IMPLEMENTED;
    return 0;
}

void
mono_arch_patch_code_new (MonoCompile *cfg, MonoDomain *domain, guint8 *code,
                          MonoJumpInfo *ji, gpointer target)
{
	NOT_IMPLEMENTED;
}

/* Set arguments in the ccontext (for i2n entry) */
void
mono_arch_set_native_call_context_args (CallContext *ccontext, gpointer frame, MonoMethodSignature *sig)
{
	NOT_IMPLEMENTED;
}

/* Set return value in the ccontext (for n2i return) */
void
mono_arch_set_native_call_context_ret (CallContext *ccontext, gpointer frame, MonoMethodSignature *sig)
{
	NOT_IMPLEMENTED;
}

/* Gets the arguments from ccontext (for n2i entry) */
void
mono_arch_get_native_call_context_args (CallContext *ccontext, gpointer frame, MonoMethodSignature *sig)
{
	NOT_IMPLEMENTED;
}

/* Gets the return value from ccontext (for i2n exit) */
void
mono_arch_get_native_call_context_ret (CallContext *ccontext, gpointer frame, MonoMethodSignature *sig)
{
	NOT_IMPLEMENTED;
}

#ifndef DISABLE_JIT

#ifdef MONO_ARCH_SOFT_FLOAT_FALLBACK

gboolean
mono_arch_is_soft_float (void)
{
	return !riscv_stdext_d;
}

#endif

gboolean
mono_arch_opcode_needs_emulation (MonoCompile *cfg, int opcode)
{
	switch (opcode) {
	case OP_IDIV:
	case OP_IDIV_UN:
	case OP_IREM:
	case OP_IREM_UN:
#ifdef TARGET_RISCV64
	case OP_LDIV:
	case OP_LDIV_UN:
	case OP_LREM:
	case OP_LREM_UN:
#endif
		return !riscv_stdext_m;
	default:
		return TRUE;
	}
}

gboolean
mono_arch_tailcall_supported (MonoCompile *cfg, MonoMethodSignature *caller_sig, MonoMethodSignature *callee_sig, gboolean virtual_)
{
	NOT_IMPLEMENTED;
}

gboolean
mono_arch_is_inst_imm (int opcode, int imm_opcode, gint64 imm)
{
	// TODO: Make a proper decision based on opcode.
	return TRUE;
}

GList *
mono_arch_get_allocatable_int_vars (MonoCompile *cfg)
{
	GList *vars = NULL;

	for (guint i = 0; i < cfg->num_varinfo; i++) {
		MonoInst *ins = cfg->varinfo [i];
		MonoMethodVar *vmv = MONO_VARINFO (cfg, i);

		if (vmv->range.first_use.abs_pos >= vmv->range.last_use.abs_pos)
			continue;

		if ((ins->flags & (MONO_INST_IS_DEAD | MONO_INST_VOLATILE | MONO_INST_INDIRECT)) ||
		    (ins->opcode != OP_LOCAL && ins->opcode != OP_ARG))
			continue;

		if (!mono_is_regsize_var (ins->inst_vtype))
			continue;

		vars = g_list_prepend (vars, vmv);
	}

	vars = mono_varlist_sort (cfg, vars, 0);

	return vars;
}

GList *
mono_arch_get_global_int_regs (MonoCompile *cfg)
{
	GList *regs = NULL;

	for (int i = RISCV_S0; i <= RISCV_S11; i++)
		regs = g_list_prepend (regs, GUINT_TO_POINTER (i));

	return regs;
}

guint32
mono_arch_regalloc_cost (MonoCompile *cfg, MonoMethodVar *vmv)
{
	return cfg->varinfo [vmv->idx]->opcode == OP_ARG ? 1 : 2;
}

#ifdef ENABLE_LLVM

LLVMCallInfo*
mono_arch_get_llvm_call_info (MonoCompile *cfg, MonoMethodSignature *sig)
{
	NOT_IMPLEMENTED;
}

#endif

static void
add_general (CallInfo *cinfo, ArgInfo *ainfo, int size, gboolean sign)
{
	if (cinfo->gr >= RISCV_A0 + RISCV_N_GAREGS) {
		ainfo->storage = ArgOnStack;
		size = RISCV_XLEN / 8;
		sign = FALSE;
		cinfo->stack_usage = ALIGN_TO (cinfo->stack_usage, size);
		ainfo->offset = cinfo->stack_usage;
		ainfo->slot_size = size;
		ainfo->sign = sign;
		cinfo->stack_usage += size;
	} else {
		ainfo->storage = ArgInIReg;
		ainfo->reg = cinfo->gr;
		cinfo->gr ++;
	}
}

static void
add_fp (CallInfo *cinfo, ArgInfo *ainfo, gboolean single)
{
	// int size = single ? 4 : 8;

	if (cinfo->fr >= RISCV_FA0 + RISCV_N_FAREGS) {
		ainfo->storage = single ? ArgOnStackR4 : ArgOnStackR8;
		ainfo->offset = cinfo->stack_usage;
		/* TODO: verify that for float and double, each on RV32 and RV64 */
		ainfo->slot_size = 8;
		/* Put arguments into 8 byte aligned stack slots */
		cinfo->stack_usage += 8;
	} else {
		if (single)
			ainfo->storage = ArgInFRegR4;
		else
			ainfo->storage = ArgInFReg;
		ainfo->reg = cinfo->fr;
		cinfo->fr ++;
	}
}

static void
add_param (CallInfo *cinfo, ArgInfo *ainfo, MonoType *t)
{
	MonoType *ptype;

	ptype = mini_get_underlying_type (t);
	switch (ptype->type) {
	case MONO_TYPE_I1:
		add_general (cinfo, ainfo, 1, TRUE);
		break;
	case MONO_TYPE_U1:
		add_general (cinfo, ainfo, 1, FALSE);
		break;
	case MONO_TYPE_I2:
		add_general (cinfo, ainfo, 2, TRUE);
		break;
	case MONO_TYPE_U2:
		add_general (cinfo, ainfo, 2, FALSE);
		break;
#ifdef TARGET_RISCV32
	case MONO_TYPE_I:
#endif
	case MONO_TYPE_I4:
		add_general (cinfo, ainfo, 4, TRUE);
		break;
#ifdef TARGET_RISCV32
	case MONO_TYPE_U:
	case MONO_TYPE_PTR:
	case MONO_TYPE_FNPTR:
	case MONO_TYPE_OBJECT:
#endif
	case MONO_TYPE_U4:
		add_general (cinfo, ainfo, 4, FALSE);
		break;
#ifdef TARGET_RISCV64
	case MONO_TYPE_I:
	case MONO_TYPE_U:
	case MONO_TYPE_PTR:
	case MONO_TYPE_FNPTR:
	case MONO_TYPE_OBJECT:
#endif
	case MONO_TYPE_U8:
	case MONO_TYPE_I8:
		add_general (cinfo, ainfo, 8, FALSE);
		break;
	case MONO_TYPE_R8:
		add_fp (cinfo, ainfo, FALSE);
		break;
	case MONO_TYPE_R4:
		add_fp (cinfo, ainfo, TRUE);
		break;
	case MONO_TYPE_VALUETYPE:
	case MONO_TYPE_TYPEDBYREF:
		NOT_IMPLEMENTED;
		// add_valuetype (cinfo, ainfo, ptype);
		break;
	case MONO_TYPE_VOID:
		ainfo->storage = ArgNone;
		break;
	case MONO_TYPE_GENERICINST:
		if (!mono_type_generic_inst_is_valuetype (ptype)) {
			add_general (cinfo, ainfo, 8, FALSE);
		} else if (mini_is_gsharedvt_variable_type (ptype)) {
			/*
			 * Treat gsharedvt arguments as large vtypes
			 */
			ainfo->storage = ArgVtypeByRef;
			ainfo->gsharedvt = TRUE;
		} else {
			NOT_IMPLEMENTED;
			// add_valuetype (cinfo, ainfo, ptype);
		}
		break;
	case MONO_TYPE_VAR:
	case MONO_TYPE_MVAR:
		g_assert (mini_is_gsharedvt_type (ptype));
		ainfo->storage = ArgVtypeByRef;
		ainfo->gsharedvt = TRUE;
		break;
	default:
		g_assert_not_reached ();
		break;
	}
}

/*
 * get_call_info:
 *
 *  Obtain information about a call according to the calling convention.
 */
static CallInfo*
get_call_info (MonoMemPool *mp, MonoMethodSignature *sig)
{
	CallInfo *cinfo;
	ArgInfo *ainfo;
	int n, pstart, pindex;

	n = sig->hasthis + sig->param_count;

	if (mp)
		cinfo = mono_mempool_alloc0 (mp, sizeof (CallInfo) + (sizeof (ArgInfo) * n));
	else
		cinfo = g_malloc0 (sizeof (CallInfo) + (sizeof (ArgInfo) * n));

	cinfo->nargs = n;
	cinfo->pinvoke = sig->pinvoke;

	/* Return value */
	add_param (cinfo, &cinfo->ret, sig->ret);
	if (cinfo->ret.storage == ArgVtypeByRef)
		cinfo->ret.reg = RISCV_A0;
	/* Reset state */
	cinfo->gr = RISCV_A0;
	cinfo->fr = RISCV_FA0;
	cinfo->stack_usage = 0;

	/* Parameters */
	if (sig->hasthis)
		add_general (cinfo, cinfo->args + 0, 8, FALSE);
	pstart = 0;
	for (pindex = pstart; pindex < sig->param_count; ++pindex) {
		ainfo = cinfo->args + sig->hasthis + pindex;

		if ((sig->call_convention == MONO_CALL_VARARG) && (pindex == sig->sentinelpos)) {
			/* Prevent implicit arguments and sig_cookie from
			   being passed in registers */
			cinfo->gr = RISCV_A0 + RISCV_N_GAREGS;
			cinfo->fr = RISCV_FA0 + RISCV_N_FAREGS;
			/* Emit the signature cookie just before the implicit arguments */
			add_param (cinfo, &cinfo->sig_cookie, mono_get_int_type ());
		}

		add_param (cinfo, ainfo, sig->params [pindex]);
		if (ainfo->storage == ArgVtypeByRef) {
			/* Pass the argument address in the next register */
			if (cinfo->gr >= RISCV_A0 + RISCV_N_GAREGS) {
				ainfo->storage = ArgVtypeByRefOnStack;
				cinfo->stack_usage = ALIGN_TO (cinfo->stack_usage, RISCV_XLEN / 8);
				ainfo->offset = cinfo->stack_usage;
				cinfo->stack_usage += RISCV_XLEN / 8;
			} else {
				ainfo->reg = cinfo->gr;
				cinfo->gr ++;
			}
		}
	}

	/* Handle the case where there are no implicit arguments */
	if ((sig->call_convention == MONO_CALL_VARARG) && (pindex == sig->sentinelpos)) {
		/* Prevent implicit arguments and sig_cookie from
		   being passed in registers */
		cinfo->gr = RISCV_A0 + RISCV_N_GAREGS;
		cinfo->fr = RISCV_FA0 + RISCV_N_FAREGS;
		/* Emit the signature cookie just before the implicit arguments */
		add_param (cinfo, &cinfo->sig_cookie, mono_get_int_type ());
	}

	cinfo->stack_usage = ALIGN_TO (cinfo->stack_usage, MONO_ARCH_FRAME_ALIGNMENT);

	return cinfo;
}

void
mono_arch_create_vars (MonoCompile *cfg)
{
	MonoMethodSignature *sig;
	CallInfo *cinfo;

	sig = mono_method_signature_internal (cfg->method);
	if (!cfg->arch.cinfo)
		cfg->arch.cinfo = get_call_info (cfg->mempool, sig);
	cinfo = cfg->arch.cinfo;

	if (cinfo->ret.storage == ArgVtypeByRef) {
		cfg->vret_addr = mono_compile_create_var (cfg, mono_get_int_type (), OP_LOCAL);
		cfg->vret_addr->flags |= MONO_INST_VOLATILE;
	}

	if (cfg->gen_sdb_seq_points) {
		MonoInst *ins;

		if (cfg->compile_aot) {
			ins = mono_compile_create_var (cfg, mono_get_int_type (), OP_LOCAL);
			ins->flags |= MONO_INST_VOLATILE;
			cfg->arch.seq_point_info_var = ins;
		}

		ins = mono_compile_create_var (cfg, mono_get_int_type (), OP_LOCAL);
		ins->flags |= MONO_INST_VOLATILE;
		cfg->arch.ss_tramp_var = ins;

		ins = mono_compile_create_var (cfg, mono_get_int_type (), OP_LOCAL);
		ins->flags |= MONO_INST_VOLATILE;
		cfg->arch.bp_tramp_var = ins;
	}

	if (cfg->method->save_lmf) {
		cfg->create_lmf_var = TRUE;
		cfg->lmf_ir = TRUE;
	}
}

MonoInst *
mono_arch_emit_inst_for_method (MonoCompile *cfg, MonoMethod *cmethod,
                                MonoMethodSignature *fsig, MonoInst **args)
{
	return NULL;
}

static void
add_outarg_reg (MonoCompile *cfg, MonoCallInst *call, ArgStorage storage, int reg, MonoInst *arg)
{
	MonoInst *ins;

	switch (storage) {
	case ArgInIReg:
		MONO_INST_NEW (cfg, ins, OP_MOVE);
		ins->dreg = mono_alloc_ireg_copy (cfg, arg->dreg);
		ins->sreg1 = arg->dreg;
		MONO_ADD_INS (cfg->cbb, ins);
		mono_call_inst_add_outarg_reg (cfg, call, ins->dreg, reg, FALSE);
		break;
	case ArgInFReg:
		MONO_INST_NEW (cfg, ins, OP_FMOVE);
		ins->dreg = mono_alloc_freg (cfg);
		ins->sreg1 = arg->dreg;
		MONO_ADD_INS (cfg->cbb, ins);
		mono_call_inst_add_outarg_reg (cfg, call, ins->dreg, reg, TRUE);
		break;
	case ArgInFRegR4:
		if (COMPILE_LLVM (cfg))
			MONO_INST_NEW (cfg, ins, OP_FMOVE);
		else if (cfg->r4fp)
			MONO_INST_NEW (cfg, ins, OP_RMOVE);
		else
			NOT_IMPLEMENTED;
			// MONO_INST_NEW (cfg, ins, OP_ARM_SETFREG_R4);
		ins->dreg = mono_alloc_freg (cfg);
		ins->sreg1 = arg->dreg;
		MONO_ADD_INS (cfg->cbb, ins);
		mono_call_inst_add_outarg_reg (cfg, call, ins->dreg, reg, TRUE);
		break;
	default:
		g_assert_not_reached ();
		break;
	}
}

static void
emit_sig_cookie (MonoCompile *cfg, MonoCallInst *call, CallInfo *cinfo)
{
	MonoMethodSignature *tmp_sig;
	int sig_reg;

	if (MONO_IS_TAILCALL_OPCODE (call))
		NOT_IMPLEMENTED;

	g_assert (cinfo->sig_cookie.storage == ArgOnStack);
			
	/*
	 * mono_ArgIterator_Setup assumes the signature cookie is 
	 * passed first and all the arguments which were before it are
	 * passed on the stack after the signature. So compensate by 
	 * passing a different signature.
	 */
	tmp_sig = mono_metadata_signature_dup (call->signature);
	tmp_sig->param_count -= call->signature->sentinelpos;
	tmp_sig->sentinelpos = 0;
	memcpy (tmp_sig->params, call->signature->params + call->signature->sentinelpos, tmp_sig->param_count * sizeof (MonoType*));

	sig_reg = mono_alloc_ireg (cfg);
	MONO_EMIT_NEW_SIGNATURECONST (cfg, sig_reg, tmp_sig);

	MONO_EMIT_NEW_STORE_MEMBASE (cfg, OP_STORE_MEMBASE_REG, RISCV_SP, cinfo->sig_cookie.offset, sig_reg);
}

void
mono_arch_emit_call (MonoCompile *cfg, MonoCallInst *call)
{
	MonoMethodSignature *sig;
	MonoInst *arg, *vtarg;
	CallInfo *cinfo;
	ArgInfo *ainfo;
	int i;

	sig = call->signature;

	cinfo = get_call_info (cfg->mempool, sig);

	switch (cinfo->ret.storage) {
	case ArgVtypeInIRegs:
	case ArgHFA:
		if (MONO_IS_TAILCALL_OPCODE (call))
			break;
		/*
		 * The vtype is returned in registers, save the return area address in a local, and save the vtype into
		 * the location pointed to by it after call in emit_move_return_value ().
		 */
		if (!cfg->arch.vret_addr_loc) {
			cfg->arch.vret_addr_loc = mono_compile_create_var (cfg, mono_get_int_type (), OP_LOCAL);
			/* Prevent it from being register allocated or optimized away */
			cfg->arch.vret_addr_loc->flags |= MONO_INST_VOLATILE;
		}

		MONO_EMIT_NEW_UNALU (cfg, OP_MOVE, cfg->arch.vret_addr_loc->dreg, call->vret_var->dreg);
		break;
	case ArgVtypeByRef:
		/* Pass the vtype return address in R8 */
		g_assert (!MONO_IS_TAILCALL_OPCODE (call) || call->vret_var == cfg->vret_addr);
		MONO_INST_NEW (cfg, vtarg, OP_MOVE);
		vtarg->sreg1 = call->vret_var->dreg;
		vtarg->dreg = mono_alloc_preg (cfg);
		MONO_ADD_INS (cfg->cbb, vtarg);

		mono_call_inst_add_outarg_reg (cfg, call, vtarg->dreg, cinfo->ret.reg, FALSE);
		break;
	default:
		break;
	}

	for (i = 0; i < cinfo->nargs; ++i) {
		ainfo = cinfo->args + i;
		arg = call->args [i];

		if ((sig->call_convention == MONO_CALL_VARARG) && (i == sig->sentinelpos)) {
			/* Emit the signature cookie just before the implicit arguments */
			emit_sig_cookie (cfg, call, cinfo);
		}

		switch (ainfo->storage) {
		case ArgInIReg:
		case ArgInFReg:
		case ArgInFRegR4:
			add_outarg_reg (cfg, call, ainfo->storage, ainfo->reg, arg);
			break;
		case ArgOnStack:
			switch (ainfo->slot_size) {
			case 8:
				MONO_EMIT_NEW_STORE_MEMBASE (cfg, OP_STORE_MEMBASE_REG, RISCV_SP, ainfo->offset, arg->dreg);
				break;
			case 4:
				MONO_EMIT_NEW_STORE_MEMBASE (cfg, OP_STOREI4_MEMBASE_REG, RISCV_SP, ainfo->offset, arg->dreg);
				break;
			case 2:
				MONO_EMIT_NEW_STORE_MEMBASE (cfg, OP_STOREI2_MEMBASE_REG, RISCV_SP, ainfo->offset, arg->dreg);
				break;
			case 1:
				MONO_EMIT_NEW_STORE_MEMBASE (cfg, OP_STOREI1_MEMBASE_REG, RISCV_SP, ainfo->offset, arg->dreg);
				break;
			default:
				g_assert_not_reached ();
				break;
			}
			break;
		case ArgOnStackR8:
			MONO_EMIT_NEW_STORE_MEMBASE (cfg, OP_STORER8_MEMBASE_REG, RISCV_SP, ainfo->offset, arg->dreg);
			break;
		case ArgOnStackR4:
			MONO_EMIT_NEW_STORE_MEMBASE (cfg, OP_STORER4_MEMBASE_REG, RISCV_SP, ainfo->offset, arg->dreg);
			break;
		case ArgVtypeInIRegs:
		case ArgVtypeByRef:
		case ArgVtypeByRefOnStack:
		case ArgVtypeOnStack:
		case ArgHFA: {
			MonoInst *ins;
			guint32 align;
			guint32 size;

			size = mono_class_value_size (arg->klass, &align);

			MONO_INST_NEW (cfg, ins, OP_OUTARG_VT);
			ins->sreg1 = arg->dreg;
			ins->klass = arg->klass;
			ins->backend.size = size;
			ins->inst_p0 = call;
			ins->inst_p1 = mono_mempool_alloc (cfg->mempool, sizeof (ArgInfo));
			memcpy (ins->inst_p1, ainfo, sizeof (ArgInfo));
			MONO_ADD_INS (cfg->cbb, ins);
			break;
		}
		default:
			g_assert_not_reached ();
			break;
		}
	}

	/* Handle the case where there are no implicit arguments */
	if (!sig->pinvoke && (sig->call_convention == MONO_CALL_VARARG) && (cinfo->nargs == sig->sentinelpos))
		emit_sig_cookie (cfg, call, cinfo);

	call->call_info = cinfo;
	call->stack_usage = cinfo->stack_usage;
}

void
mono_arch_emit_outarg_vt (MonoCompile *cfg, MonoInst *ins, MonoInst *src)
{
	NOT_IMPLEMENTED;
}

void
mono_arch_emit_setret (MonoCompile *cfg, MonoMethod *method, MonoInst *val)
{
	NOT_IMPLEMENTED;
}

void
mono_arch_decompose_opts (MonoCompile *cfg, MonoInst *ins)
{
	NOT_IMPLEMENTED;
}

void
mono_arch_decompose_long_opts (MonoCompile *cfg, MonoInst *long_ins)
{
#ifdef TARGET_RISCV32
	NOT_IMPLEMENTED;
#endif
}

void
mono_arch_allocate_vars (MonoCompile *cfg)
{
	MonoMethodSignature *sig;
	MonoInst *ins;
	CallInfo *cinfo;
	ArgInfo *ainfo;
	int i, offset, size, align;
	guint32 locals_stack_size, locals_stack_align;
	gint32 *offsets;

	/*
	 * Allocate arguments and locals to either register (OP_REGVAR) or to a stack slot (OP_REGOFFSET).
	 * Compute cfg->stack_offset and update cfg->used_int_regs.
	 */

	sig = mono_method_signature_internal (cfg->method);

	if (!cfg->arch.cinfo)
		cfg->arch.cinfo = get_call_info (cfg->mempool, sig);
	cinfo = cfg->arch.cinfo;

	/*
	 * The ARM64 ABI always uses a frame pointer.
	 * The instruction set prefers positive offsets, so fp points to the bottom of the
	 * frame, and stack slots are at positive offsets.
	 * If some arguments are received on the stack, their offsets relative to fp can
	 * not be computed right now because the stack frame might grow due to spilling
	 * done by the local register allocator. To solve this, we reserve a register
	 * which points to them.
	 * The stack frame looks like this:
	 * args_reg -> <bottom of parent frame>
	 *             <locals etc>
	 *       fp -> <saved fp+lr>
     *       sp -> <localloc/params area>
	 */
	cfg->frame_reg = RISCV_FP;
	cfg->flags |= MONO_CFG_HAS_SPILLUP;
	offset = 0;

	/* Saved fp+lr */
	offset += 16;

	if (cinfo->stack_usage) {
		/* TODO: check if this is needed for RISC-V */
		g_assert (!(cfg->used_int_regs & (1 << RISCV_X28)));
		cfg->arch.args_reg = RISCV_X28;
		cfg->used_int_regs |= 1 << RISCV_X28;
	}

	if (cfg->method->save_lmf) {
		/* The LMF var is allocated normally */
	} else {
		/* Callee saved regs */
		cfg->arch.saved_gregs_offset = offset;
		for (i = 0; i < 32; ++i)
			if ((MONO_ARCH_CALLEE_SAVED_REGS & (1 << i)) && (cfg->used_int_regs & (1 << i)))
				offset += 8;
	}

	/* Return value */
	switch (cinfo->ret.storage) {
	case ArgNone:
		break;
	case ArgInIReg:
	case ArgInFReg:
	case ArgInFRegR4:
		cfg->ret->opcode = OP_REGVAR;
		cfg->ret->dreg = cinfo->ret.reg;
		break;
	case ArgVtypeInIRegs:
	case ArgHFA:
		/* TODO: remove for RISC-V? */
		/* Allocate a local to hold the result, the epilog will copy it to the correct place */
		cfg->ret->opcode = OP_REGOFFSET;
		cfg->ret->inst_basereg = cfg->frame_reg;
		cfg->ret->inst_offset = offset;
		if (cinfo->ret.storage == ArgHFA)
			// FIXME:
			offset += 64;
		else
			offset += 16;
		break;
	case ArgVtypeByRef:
		/* This variable will be initalized in the prolog from R8 */
		cfg->vret_addr->opcode = OP_REGOFFSET;
		cfg->vret_addr->inst_basereg = cfg->frame_reg;
		cfg->vret_addr->inst_offset = offset;
		offset += 8;
		if (G_UNLIKELY (cfg->verbose_level > 1)) {
			printf ("vret_addr =");
			mono_print_ins (cfg->vret_addr);
		}
		break;
	default:
		g_assert_not_reached ();
		break;
	}

	/* Arguments */
	for (i = 0; i < sig->param_count + sig->hasthis; ++i) {
		ainfo = cinfo->args + i;

		ins = cfg->args [i];
		if (ins->opcode == OP_REGVAR)
			continue;

		ins->opcode = OP_REGOFFSET;
		ins->inst_basereg = cfg->frame_reg;

		switch (ainfo->storage) {
		case ArgInIReg:
		case ArgInFReg:
		case ArgInFRegR4:
			// FIXME: Use nregs/size
			/* These will be copied to the stack in the prolog */
			ins->inst_offset = offset;
			offset += 8;
			break;
		case ArgOnStack:
		case ArgOnStackR4:
		case ArgOnStackR8:
		case ArgVtypeOnStack:
			/* These are in the parent frame */
			g_assert (cfg->arch.args_reg);
			ins->inst_basereg = cfg->arch.args_reg;
			ins->inst_offset = ainfo->offset;
			break;
		case ArgVtypeInIRegs:
		case ArgHFA:
			/* TODO: remove for RISC-V? */
			ins->opcode = OP_REGOFFSET;
			ins->inst_basereg = cfg->frame_reg;
			/* These arguments are saved to the stack in the prolog */
			ins->inst_offset = offset;
			if (cfg->verbose_level >= 2)
				printf ("arg %d allocated to %s+0x%0x.\n", i, mono_arch_regname (ins->inst_basereg), (int)ins->inst_offset);
			if (ainfo->storage == ArgHFA)
				// FIXME:
				offset += 64;
			else
				offset += 16;
			break;
		case ArgVtypeByRefOnStack: {
			MonoInst *vtaddr;

			if (ainfo->gsharedvt) {
				ins->opcode = OP_REGOFFSET;
				ins->inst_basereg = cfg->arch.args_reg;
				ins->inst_offset = ainfo->offset;
				break;
			}

			/* The vtype address is in the parent frame */
			g_assert (cfg->arch.args_reg);
			MONO_INST_NEW (cfg, vtaddr, 0);
			vtaddr->opcode = OP_REGOFFSET;
			vtaddr->inst_basereg = cfg->arch.args_reg;
			vtaddr->inst_offset = ainfo->offset;

			/* Need an indirection */
			ins->opcode = OP_VTARG_ADDR;
			ins->inst_left = vtaddr;
			break;
		}
		case ArgVtypeByRef: {
			MonoInst *vtaddr;

			if (ainfo->gsharedvt) {
				ins->opcode = OP_REGOFFSET;
				ins->inst_basereg = cfg->frame_reg;
				ins->inst_offset = offset;
				offset += 8;
				break;
			}

			/* The vtype address is in a register, will be copied to the stack in the prolog */
			MONO_INST_NEW (cfg, vtaddr, 0);
			vtaddr->opcode = OP_REGOFFSET;
			vtaddr->inst_basereg = cfg->frame_reg;
			vtaddr->inst_offset = offset;
			offset += 8;

			/* Need an indirection */
			ins->opcode = OP_VTARG_ADDR;
			ins->inst_left = vtaddr;
			break;
		}
		default:
			g_assert_not_reached ();
			break;
		}
	}

	/* Allocate these first so they have a small offset, OP_SEQ_POINT depends on this */
	// FIXME: Allocate these to registers
	ins = cfg->arch.seq_point_info_var;
	if (ins) {
		size = 8;
		align = 8;
		offset += align - 1;
		offset &= ~(align - 1);
		ins->opcode = OP_REGOFFSET;
		ins->inst_basereg = cfg->frame_reg;
		ins->inst_offset = offset;
		offset += size;
	}
	ins = cfg->arch.ss_tramp_var;
	if (ins) {
		size = 8;
		align = 8;
		offset += align - 1;
		offset &= ~(align - 1);
		ins->opcode = OP_REGOFFSET;
		ins->inst_basereg = cfg->frame_reg;
		ins->inst_offset = offset;
		offset += size;
	}
	ins = cfg->arch.bp_tramp_var;
	if (ins) {
		size = 8;
		align = 8;
		offset += align - 1;
		offset &= ~(align - 1);
		ins->opcode = OP_REGOFFSET;
		ins->inst_basereg = cfg->frame_reg;
		ins->inst_offset = offset;
		offset += size;
	}

	/* Locals */
	offsets = mono_allocate_stack_slots (cfg, FALSE, &locals_stack_size, &locals_stack_align);
	if (locals_stack_align)
		offset = ALIGN_TO (offset, locals_stack_align);

	for (i = cfg->locals_start; i < cfg->num_varinfo; i++) {
		if (offsets [i] != -1) {
			ins = cfg->varinfo [i];
			ins->opcode = OP_REGOFFSET;
			ins->inst_basereg = cfg->frame_reg;
			ins->inst_offset = offset + offsets [i];
			// printf ("allocated local %d to ", i); mono_print_ins (ins);
		}
	}
	offset += locals_stack_size;

	offset = ALIGN_TO (offset, MONO_ARCH_FRAME_ALIGNMENT);

	cfg->stack_offset = offset;
}

void
mono_arch_lowering_pass (MonoCompile *cfg, MonoBasicBlock *bb)
{
	MonoInst *ins, *temp, *last_ins = NULL;

	MONO_BB_FOR_EACH_INS (bb, ins) {
		switch (ins->opcode) {
		case OP_SBB:
		case OP_ISBB:
		case OP_SUBCC:
		case OP_ISUBCC:
			NOT_IMPLEMENTED;
			break;
		case OP_IDIV_IMM:
		case OP_IREM_IMM:
		case OP_IDIV_UN_IMM:
		case OP_IREM_UN_IMM:
		case OP_LREM_IMM:
			mono_decompose_op_imm (cfg, bb, ins);
			break;
		case OP_LOCALLOC_IMM:
			if (ins->inst_imm > 32) {
#define ADD_NEW_INS(cfg,dest,op) do {       \
		MONO_INST_NEW ((cfg), (dest), (op)); \
        mono_bblock_insert_before_ins (bb, ins, (dest)); \
	} while (0)
				ADD_NEW_INS (cfg, temp, OP_ICONST);
				temp->inst_c0 = ins->inst_imm;
				temp->dreg = mono_alloc_ireg (cfg);
				ins->sreg1 = temp->dreg;
				ins->opcode = mono_op_imm_to_op (ins->opcode);
			}
			break;
		case OP_ICOMPARE_IMM:
			NOT_IMPLEMENTED;
			if (ins->inst_imm == 0 && ins->next && ins->next->opcode == OP_IBEQ) {
				// ins->next->opcode = OP_ARM64_CBZW;
				ins->next->sreg1 = ins->sreg1;
				NULLIFY_INS (ins);
			} else if (ins->inst_imm == 0 && ins->next && ins->next->opcode == OP_IBNE_UN) {
				// ins->next->opcode = OP_ARM64_CBNZW;
				ins->next->sreg1 = ins->sreg1;
				NULLIFY_INS (ins);
			}
			break;
		case OP_LCOMPARE_IMM:
		case OP_COMPARE_IMM:
			NOT_IMPLEMENTED;
			if (ins->inst_imm == 0 && ins->next && ins->next->opcode == OP_LBEQ) {
				// ins->next->opcode = OP_ARM64_CBZX;
				ins->next->sreg1 = ins->sreg1;
				NULLIFY_INS (ins);
			} else if (ins->inst_imm == 0 && ins->next && ins->next->opcode == OP_LBNE_UN) {
				// ins->next->opcode = OP_ARM64_CBNZX;
				ins->next->sreg1 = ins->sreg1;
				NULLIFY_INS (ins);
			}
			break;
		case OP_FCOMPARE:
		case OP_RCOMPARE: {
			NOT_IMPLEMENTED;
			gboolean swap = FALSE;
			int reg;

			if (!ins->next) {
				/* Optimized away */
				NULLIFY_INS (ins);
				break;
			}

			/*
			 * FP compares with unordered operands set the flags
			 * to NZCV=0011, which matches some non-unordered compares
			 * as well, like LE, so have to swap the operands.
			 */
			switch (ins->next->opcode) {
			case OP_FBLT:
				ins->next->opcode = OP_FBGT;
				swap = TRUE;
				break;
			case OP_FBLE:
				ins->next->opcode = OP_FBGE;
				swap = TRUE;
				break;
			case OP_RBLT:
				ins->next->opcode = OP_RBGT;
				swap = TRUE;
				break;
			case OP_RBLE:
				ins->next->opcode = OP_RBGE;
				swap = TRUE;
				break;
			default:
				break;
			}
			if (swap) {
				reg = ins->sreg1;
				ins->sreg1 = ins->sreg2;
				ins->sreg2 = reg;
			}
			break;
		}
		default:
			break;
		}

		last_ins = ins;
	}
	bb->last_ins = last_ins;
	bb->max_vreg = cfg->next_vreg;
}

void
mono_arch_peephole_pass_1 (MonoCompile *cfg, MonoBasicBlock *bb)
{
}

void
mono_arch_peephole_pass_2 (MonoCompile *cfg, MonoBasicBlock *bb)
{
}

// Uses at most 8 bytes on RV32I and 16 bytes on RV64I.
guint8 *
mono_riscv_emit_imm (guint8 *code, int rd, gsize imm)
{
#ifdef TARGET_RISCV64
	if (RISCV_VALID_I_IMM (imm)) {
		riscv_addi (code, rd, RISCV_ZERO, imm);
		return code;
	}

	/*
	 * This is not pretty, but RV64I doesn't make it easy to load constants.
	 * Need to figure out something better.
	 */
	riscv_jal (code, rd, sizeof (guint64));
	*(guint64 *) code = imm;
	code += sizeof (guint64);
	riscv_ld (code, rd, rd, 0);
#else
	if (RISCV_VALID_I_IMM (imm)) {
		riscv_addi (code, rd, RISCV_ZERO, imm);
		return code;
	}

	riscv_lui (code, rd, RISCV_BITS (imm, 12, 20));

	if (!RISCV_VALID_U_IMM (imm))
		riscv_ori (code, rd, rd, RISCV_BITS (imm, 0, 12));
#endif

	return code;
}

// Uses at most 16 bytes on RV32I and 24 bytes on RV64I.
guint8 *
mono_riscv_emit_load (guint8 *code, int rd, int rs1, gint32 imm)
{
	if (RISCV_VALID_I_IMM (imm)) {
#ifdef TARGET_RISCV64
		riscv_ld (code, rd, rs1, imm);
#else
		riscv_lw (code, rd, rs1, imm);
#endif
	} else {
		code = mono_riscv_emit_imm (code, rd, imm);
		riscv_add (code, rd, rs1, rd);
#ifdef TARGET_RISCV64
		riscv_ld (code, rd, rd, 0);
#else
		riscv_lw (code, rd, rd, 0);
#endif
	}

	return code;
}

// May clobber t1. Uses at most 16 bytes on RV32I and 24 bytes on RV64I.
guint8 *
mono_riscv_emit_store (guint8 *code, int rs1, int rs2, gint32 imm)
{
	if (RISCV_VALID_S_IMM (imm)) {
#ifdef TARGET_RISCV64
		riscv_sd (code, rs1, rs2, imm);
#else
		riscv_sw (code, rs1, rs2, imm);
#endif
	} else {
		code = mono_riscv_emit_imm (code, RISCV_T1, imm);
		riscv_add (code, RISCV_T1, rs2, RISCV_T1);
#ifdef TARGET_RISCV64
		riscv_sd (code, rs1, RISCV_T1, 0);
#else
		riscv_sw (code, rs1, RISCV_T1, 0);
#endif
	}

	return code;
}

// May clobber t1.
guint8 *
mono_riscv_emit_add_imm (guint8 *code, int rd, int rs1, gint32 imm)
{
	if (RISCV_VALID_S_IMM (imm)) {
		riscv_addi (code, rd, rs1, imm);
	} else {
		code = mono_riscv_emit_imm (code, RISCV_T1, imm);
		riscv_add (code, rd, rs1, RISCV_T1);
	}

	return code;
}

/*
 * emit_setup_lmf:
 *
 *   Emit code to initialize an LMF structure at LMF_OFFSET.
 */
static guint8*
emit_setup_lmf (MonoCompile *cfg, guint8 *code, gint32 lmf_offset, int cfa_offset)
{
	/*
	 * The LMF should contain all the state required to be able to reconstruct the machine state
	 * at the current point of execution. Since the LMF is only read during EH, only callee
	 * saved etc. registers need to be saved.
	 * FIXME: Save callee saved fp regs, JITted code doesn't use them, but native code does, and they
	 * need to be restored during EH.
	 */

	NOT_IMPLEMENTED;
#if 0
	/* pc */
	arm_adrx (code, ARMREG_LR, code);
	code = emit_strx (code, ARMREG_LR, ARMREG_FP, lmf_offset + MONO_STRUCT_OFFSET (MonoLMF, pc));
	/* gregs + fp + sp */
	/* Don't emit unwind info for sp/fp, they are already handled in the prolog */
	code = emit_store_regset_cfa (cfg, code, MONO_ARCH_LMF_REGS, ARMREG_FP, lmf_offset + MONO_STRUCT_OFFSET (MonoLMF, gregs), cfa_offset, (1 << ARMREG_FP) | (1 << ARMREG_SP));

#endif
	return code;
}
/* Same as emit_store_regset, but emit unwind info too */
/* CFA_OFFSET is the offset between the CFA and basereg */
static __attribute__ ((__warn_unused_result__)) guint8*
emit_store_regset_cfa (MonoCompile *cfg, guint8 *code, guint64 regs, int basereg, int offset, int cfa_offset, guint64 no_cfa_regset)
{
	int i, j, pos, nregs;
	guint32 cfa_regset = regs & ~no_cfa_regset;

	pos = 0;
	for (i = 0; i < RISCV_N_GREGS; ++i) {
		nregs = 1;
		if (regs & (1 << i)) {
			code = mono_riscv_emit_store (code, i, basereg, offset + (pos * RISCV_XLEN / 8));
			if (i == RISCV_SP) {
				// TODO: does this need any special casing?
				NOT_IMPLEMENTED;
			}

			for (j = 0; j < nregs; ++j) {
				if (cfa_regset & (1 << (i + j)))
					mono_emit_unwind_op_offset (cfg, code, i + j, (- cfa_offset) + offset + ((pos + j) * 8));
			}

			i += nregs - 1;
			pos += nregs;
		}
	}
	return code;
}


static guint8*
emit_move_args (MonoCompile *cfg, guint8 *code)
{
	MonoInst *ins;
	CallInfo *cinfo;
	ArgInfo *ainfo;
	int i, part;
	MonoMethodSignature *sig = mono_method_signature_internal (cfg->method);

	cinfo = cfg->arch.cinfo;
	g_assert (cinfo);
	for (i = 0; i < cinfo->nargs; ++i) {
		ainfo = cinfo->args + i;
		ins = cfg->args [i];

		if (ins->opcode == OP_REGVAR) {
			switch (ainfo->storage) {
			case ArgInIReg:
				riscv_addi (code, ins->dreg, ainfo->reg, 0);
				if (i == 0 && sig->hasthis) {
					mono_add_var_location (cfg, ins, TRUE, ainfo->reg, 0, 0, code - cfg->native_code);
					mono_add_var_location (cfg, ins, TRUE, ins->dreg, 0, code - cfg->native_code, 0);
				}
				break;
			case ArgOnStack:
				NOT_IMPLEMENTED;
#if 0
				switch (ainfo->slot_size) {
				case 1:
					if (ainfo->sign)
						code = emit_ldrsbx (code, ins->dreg, cfg->arch.args_reg, ainfo->offset);
					else
						code = emit_ldrb (code, ins->dreg, cfg->arch.args_reg, ainfo->offset);
					break;
				case 2:
					if (ainfo->sign)
						code = emit_ldrshx (code, ins->dreg, cfg->arch.args_reg, ainfo->offset);
					else
						code = emit_ldrh (code, ins->dreg, cfg->arch.args_reg, ainfo->offset);
					break;
				case 4:
					if (ainfo->sign)
						code = emit_ldrswx (code, ins->dreg, cfg->arch.args_reg, ainfo->offset);
					else
						code = emit_ldrw (code, ins->dreg, cfg->arch.args_reg, ainfo->offset);
					break;
				default:
					code = emit_ldrx (code, ins->dreg, cfg->arch.args_reg, ainfo->offset);
					break;
				}
				break;
#endif
			default:
				g_assert_not_reached ();
				break;
			}
		} else {
			if (ainfo->storage != ArgVtypeByRef && ainfo->storage != ArgVtypeByRefOnStack)
				g_assert (ins->opcode == OP_REGOFFSET);

			switch (ainfo->storage) {
			case ArgInIReg:
				/* Stack slots for arguments have size 8 */
				code = mono_riscv_emit_store (code, ainfo->reg, ins->inst_basereg, ins->inst_offset);
				if (i == 0 && sig->hasthis) {
					mono_add_var_location (cfg, ins, TRUE, ainfo->reg, 0, 0, code - cfg->native_code);
					mono_add_var_location (cfg, ins, FALSE, ins->inst_basereg, ins->inst_offset, code - cfg->native_code, 0);
				}
				break;
#if 0
			case ArgInFReg:
				code = emit_strfpx (code, ainfo->reg, ins->inst_basereg, ins->inst_offset);
				break;
			case ArgInFRegR4:
				code = emit_strfpw (code, ainfo->reg, ins->inst_basereg, ins->inst_offset);
				break;
			case ArgOnStack:
			case ArgOnStackR4:
			case ArgOnStackR8:
			case ArgVtypeByRefOnStack:
			case ArgVtypeOnStack:
				break;
			case ArgVtypeByRef: {
				MonoInst *addr_arg = ins->inst_left;

				if (ainfo->gsharedvt) {
					g_assert (ins->opcode == OP_GSHAREDVT_ARG_REGOFFSET);
					arm_strx (code, ainfo->reg, ins->inst_basereg, ins->inst_offset);
				} else {
					g_assert (ins->opcode == OP_VTARG_ADDR);
					g_assert (addr_arg->opcode == OP_REGOFFSET);
					arm_strx (code, ainfo->reg, addr_arg->inst_basereg, addr_arg->inst_offset);
				}
				break;
			}
			case ArgVtypeInIRegs:
				for (part = 0; part < ainfo->nregs; part ++) {
					code = emit_strx (code, ainfo->reg + part, ins->inst_basereg, ins->inst_offset + (part * 8));
				}
				break;
			case ArgHFA:
				for (part = 0; part < ainfo->nregs; part ++) {
					if (ainfo->esize == 4)
						code = emit_strfpw (code, ainfo->reg + part, ins->inst_basereg, ins->inst_offset + ainfo->foffsets [part]);
					else
						code = emit_strfpx (code, ainfo->reg + part, ins->inst_basereg, ins->inst_offset + ainfo->foffsets [part]);
				}
				break;
#endif
			default:
				g_assert_not_reached ();
				break;
			}
		}
	}

	return code;
}

guint8 *
mono_arch_emit_prolog (MonoCompile *cfg)
{
	MonoMethod *method = cfg->method;
	MonoMethodSignature *sig;
	MonoBasicBlock *bb;
	guint8 *code;
	int cfa_offset, max_offset;

	sig = mono_method_signature_internal (method);
	cfg->code_size = 256 + sig->param_count * 64;
	code = cfg->native_code = g_malloc (cfg->code_size);

	/* This can be unaligned */
	cfg->stack_offset = ALIGN_TO (cfg->stack_offset, MONO_ARCH_FRAME_ALIGNMENT);

	/*
	 * - Setup frame
	 */
	cfa_offset = 0;
	mono_emit_unwind_op_def_cfa (cfg, code, RISCV_SP, 0);

	/* Setup frame */
	riscv_addi (code, RISCV_SP, RISCV_SP, -cfg->stack_offset);
	code = mono_riscv_emit_store (code, RISCV_RA, RISCV_SP, RISCV_XLEN / 8);
	cfa_offset += cfg->stack_offset;
	mono_emit_unwind_op_def_cfa_offset (cfg, code, cfa_offset);
	mono_emit_unwind_op_offset (cfg, code, RISCV_FP, (- cfa_offset) + 0);
	mono_emit_unwind_op_offset (cfg, code, RISCV_RA, (- cfa_offset) + RISCV_XLEN / 8);
	riscv_addi (code, RISCV_FP, RISCV_SP, 0);
	mono_emit_unwind_op_def_cfa_reg (cfg, code, RISCV_FP);

	if (cfg->param_area) {
		NOT_IMPLEMENTED;
		// ???
		/* The param area is below the frame pointer */
		// code = emit_subx_sp_imm (code, cfg->param_area);
	}

	if (cfg->method->save_lmf) {
		code = emit_setup_lmf (cfg, code, cfg->lmf_var->inst_offset, cfa_offset);
	} else {
		/* Save gregs */
		code = emit_store_regset_cfa (cfg, code, MONO_ARCH_CALLEE_SAVED_REGS & cfg->used_int_regs, RISCV_SP, cfg->arch.saved_gregs_offset, cfa_offset, 0);
	}

	/* Setup args reg */
	if (cfg->arch.args_reg) {
		/* The register was already saved above */
		code = mono_riscv_emit_add_imm (code, cfg->arch.args_reg, RISCV_FP, cfg->stack_offset);
	}

	/* Save return area addr received in R8 */
	if (cfg->vret_addr) {
		NOT_IMPLEMENTED;
#if 0
		MonoInst *ins = cfg->vret_addr;

		g_assert (ins->opcode == OP_REGOFFSET);
		code = emit_strx (code, ARMREG_R8, ins->inst_basereg, ins->inst_offset);
#endif
	}

	/* Save mrgctx received in MONO_ARCH_RGCTX_REG */
	if (cfg->rgctx_var) {
		NOT_IMPLEMENTED;
#if 0
		MonoInst *ins = cfg->rgctx_var;

		g_assert (ins->opcode == OP_REGOFFSET);

		code = emit_strx (code, MONO_ARCH_RGCTX_REG, ins->inst_basereg, ins->inst_offset); 

		mono_add_var_location (cfg, cfg->rgctx_var, TRUE, MONO_ARCH_RGCTX_REG, 0, 0, code - cfg->native_code);
		mono_add_var_location (cfg, cfg->rgctx_var, FALSE, ins->inst_basereg, ins->inst_offset, code - cfg->native_code, 0);
#endif
	}
		
	/*
	 * Move arguments to their registers/stack locations.
	 */
	code = emit_move_args (cfg, code);

	/* Initialize seq_point_info_var */
	if (cfg->arch.seq_point_info_var) {
		NOT_IMPLEMENTED;
#if 0
		MonoInst *ins = cfg->arch.seq_point_info_var;

		/* Initialize the variable from a GOT slot */
		code = emit_aotconst (cfg, code, ARMREG_IP0, MONO_PATCH_INFO_SEQ_POINT_INFO, cfg->method);
		g_assert (ins->opcode == OP_REGOFFSET);
		code = emit_strx (code, ARMREG_IP0, ins->inst_basereg, ins->inst_offset);

		/* Initialize ss_tramp_var */
		ins = cfg->arch.ss_tramp_var;
		g_assert (ins->opcode == OP_REGOFFSET);

		code = emit_ldrx (code, ARMREG_IP1, ARMREG_IP0, MONO_STRUCT_OFFSET (SeqPointInfo, ss_tramp_addr));
		code = emit_strx (code, ARMREG_IP1, ins->inst_basereg, ins->inst_offset);
#endif
	} else {
		MonoInst *ins;

		if (cfg->arch.ss_tramp_var) {
			/* Initialize ss_tramp_var */
			ins = cfg->arch.ss_tramp_var;
			g_assert (ins->opcode == OP_REGOFFSET);

			code = mono_riscv_emit_imm (code, RISCV_T0, (guint64)&ss_trampoline);
			code = mono_riscv_emit_store (code, RISCV_T0, ins->inst_basereg, ins->inst_offset);
		}

		if (cfg->arch.bp_tramp_var) {
			/* Initialize bp_tramp_var */
			ins = cfg->arch.bp_tramp_var;
			g_assert (ins->opcode == OP_REGOFFSET);

			code = mono_riscv_emit_imm (code, RISCV_T0, (guint64)&bp_trampoline);
			code = mono_riscv_emit_store (code, RISCV_T0, ins->inst_basereg, ins->inst_offset);
		}
	}

	max_offset = 0;
	if (cfg->opt & MONO_OPT_BRANCH) {
		for (bb = cfg->bb_entry; bb; bb = bb->next_bb) {
			MonoInst *ins;
			bb->max_offset = max_offset;

			MONO_BB_FOR_EACH_INS (bb, ins) {
				max_offset += ins_get_size (ins->opcode);
			}
		}
	}
	if (max_offset > 0x3ffff * 4)
		cfg->arch.cond_branch_islands = TRUE;

	return code;
}

void
mono_arch_emit_epilog (MonoCompile *cfg)
{
	NOT_IMPLEMENTED;
}

void
mono_arch_output_basic_block (MonoCompile *cfg, MonoBasicBlock *bb)
{
	MonoInst *ins;
	MonoCallInst *call;
	guint8 *code = cfg->native_code + cfg->code_len;
	int start_offset, max_len, dreg, sreg1, sreg2;
	target_mgreg_t imm;

	if (cfg->verbose_level > 2)
		g_print ("Basic block %d starting at offset 0x%x\n", bb->block_num, bb->native_offset);

	start_offset = code - cfg->native_code;
	g_assert (start_offset <= cfg->code_size);

	MONO_BB_FOR_EACH_INS (bb, ins) {
		guint offset = code - cfg->native_code;
		set_code_cursor (cfg, code);
		max_len = ins_get_size (ins->opcode);
		code = realloc_code (cfg, max_len);

		mono_debug_record_line_number (cfg, ins, offset);

		dreg = ins->dreg;
		sreg1 = ins->sreg1;
		sreg2 = ins->sreg2;
		imm = ins->inst_imm;

		switch (ins->opcode) {
		case OP_ICONST:
		case OP_I8CONST:
			code = mono_riscv_emit_imm (code, dreg, ins->inst_c0);
			break;
		case OP_LOCALLOC: {
			guint8 *buf [16];

			riscv_addi (code, RISCV_T0, sreg1, (MONO_ARCH_FRAME_ALIGNMENT - 1));
#if 1
			code = mono_riscv_emit_imm (code, RISCV_T1, sreg1, -MONO_ARCH_FRAME_ALIGNMENT));
			riscv_and (code, RISCV_T0, RISCV_T0, RISCV_T1);
#else
			// TODO: does proper sign extending work?
			riscv_andi (code, RISCV_T0, RISCV_T0, -MONO_ARCH_FRAME_ALIGNMENT);
#endif
			riscv_sub (code, RISCV_SP, RISCV_SP, RISCV_T0);
			riscv_addi (code, RISCV_T1, RISCV_SP, 0);

			/* Init */
			/* t1 = pointer, t0 = end */
			riscv_add (code, RISCV_T0, RISCV_T1, RISCV_T0);

			buf [0] = code;
			// TODO: patch me!
			riscv_beq (code, RISCV_T1, RISCV_T0, 0); // if true, branch is taken

			// wrong branch here
			buf [1] = code;
			arm_bcc (code, ARMCOND_EQ, 0);
			arm_stpx (code, ARMREG_RZR, ARMREG_RZR, RISCV_T1, 0);
			arm_addx_imm (code, RISCV_T1, RISCV_T1, 16);
			arm_b (code, buf [0]);
			arm_patch_rel (buf [1], code, MONO_R_ARM64_BCC);
			// true branch here

			arm_movspx (code, dreg, ARMREG_SP);
			if (cfg->param_area)
				code = emit_subx_sp_imm (code, cfg->param_area);
			break;
		}
		case OP_LOCALLOC_IMM: {
			int imm, offset;

			imm = ALIGN_TO (ins->inst_imm, MONO_ARCH_FRAME_ALIGNMENT);
			g_assert (arm_is_arith_imm (imm));
			arm_subx_imm (code, ARMREG_SP, ARMREG_SP, imm);

			/* Init */
			g_assert (MONO_ARCH_FRAME_ALIGNMENT == 16);
			offset = 0;
			while (offset < imm) {
				arm_stpx (code, ARMREG_RZR, ARMREG_RZR, ARMREG_SP, offset);
				offset += 16;
			}
			arm_movspx (code, dreg, ARMREG_SP);
			if (cfg->param_area)
				code = emit_subx_sp_imm (code, cfg->param_area);
			break;
		}
		default:
			g_warning ("unknown opcode %s in %s()\n", mono_inst_name (ins->opcode), __FUNCTION__);
			g_assert_not_reached ();
		}

		if ((cfg->opt & MONO_OPT_BRANCH) && ((code - cfg->native_code - offset) > max_len)) {
			g_warning ("wrong maximal instruction length of instruction %s (expected %d, got %d)",
				   mono_inst_name (ins->opcode), max_len, code - cfg->native_code - offset);
			g_assert_not_reached ();
		}
	}
	set_code_cursor (cfg, code);
}

void
mono_arch_emit_exceptions (MonoCompile *cfg)
{
	NOT_IMPLEMENTED;
}

guint32
mono_arch_get_patch_offset (guint8 *code)
{
	NOT_IMPLEMENTED;
	return 0;
}

GSList *
mono_arch_get_trampolines (gboolean aot)
{
	NOT_IMPLEMENTED;
	return NULL;
}

#endif

#if defined(MONO_ARCH_SOFT_DEBUG_SUPPORTED)
void
mono_arch_set_breakpoint (MonoJitInfo *ji, guint8 *ip)
{
	NOT_IMPLEMENTED;
}

void
mono_arch_clear_breakpoint (MonoJitInfo *ji, guint8 *ip)
{
	NOT_IMPLEMENTED;
}

void
mono_arch_start_single_stepping (void)
{
	ss_trampoline = mini_get_single_step_trampoline ();
}

void
mono_arch_stop_single_stepping (void)
{
	ss_trampoline = NULL;
}

gboolean
mono_arch_is_single_step_event (void *info, void *sigctx)
{
	NOT_IMPLEMENTED;
	return FALSE;
}

gboolean
mono_arch_is_breakpoint_event (void *info, void *sigctx)
{
	NOT_IMPLEMENTED;
	return FALSE;
}

void
mono_arch_skip_breakpoint (MonoContext *ctx, MonoJitInfo *ji)
{
	NOT_IMPLEMENTED;
}

void
mono_arch_skip_single_step (MonoContext *ctx)
{
	NOT_IMPLEMENTED;
}

gpointer
mono_arch_get_seq_point_info (MonoDomain *domain, guint8 *code)
{
	NOT_IMPLEMENTED;
	return NULL;
}
#endif /* MONO_ARCH_SOFT_DEBUG_SUPPORTED */

gpointer
mono_arch_load_function (MonoJitICallId jit_icall_id)
{
	return NULL;
}
