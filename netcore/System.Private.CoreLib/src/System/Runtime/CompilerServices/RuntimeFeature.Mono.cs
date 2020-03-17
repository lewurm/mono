// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

using System.Runtime.InteropServices;

namespace System.Runtime.CompilerServices
{
	partial class RuntimeFeature
	{
		public static bool IsDynamicCodeSupported {
			[Intrinsic]  // the JIT/AOT compiler will change this flag to false for FullAOT scenarios, otherwise true
			get => IsDynamicCodeSupported;
		}

		public static bool IsDynamicCodeCompiled {
			[Intrinsic]  // the JIT/AOT compiler will change this flag to false for FullAOT scenarios, otherwise true
			get => IsDynamicCodeCompiled;
		}

		[MethodImplAttribute (MethodImplOptions.InternalCall)]
		private static unsafe extern void LoadMetadataUpdate_internal (string basename, string dmeta_path, string dil_path, byte* dmeta_bytes, int dmeta_length);

		private static int count;

		public static void LoadMetadataUpdate () {
			count++;
			string basename = "here.exe";
			string dmeta_name = $"{basename}.{count}.dmeta";
			string dil_name = $"{basename}.{count}.dil";

			byte[] dmeta = System.IO.File.ReadAllBytes (dmeta_name);
			unsafe {
				fixed (byte* dmeta_bytes = dmeta) {
				       LoadMetadataUpdate_internal (basename, dmeta_name, dil_name, dmeta_bytes, dmeta.Length);
				}
			}
		}
	}
}
