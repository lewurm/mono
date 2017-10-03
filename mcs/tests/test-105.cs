using System;
using System.Threading;
using System.Runtime.InteropServices;

class Test {
	delegate int SimpleDelegate (int a);

	static int F (int a) {
		return a;
	}

	static void async_callback (IAsyncResult ar)
	{
		ar.AsyncState.ToString ();
	}
	
	public static int Main () {
		SimpleDelegate d = new SimpleDelegate (F);
		AsyncCallback ac = new AsyncCallback (async_callback);
		string state1 = "STATE1";
		
		IAsyncResult ar1 = d.BeginInvoke (1, ac, state1);

		Thread.Sleep (30000);
		return 0;
	}
}
