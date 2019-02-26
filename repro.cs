using System;
using System.Collections.Generic;
using System.Reflection;
using System.Linq;
using System.Linq.Expressions;

public class Repro {
	public struct GazonkOld {
		public string Tzap;

		public GazonkOld (string tzap)
		{
			Tzap = tzap;
		}
	}

	public static int CompileStructInstanceField ()
	{
		var p = Expression.Parameter (typeof (GazonkOld), "gazonk");
		var gazonker = Expression.Lambda<Func<GazonkOld, string>> ( Expression.Field (p, typeof (GazonkOld).GetField ("Tzap")), p).Compile ();

		return "bang" == gazonker (new GazonkOld ("bang")) ? 0 : 1;
	}

#if false
		[Test]
		public void NegateLiftedDecimal ()
		{
			var d = Expression.Parameter (typeof (decimal?), "l");

			var meth = typeof (decimal).GetMethod ("op_UnaryNegation", new [] { typeof (decimal) });

			var node = Expression.Negate (d);
			Assert.IsTrue (node.IsLifted);
			Assert.IsTrue (node.IsLiftedToNull);
			Assert.AreEqual (typeof (decimal?), node.Type);
			Assert.AreEqual (meth, node.Method);

			var neg = Expression.Lambda<Func<decimal?, decimal?>> (node, d).Compile ();

			Assert.AreEqual (-2m, neg (2m));
			Assert.AreEqual (null, neg (null));
		}
#endif
		public class Foo {
			public string Bar;
			public string Baz;

			public Gazonk Gaz;

			public Gazonk Gazoo { get; set; }

			public string Gruik { get; set; }

			public Foo ()
			{
				Gazoo = new Gazonk ();
				Gaz = new Gazonk ();
			}
		}

		public class Gazonk {
			public string Tzap;

			public int Klang;

			public string Couic { get; set; }

			public string Bang () { return ""; }
		}

		public static int CompiledMemberBinding ()
		{
			var getfoo = Expression.Lambda<Func<Foo>> (
					Expression.MemberInit (
						Expression.New (typeof (Foo)),
						Expression.MemberBind (
							typeof (Foo).GetProperty ("Gazoo"),
							Expression.Bind (typeof (Gazonk).GetField ("Tzap"),
								Expression.Constant ("tzap")),
							Expression.Bind (typeof (Gazonk).GetField ("Klang"),
								Expression.Constant (42))))).Compile ();

			var foo = getfoo ();

			if (foo == null)
				return 2;
			if (foo.Gazoo.Klang != 42)
				return 3;
			if (foo.Gazoo.Tzap != "tzap")
				return 4;

			return 0;
		}

		public static Func<decimal?, decimal?> Yeah ()
		{
			var d = Expression.Parameter (typeof (decimal?), "l");


			var node = Expression.Negate (d);
#if false
			var meth = typeof (decimal).GetMethod ("op_UnaryNegation", new [] { typeof (decimal) });
			if (!node.IsLifted)
				return 1;
			if (!node.IsLiftedToNull)
				return 2;
			if (typeof (decimal?) != node.Type)
				return 3;
			if (meth != node.Method)
				return 4;
#endif

			var neg = Expression.Lambda<Func<decimal?, decimal?>> (node, d).Compile ();
			return neg;
		}

		public static int NegateLiftedDecimal ()
		{
			var neg = Yeah ();

			if ((-2m) != neg (2m))
				return 5;

#if false
			if (null != neg (null))
				return 6;
#endif
			return 0;
		}

	public static int Main (string []args)
	{
		int ret = -1;
#if false
		ret = CompileStructInstanceField ();
		if (ret != 0) {
			return 1;
		}

		ret = CompiledMemberBinding ();
		if (ret != 0) {
			Console.WriteLine ("CompiledMemberBinding: " + ret);
			return 2;
		}
#endif
		ret = NegateLiftedDecimal ();
		if (ret != 0) {
			Console.WriteLine ("NegateLiftedDecimal: " + ret);
			return 3;
		}

		return 0;
	}
}
