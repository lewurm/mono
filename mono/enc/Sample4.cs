using System;
using System.Reflection;
using System.Runtime.CompilerServices;

public class Class2 {
}

public class Sample {
	public static void Main (string []args) {
		Calculate ();

		ApplyMagicMethodBodyReplacement ();

		Calculate ();
	}

	private static void Calculate () {
		for (int i = 1; i < 5; i++)
			Console.WriteLine ("Do (): " + Do ().ToString ());
	}

	[MethodImpl(MethodImplOptions.NoInlining)]
	private static Type Do () {
		return typeof (Sample);
	}

	private static void ApplyMagicMethodBodyReplacement () {
#if false
		var name = "System.Runtime.CompilerServices.RuntimeFeature";
#else
		var name = "Mono.Runtime";
#endif
		var monoType = Type.GetType (name, false);
		try {
			var update = monoType.GetMethod("LoadMetadataUpdate");
			update.Invoke (null, null);
		} catch (Exception e) {
			Console.WriteLine ("the impossible happen: " + e);
		}
	}
}
