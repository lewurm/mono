using System;
using System.Runtime.CompilerServices;

class MonoEmbed {
	[MethodImplAttribute(MethodImplOptions.InternalCall)]
	extern static string gimme();
	
	public static void ThrowException ()
	{
		throw new Exception ("go through native");
	}

	static void Main() {
		Console.WriteLine (gimme ());
	}
}
