using System;

public class ReproGithub13284
{

	private static int CheckingStuff (ulong x, ulong y, ulong z)
	{
		checked {
			try {
				if (x - y - z >= 0)
					return 1;
				else
					return 2;
			} catch (OverflowException) {
				return 0;
			}
		}
	}
	public static void Main (string[] args) {
		int ret = CheckingStuff (2UL, 1UL, 3UL);
		Environment.Exit (ret);
	}
}

