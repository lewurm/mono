namespace MonoEnc {
	public class EncHelper {
#if false
		const string name = "System.Runtime.CompilerServices.RuntimeFeature";
#else
		const string name = "Mono.Runtime";
#endif
		private static MethodBase _updateMethod;

		private static MethodBase UpdateMethod => _updateMethod ?? InitUpdateMethod();

		private static MethodBase InitUpdateMethod ()
		{
			var monoType = Type.GetType (name, false);
			_updateMethod = monoType.GetMethod ("LoadMetadataUpdate");
			if (_updateMethod == null)
				throw new Exception ($"Couldn't get LoadMetadataUpdate from {name}");
			return _updateMethod;
		}

		Replacer () { }

		public static Replacer Make ()
		{
			return new Replacer ();
		}

		private Dictionary<Assembly, int> assembly_count = new Dictionary<Assembly, int> ();

		public void Update (Assembly assm) {
			Console.WriteLine ("Apply Delta Update");

			int count;
			if (!assembly_count.TryGetValue (assm, out count))
				count = 1;
			else
				count++;
			assembly_count [assm] = count;

			string basename = assm.Location;
			string dmeta_name = $"{basename}.{count}.dmeta";
			string dil_name = $"{basename}.{count}.dil";
			byte[] dmeta_data = System.IO.File.ReadAllBytes (dmeta_name);
			byte[] dil_data = System.IO.File.ReadAllBytes (dil_name);

			UpdateMethod.Invoke (null, new object [] { assm, dmeta_data, dil_data});
		}
	}
}
