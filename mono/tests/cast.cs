using System;
using System.Runtime.CompilerServices;

public class Cast
{
	public class A {
		public virtual void foo () {
			Console.WriteLine ("A::foo ()");
		}
	}

	public class B : A {
		public override void foo () {
			Console.WriteLine ("B::foo ()");
		}
	}

	public class C {
		public void bar () {
			Console.WriteLine ("C::bar ()");
		}
	}

	public interface I {
		void baz ();
	}

	public static int test_0_vector_array_cast () {
		Array arr1 = Array.CreateInstance (typeof (int), new int[] {1}, new int[] {0});
		Array arr2 = Array.CreateInstance (typeof (int), new int[] {1}, new int[] {10});

		int[] b = (int[])arr1;

		try {
			b = (int[])arr2;
			return 3;
		} catch (InvalidCastException) {
		}

		if (arr2 is int[])
			return 4;

		return 0;
	}

	public class MulOvfClass {
		[MethodImplAttribute (MethodImplOptions.NoInlining)]
		public unsafe void EncodeIntoBuffer(char* value, int valueLength, char* buffer, int bufferLength) {
		}
	}

	public static unsafe int test_0_mul_ovf_regress_36052 () {
		var p = new MulOvfClass ();

		string typeName = typeof(int).Name;
		int bufferSize = 45;

		fixed (char* value = typeName) {
			char* buffer = stackalloc char[bufferSize];
			p.EncodeIntoBuffer(value, typeName.Length, buffer, bufferSize);
		}
		return 0;
	}

	public static int Main() {
		B b = new B ();
		object o = b;
		int res = 0;

		if (o is A) {
			((A) o).foo ();
			res++;
		}

		if (o is B) {
			((B) o).foo ();
			res++;
		}

		if (o is C) {
			((C) o).bar ();
			res++;
		}

		if (o is I) {
			((I) o).baz ();
			res++;
		}

		int vecstuff = test_0_vector_array_cast ();
		int mulovf = test_0_mul_ovf_regress_36052 ();
		return res == 2 ? 0 : (res + vecstuff + mulovf);
	}
}
