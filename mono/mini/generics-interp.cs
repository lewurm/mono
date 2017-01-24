using System;

#if __MOBILE__
class GenericsTests
#else
class Tests
#endif
{
#if !__MOBILE__
	public static int Main (string[] args) {
		return TestDriver.RunTests (typeof (Tests), args);
	}
#endif

	public static int test_1_no_nullable_unbox ()
	{
		return Unbox<int> (1);
	}

	public static int test_1_nullable_unbox_null ()
	{
		return Unbox<int?> (null).HasValue ? 0 : 1;
	}

	public static int test_1_nullable_box ()
	{
		return (int) Box<int?> (1);
	}

	public static int test_1_nullable_box_null ()
	{
		return Box<int?> (null) == null ? 1 : 0;
	}

	public static int test_1_isinst_nullable ()
	{
		object o = 1;
		return (o is int?) ? 1 : 0;
	}

	static object Box<T> (T t)
	{
		return t;
	}

	static T Unbox <T> (object o) {
		return (T) o;
	}
}
