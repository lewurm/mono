using System;

interface IFoo
{
	bool Equals (object o);
}

class Hello : IFoo
{
	static void Main ()
	{
		IFoo f = new Hello ();
		int i = f.GetHashCode ();
		bool b = f.Equals (f);
	}
}
