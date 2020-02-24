using System;
using System.Reflection;
using System.Runtime.CompilerServices;

public class Sample {
	public static void Main (string []args) {
		Calculate ();

		ApplyMagicMethodBodyReplacement ();

		Calculate ();
	}

	private static void Calculate () {
		for (int i = 1; i < 5; i++)
			Console.WriteLine ("Do (): " + Do ());
	}

	[MethodImpl(MethodImplOptions.NoInlining)]
	private static string Do () {
		return "Not replaced";
	}

	private static void ApplyMagicMethodBodyReplacement () {
		var monoType = Type.GetType ("System.Runtime.CompilerServices.RuntimeFeature", false);
		try {
			var update = monoType.GetMethod("LoadMetadataUpdate");
			update.Invoke (null, null);
		} catch (Exception e) {
			Console.WriteLine ("the impossible happen: " + e);
		}
	}
}
