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
	public static int test_1337_no_nullable_unbox ()
	{
		return Unbox<int> (1337);
	}

	public static int test_43981_nullable_unbox ()
	{
		return Unbox<int?> (0xabcd).Value;
	}

	public static int test_1_nullable_unbox_null ()
	{
		return Unbox<int?> (null).HasValue ? 0 : 1;
	}

	static T Unbox <T> (object o) {
		return (T) o;
	}
}
