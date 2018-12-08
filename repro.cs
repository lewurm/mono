using System;
using System.Runtime.CompilerServices;

public class Repro
{
	public static void Main (string []args)
	{
		object o = null;
		try {
			Console.WriteLine ("o: " + o.ToString ());
		} catch (Exception e) {
			Console.WriteLine ("failed ToString ()");
		}

		long res = Add (0x1);

		if (res != 0x33331338)
			Console.WriteLine ("add does not work: 0x" + res.ToString ("x"));
		else
			Console.WriteLine ("we are good");
	}

	public static long Add (long n)
	{
		return n + 0x33331337;
	}
}

























// cheat comment
//	[MethodImpl(MethodImplOptions.NoInlining)]
