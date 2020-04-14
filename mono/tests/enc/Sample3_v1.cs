using System;
using System.Reflection;
using System.Runtime.CompilerServices;

public class Sample3 {
	public static void Main (string []args) {
		Assembly assm = typeof (Sample3).Assembly;
		var replacer = Replacer.Make ();

		DiffTestMethod1 (3, 8);
		replacer.Update (assm);
		DiffTestMethod1 (3, 8);
	}

	[MethodImpl(MethodImplOptions.NoInlining)]
	public static int DiffTestMethod1 (int x, int y) {
		SecondMethod ();
		return y;
	}

	[MethodImpl(MethodImplOptions.NoInlining)]
	public static void SecondMethod () {
		Console.WriteLine ("HELLO NEW WORLD");
	}
}

public class Replacer {
#if false
	const string name = "System.Runtime.CompilerServices.RuntimeFeature";
#else
	const string name = "Mono.Runtime";
#endif
	private static MethodBase _updateMethod;

	private static MethodBase UpdateMethod => _updateMethod ?? InitUpdateMethod();

	private static MethodBase InitUpdateMethod ()
	{
		var monoType = Type.GetType (name, false);
		_updateMethod = monoType.GetMethod ("LoadMetadataUpdate");
		if (_updateMethod == null)
			throw new Exception ($"Couldn't get LoadMetadataUpdate from {name}");
		return _updateMethod;
	}

	Replacer () { }

	public static Replacer Make ()
	{
		return new Replacer ();
	}

	public void Update (Assembly assm) {
		Console.WriteLine ("Apply Delta Update");
		UpdateMethod.Invoke (null, new object [] { assm });
	}
}
