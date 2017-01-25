using System;

#if __MOBILE__
class GenericsTests
#else
class Tests
#endif
{
	struct TestStruct {
		public int i;
		public int j;

		public TestStruct (int i, int j) {
			this.i = i;
			this.j = j;
		}
	}

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

	public static int test_1_nullable_unbox_vtype ()
	{
		return Unbox<TestStruct?> (new TestStruct (1, 2)).Value.i;
	}


	public static int test_1_nullable_unbox_null_vtype ()
	{
		return Unbox<TestStruct?> (null).HasValue ? 0 : 1;
	}

	public static int test_1_nullable_box_vtype ()
	{
		return ((TestStruct)(Box<TestStruct?> (new TestStruct (1, 2)))).i;
	}

	public static int test_1_nullable_box_null_vtype ()
	{
		return Box<TestStruct?> (null) == null ? 1 : 0;
	}

	public static int test_1_isinst_nullable_vtype ()
	{
		object o = new TestStruct (1, 2);
		return (o is TestStruct?) ? 1 : 0;
	}

	public static int test_0_nullable_normal_unbox ()
	{
		int? i = 5;

		object o = i;
		// This uses unbox instead of unbox_any
		int? j = (int?)o;

		if (j != 5)
			return 1;

		return 0;
	}

#if FALSE
	public static void stelem_any<T> (T[] arr, T elem) {
		arr [0] = elem;
	}

	public static T ldelem_any<T> (T[] arr) {
		return arr [0];
	}

	public static int test_1_ldelem_stelem_any_int () {
		int[] arr = new int [3];
		stelem_any (arr, 1);

		return ldelem_any (arr);
	}
#endif

	static object Box<T> (T t)
	{
		return t;
	}

	static T Unbox <T> (object o) {
		return (T) o;
	}
}
