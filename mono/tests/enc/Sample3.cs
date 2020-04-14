using System;
using System.Reflection;
using System.Runtime.CompilerServices;
using MonoEnc;

public class Sample3 {
	public static void Main (string []args) {
		Assembly assm = typeof (Sample3).Assembly;
		var replacer = EncHelper.Make ();

		DiffTestMethod1 (3, 8);
		replacer.Update (assm);
		DiffTestMethod1 (3, 8);
	}

	[MethodImpl(MethodImplOptions.NoInlining)]
	public static int DiffTestMethod1 (int x, int y) {
		Console.WriteLine ("Hello old World");
		SecondMethod ();
		return x;
	}

	[MethodImpl(MethodImplOptions.NoInlining)]
	public static void SecondMethod () {
		Console.WriteLine ("Print something old-worldish");
	}
}

