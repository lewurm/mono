using System;
using System.Collections;
using System.Threading;
using System.Runtime.ConstrainedExecution;

class P
{
	int index;
	ManualResetEvent mre;

	public static int Count = 0;

	public P (int index, ManualResetEvent mre)
	{
		this.index = index;
		this.mre = mre;
	}

	~P ()
	{
		mre.Set ();

		Console.Write (String.Format ("[{0}] Finalize\n", index));
		Count ++;
		Console.Write (String.Format ("[{0}] Finalize -- end\n", index));
	}
}

class Driver
{
	static int Main ()
	{
		Thread thread;
		ManualResetEvent mre;
		int collected, total = 50;

		for (int i = 1; i <= 300; ++i) {
			P.Count = 0;

			mre = new ManualResetEvent (false);

			thread = new Thread (() => {
				for (int j = 0; j < total; ++j)
					new P (i, mre);
			});
			thread.Start ();
			thread.Join ();

			GC.Collect ();

			Console.Write (String.Format ("[{0}] Wait for pending finalizers\n", i));
			GC.WaitForPendingFinalizers ();
			Console.Write (String.Format ("[{0}] Wait for pending finalizers -- end\n", i));

			collected = P.Count;
			if (collected == 0) {
				if (!mre.WaitOne (5000)) {
					Console.Write (String.Format ("[{0}] Finalizer never started\n", i));
					return 1;
				}

				Console.Write (String.Format ("[{0}] Wait for pending finalizers (2)\n", i));
				GC.WaitForPendingFinalizers ();
				Console.Write (String.Format ("[{0}] Wait for pending finalizers (2) -- end\n", i));

				collected = P.Count;
				if (collected == 0) {
					/* At least 1 finalizer started (as mre has been Set), but P.Count has not been incremented */
					Console.Write (String.Format ("[{0}] Did not wait for finalizers to run\n", i));
					return 2;
				}
			}

			if (collected != total) {
				/* Not all finalizer finished, before returning from WaitForPendingFinalizers. Or not all objects
				 * have been garbage collected; this might be due to false pinning */
				Console.Write (String.Format ("[{0}] Finalized {1} of {2} objects\n", i, collected, total));
			}
		}

		Console.WriteLine ("almost done");

		ArrayList list = new ArrayList (); 
		Thread.SetData(foo.dataslot, "ID is wibble");
		Environment.ExitCode = 2;
		while(true) { 
			foo instance = new foo(); 
			list.Add (new WeakReference(instance)); 
			Thread.Sleep (0);
		}
		return 1;

	}
}

public class foo  { 
	public static LocalDataStoreSlot dataslot = Thread.AllocateDataSlot();
	public static int final_count=0;

	~foo() { 
		// Demonstrate that this is still the same thread
		string ID=(string)Thread.GetData(dataslot);
		if(ID==null) {
			Console.WriteLine("Set ID: foo");
			Thread.SetData(dataslot, "foo");
		}

		// Don't run forever
		if(final_count++>10) {
			Environment.Exit(0);
		}

		Console.WriteLine("finalizer thread ID: {0}", (string)Thread.GetData(dataslot));

		if ((string)Thread.GetData(dataslot) != "foo")
			throw new Exception ();
	} 
} 
