using System;
using System.Collections.Generic;

public class Lol {
    public static void Main (string[] args) {
        var l = GenerateList ();
        var vstr = l[0].Value.ToString ();
    }

	static List<KeyValuePair<DateTime, string>> GenerateList () {
        var list = new List<KeyValuePair<DateTime, string>> (10);
        DateTime ttime = new DateTime (2012, 1, 1);
        list.Add (new KeyValuePair<DateTime, string> (ttime, "hello"));
        return list;
    }
}

