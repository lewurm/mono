using System;

using Mono.Compiler;
using SimpleJit.Metadata;
using SimpleJit.CIL;

using LLVMSharp;

/// <summary>
///   Compile from CIL to LLVM IR (and then to native code) in one big step
///   (without using our own intermediate representation).
///
///   Basically mimic the Kaleidoscope tutorial.
/// </summary>
namespace Mono.Compiler.BigStep
{
	public class BigStep
	{
		// FIXME
		const string TargetTriple = "x86_64-apple-macosx10.12.3";

		const CompilationResult Ok = CompilationResult.Ok;
		CompilationFlags Flags { get; }
		IRuntimeInformation RuntimeInfo { get; }

		public BigStep (IRuntimeInformation runtimeInfo, CompilationFlags flags)
		{
			this.Flags = flags;
			this.RuntimeInfo = runtimeInfo;
		}

		public CompilationResult CompileMethod (MethodInfo methodInfo, out NativeCodeHandle result)
		{
			var builder = new Builder ();
			var env = new Env (RuntimeInfo, methodInfo);

			Preamble (env, builder);

			result = NativeCodeHandle.Invalid;
			var r = TranslateBody (env, builder, methodInfo.Body);
			if (r != Ok)
				return r;
			r = builder.Finish (out result);
			return r;
		}

		// translation environment for a single function
		class Env {
			private ArgStack currentStack;
			private IRuntimeInformation RuntimeInfo { get; }
			internal ArgStack ArgStack { get => currentStack ; }
			public Env (IRuntimeInformation runtimeInfo, MethodInfo methodInfo)
			{
				this.RuntimeInfo = runtimeInfo;
				this.MethodName = methodInfo.ClassInfo.Name + "::" + methodInfo.Name;
				this.BSTypes = new BSTypes (runtimeInfo);
				this.ReturnType = methodInfo.ReturnType;
				currentStack = new ArgStack ();
			}

			public ClrType ReturnType { get; }

			public readonly BSTypes BSTypes;
			public readonly string MethodName;
		}

		// encapsulate the LLVM module and builder here.
		class Builder {
			static readonly LLVMBool Success = new LLVMBool (0);

			LLVMModuleRef module;
			LLVMBuilderRef builder;
			LLVMValueRef function;
			LLVMBasicBlockRef entry;
			LLVMBasicBlockRef currentBB;

			public LLVMModuleRef Module { get => module; }
			public LLVMValueRef Function { get => function; }

			public Builder () {
				module = LLVM.ModuleCreateWithName ("BigStepCompile");
				builder = LLVM.CreateBuilder ();
			}

			public void BeginFunction (string name, BSType returnType) {
				//FIXME: get types of args
				var funTy = LLVM.FunctionType (returnType.Lowered, Array.Empty <LLVMTypeRef> (), false);
				function = LLVM.AddFunction (module, name, funTy);
				entry = LLVM.AppendBasicBlock (function, "entry");
				LLVM.PositionBuilderAtEnd (builder, entry);
				currentBB = entry;
			}


			internal CompilationResult Finish (out NativeCodeHandle result) {

				// FIXME: get rid of this printf
				LLVM.DumpModule (Module);

				//FIXME: do this once
				LLVM.LinkInMCJIT ();
				LLVM.InitializeX86TargetMC ();
				LLVM.InitializeX86Target ();
				LLVM.InitializeX86TargetInfo ();
				LLVM.InitializeX86AsmParser ();
				LLVM.InitializeX86AsmPrinter ();
				LLVM.InitializeX86Disassembler ();

				/* this looks like unused code, but it initializes the target configuration */
				LLVMTargetRef target = LLVM.GetTargetFromName("x86-64");
				LLVMTargetMachineRef tmachine = LLVM.CreateTargetMachine(
						target,
						TargetTriple,
						"x86-64",  // processor
						"",  // feature
						LLVMCodeGenOptLevel.LLVMCodeGenLevelNone,
						LLVMRelocMode.LLVMRelocDefault,
						LLVMCodeModel.LLVMCodeModelDefault);
				/* </side effect code> */

				LLVMMCJITCompilerOptions options = new LLVMMCJITCompilerOptions { NoFramePointerElim = 0 };
				LLVM.InitializeMCJITCompilerOptions(options);
				if (LLVM.CreateMCJITCompilerForModule(out var engine, Module, options, out var error) != Success)
				{
					/* FIXME: If I make completely bogus LLVM IR, I would expect to
					 * fail here and get some kind of error, but I don't.
					 */
					Console.Error.WriteLine($"Error: {error}");
					result = NativeCodeHandle.Invalid;
					return CompilationResult.BadCode;
				}
				IntPtr fnptr = LLVM.GetPointerToGlobal (engine, Function);
				if (fnptr == IntPtr.Zero) {
					result = NativeCodeHandle.Invalid;
					Console.Error.WriteLine ("LLVM.GetPointerToGlobal returned null");
					return CompilationResult.InternalError;
				} else {
					Console.Error.WriteLine ("saw {0}", fnptr);
				}
				unsafe {
					result = new NativeCodeHandle ((byte*)fnptr, -1);
				}

				//FIXME: cleanup in a Dispose method?

				LLVM.DisposeBuilder (builder);

				// FIXME: can I really dispose of the EE while code is installed in Mono :-(

				// LLVM.DisposeExecutionEngine (engine);

				return Ok;
			}

			public LLVMValueRef ConstInt (BSType t, ulong value, bool signextend)
			{
				return LLVM.ConstInt (t.Lowered, value, signextend);
			}

			public void EmitRetVoid () {
				LLVM.BuildRetVoid (builder);
			}

			public void EmitRet (LLVMValueRef v)
			{
				LLVM.BuildRet (builder, v);
			}

			public LLVMValueRef EmitAlloca (BSType t, string nameHint)
			{
				return LLVM.BuildAlloca (builder, t.Lowered, nameHint);
			}

			public LLVMValueRef EmitLoad (LLVMValueRef ptr, string nameHint)
			{
				return LLVM.BuildLoad (builder, ptr, nameHint);
			}

			public void EmitStore (LLVMValueRef value, LLVMValueRef ptr)
			{
				LLVM.BuildStore (builder, value, ptr);
			}


		}

		void Preamble (Env env, Builder builder)
		{
			var rt = LowerType (env, env.ReturnType);
			builder.BeginFunction (env.MethodName, rt);
		}

		CompilationResult TranslateBody (Env env, Builder builder, MethodBody body)
		{
			var iter = body.GetIterator ();
			// TODO: alloca for locals and stack; store in env

			var r = Ok;

			while (iter.MoveNext ()) {
				var opcode = iter.Opcode;
				var opflags = iter.Flags;
				switch (opcode) {
					case Opcode.LdcI4S:
						r = TranslateLdcI4 (env, builder, iter.DecodeParamI ());
						break;
					case Opcode.Ret:
						r = TranslateRet (env, builder);
						break;
					default:
						throw NIE ($"BigStep.Translate {opcode}");
				}
				if (r != Ok)
					break;
			}
			return r;
		}

		CompilationResult TranslateRet (Env env, Builder builder)
		{
			if (env.ReturnType == RuntimeInfo.VoidType) {
				builder.EmitRetVoid ();
				return Ok;
			} else {
				var a = Pop (env, builder);
				var v = builder.EmitLoad (a.Ptr, "ret-value");
				builder.EmitRet (v);
				return Ok;
			}
				
		}

		CompilationResult TranslateLdcI4 (Env env, Builder builder, System.Int32 c)
		{
			BSType t = env.BSTypes.Int32Type;
			var a = Push (env, builder, t);
			
			var v = builder.ConstInt (t, (ulong)c, false);
			builder.EmitStore (v, a.Ptr);
			return Ok;
		}

		ArgStackValue Push (Env env, Builder builder, BSType t)
		{
			// FIXME: create stack slots up front and just bump a
			// stack height in the env and pick out the
			// pre-allocated slot.
			var v = builder.EmitAlloca (t, "stack-slot");
			var a = new ArgStackValue ();
			a.Ptr = v;
			env.ArgStack.Push (a);
			return a;
		}

		ArgStackValue Pop (Env env, Builder builder)
		{
			var a = env.ArgStack.Pop ();
			return a;
		}

		BSType LowerType (Env env, ClrType t)
		{
			if (t == RuntimeInfo.VoidType)
				return env.BSTypes.VoidType;
			else if (t == RuntimeInfo.Int32Type)
				return env.BSTypes.Int32Type;
			else
				throw NIE ($"don't know how to lower type {t}");
		}

		private static Exception NIE (string msg)
		{
			return new NotImplementedException (msg);
		}

	}
		
}
