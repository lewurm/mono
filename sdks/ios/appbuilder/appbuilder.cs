using System;
using System.IO;
using System.Globalization;
using System.Linq;
using System.Reflection;
using System.Collections.Generic;
using Mono.Options;

public class AppBuilder
{
	public static void Main (String[] args) {
		new AppBuilder ().Run (args);
	}

	void GenMain (string builddir, List<string> assembly_names, bool isinterp) {
		var symbols = new List<string> ();
		foreach (var img in assembly_names) {
			symbols.Add (String.Format ("mono_aot_module_{0}_info", img.Replace ('.', '_').Replace ('-', '_')));
		}

		var w = File.CreateText (Path.Combine (builddir, "main.m"));

		/* copy from <mono/mini/jit.h> */
		w.WriteLine ("typedef enum {");
		w.WriteLine ("	MONO_AOT_MODE_NONE,");
		w.WriteLine ("	MONO_AOT_MODE_NORMAL,");
		w.WriteLine ("	MONO_AOT_MODE_HYBRID,");
		w.WriteLine ("	MONO_AOT_MODE_FULL,");
		w.WriteLine ("	MONO_AOT_MODE_LLVMONLY,");
		w.WriteLine ("	MONO_AOT_MODE_INTERP,");
		w.WriteLine ("	MONO_AOT_MODE_INTERP_LLVMONLY");
		w.WriteLine ("} MonoAotMode;");
		w.WriteLine ();
		w.WriteLine ("void mono_jit_set_aot_mode (MonoAotMode mode);");
		w.WriteLine ();

		w.WriteLine ("extern void mono_aot_register_module (char *name);");
		w.WriteLine ();

		if (isinterp) {
			w.WriteLine ("extern void mono_ee_interp_init (const char *);");
			w.WriteLine ("extern void mono_icall_table_init (void);");
			w.WriteLine ("extern void mono_marshal_ilgen_init (void);");
			w.WriteLine ("extern void mono_method_builder_ilgen_init (void);");
			w.WriteLine ("extern void mono_sgen_mono_ilgen_init (void);");
			w.WriteLine ();
		}

		foreach (var symbol in symbols) {
			w.WriteLine ($"extern void *{symbol};");
		}

		w.WriteLine ();
		w.WriteLine ("void mono_ios_register_modules (void)");
		w.WriteLine ("{");
		foreach (var symbol in symbols) {
			w.WriteLine ($"\tmono_aot_register_module ({symbol});");
		}
		w.WriteLine ("}");

		w.WriteLine ();
		w.WriteLine ("void mono_ios_setup_execution_mode (void)");
		w.WriteLine ("{");
		if (isinterp) {
			w.WriteLine ("\tmono_icall_table_init ();");
			w.WriteLine ("\tmono_marshal_ilgen_init ();");
			w.WriteLine ("\tmono_method_builder_ilgen_init ();");
			w.WriteLine ("\tmono_sgen_mono_ilgen_init ();");
			w.WriteLine ("\tmono_ee_interp_init (0);");
		}
		w.WriteLine ("\tmono_jit_set_aot_mode ({0});", isinterp ? "MONO_AOT_MODE_INTERP" : "MONO_AOT_MODE_FULL");
		w.WriteLine ("}");

		w.WriteLine ();
		w.Close ();
	}

	void check_mandatory (string val, string name) {
		if (val == null) {
			Console.Error.WriteLine ($"The {name} argument is mandatory.");
			Environment.Exit (1);
		}
	}

	static void TemplateInfoPlist (string template_path, string result_path, string identifier, string executable, string name, string container_identifier, string platform) {
		var lines = File.ReadAllLines (template_path);
		for (int i = 0; i < lines.Length; ++i) {
			string line = lines [i];
			line = line.Replace ("BUNDLE_IDENTIFIER", identifier);
			line = line.Replace ("BUNDLE_EXECUTABLE", executable);
			line = line.Replace ("BUNDLE_NAME", name);
			line = line.Replace ("BUNDLE_CONTAINER", container_identifier);
			line = line.Replace ("PLATFORM", platform);

			lines [i] = line;
		}
		File.WriteAllLines (result_path, lines);
	}

	void Run (String[] args) {
		string target = null;
		string appdir = null;
		string appdir_container = null;
		string builddir = null;
		string builddir_container = null;
		string runtimedir = null;
		string mono_sdkdir = null;
		string bundle_identifier = null;
		string bundle_name = null;
		string bundle_executable = null;
		string sysroot = null;
		string aotdir = null;
		string exe = null;
		string signing_identity = null;
		string profile = null;
		string profile_watchos_container = null;
		string profile_watchos_appex = null;
		string clang_arch = null;
		string version_min = null;
		string linker_args = null;
		bool isdev = false;
		bool iswatch = false;
		bool isrelease = false;
		bool isllvm = false;
		bool isinterponly = false;
		bool isinterpmixed = false;
		var assemblies = new List<string> ();
		var p = new OptionSet () {
				{ "target=", s => target = s },
				{ "appdir=", s => appdir = s },
				{ "builddir=", s => builddir = s },
				{ "runtimedir=", s => runtimedir = s },
				{ "mono-sdkdir=", s => mono_sdkdir = s },
				{ "sysroot=", s => sysroot = s },
				{ "aot-cachedir=", s => aotdir = s },
				{ "bundle-identifier=", s => bundle_identifier = s },
				{ "bundle-name=", s => bundle_name = s },
				{ "bundle-executable=", s => bundle_executable = s },
				{ "signing-identity=", s => signing_identity = s },
				{ "profile=", s => profile = s },
				{ "profile-appex=", s => profile_watchos_appex = s },
				{ "profile-container=", s => profile_watchos_container = s },
				{ "llvm", s => isllvm = true },
				{ "interp-only", s => isinterponly = true },
				{ "interp-mixed", s => isinterpmixed = true },
				{ "exe=", s => exe = s },
				{ "r=", s => assemblies.Add (s) },
			};

		var new_args = p.Parse (args).ToArray ();

		check_mandatory (target, "--target");
		check_mandatory (runtimedir, "--runtimedir");
		check_mandatory (appdir, "--appdir");
		check_mandatory (mono_sdkdir, "--mono-sdkdir");
		check_mandatory (sysroot, "--sysroot");
		check_mandatory (signing_identity, "--signing-identity");

		switch (target) {
		case "ios-dev64":
			clang_arch = "arm64";
			isdev = true;
			version_min = "iphoneos-version-min=10.1";
			linker_args = "-framework UIKit -framework GSS";
			break;
		case "ios-sim64":
			version_min = "iphoneos-version-min=10.1";
			break;
		case "watch-dev6432":
			clang_arch = "arm64_32";
			iswatch = true;
			version_min = "watchos-version-min=5.1";
			linker_args = "-framework WatchKit -lc++ -fapplication-extension";
			builddir_container = builddir;
			builddir = Path.Combine (builddir, "PlugIns", bundle_executable + ".appex"); // location of actual app
			appdir_container = appdir;
			appdir = Path.Combine (appdir, "Plugins", bundle_executable + ".appex"); // location of actual app
			break;
		default:
			Console.WriteLine ($"Possible values for the '--target=' argument are 'ios-dev64', 'ios-sim64', 'watch-dev6432', got {target}.");
			Environment.Exit (1);
			break;
		}

		if (iswatch) {
			check_mandatory (profile_watchos_appex, "--profile-appex");
			check_mandatory (profile_watchos_container, "--profile-container");
		}

		if (isllvm)
			isrelease = true;

		bool isinterpany = isinterponly || isinterpmixed;

		string aot_args = "";
		string cross_runtime_args = "";

		if (isinterponly) {
			aot_args = "interp";
		} else if (isinterpmixed) {
			aot_args = "interp,full";
		} else {
			aot_args = "full";
		}

		if (!isrelease)
			aot_args += ",soft-debug";
		if (isllvm) {
			cross_runtime_args = "--llvm";
			aot_args += ",llvm-path=$mono_sdkdir/llvm-llvm64/bin,llvm-outfile=$llvm_outfile";
		}

		Directory.CreateDirectory (builddir);

		// Create Info.plist file
		string plist_src = Path.Combine (runtimedir, "Info.plist.in");
		string plist_dst = Path.Combine (builddir, "Info.plist");
		if (iswatch) {
			string base_identifier = bundle_identifier + ".watchkitapp";
			plist_src = Path.Combine (runtimedir, "Info.watch.ext.plist.in");
			TemplateInfoPlist (plist_src, plist_dst, base_identifier + ".watchkitextension", bundle_executable + " WatchKit Extension", bundle_name + " Ext", base_identifier, "watchOS");

			plist_src = Path.Combine (runtimedir, "Info.watch.plist.in");
			plist_dst = Path.Combine (builddir_container, "Info.plist");
			TemplateInfoPlist (plist_src, plist_dst, base_identifier, bundle_executable + " WatchKit App", bundle_name, "", "watchOS");
		} else if (isdev) {
			TemplateInfoPlist (plist_src, plist_dst, bundle_identifier, bundle_executable, bundle_name, "", "iPhoneOS");
		} else {
			TemplateInfoPlist (plist_src, plist_dst, bundle_identifier, bundle_executable, bundle_name, "", "iPhoneSimulator");
		}

		// Create config.json file
		string config = "{ \"exe\" : \"" + exe + "\" }";
		File.WriteAllLines (Path.Combine (builddir, "config.json"), new string [] { config });

		var ninja = File.CreateText (Path.Combine (builddir, "build.ninja"));

		// Defines
		ninja.WriteLine ($"mono_sdkdir = {mono_sdkdir}");
		ninja.WriteLine ($"monoios_dir = {runtimedir}");
		ninja.WriteLine ($"appdir = {appdir}");
		ninja.WriteLine ($"sysroot = {sysroot}");
		if (iswatch)
			ninja.WriteLine ("cross = $mono_sdkdir/ios-crosswatch64_32-release/bin/aarch64-apple-darwin10_ilp32-mono-sgen");
		else
			ninja.WriteLine ("cross = $mono_sdkdir/ios-cross64-release/bin/aarch64-darwin-mono-sgen");
		ninja.WriteLine ($"builddir = .");
		if (iswatch) {
			ninja.WriteLine ($"appdir_container = {appdir_container}");
			ninja.WriteLine ($"builddir_container = {builddir_container}");
		}
		if (aotdir != null)
			ninja.WriteLine ($"aotdir = {aotdir}");
		ninja.WriteLine ($"signing_identity = {signing_identity}");
		// Rules
		ninja.WriteLine ("rule aot");
		ninja.WriteLine ($"  command = MONO_PATH=$mono_path $cross -O=gsharedvt,float32 --debug {cross_runtime_args} --aot=mtriple=arm64-ios,static,asmonly,direct-icalls,no-direct-calls,dwarfdebug,{aot_args},outfile=$outfile,data-outfile=$data_outfile $src_file");
		ninja.WriteLine ("  description = [AOT] $src_file -> $outfile");
		// ninja remakes files it hadn't seen before even if the timestamp is newer, so have to add a test ourselves
		ninja.WriteLine ("rule aot-cached");
		ninja.WriteLine ($"  command = if ! test -f $outfile; then MONO_PATH=$mono_path $cross -O=gsharedvt,float32 --debug {cross_runtime_args} --aot=mtriple=arm64-ios,static,asmonly,direct-icalls,no-direct-calls,dwarfdebug,{aot_args},outfile=$outfile,data-outfile=$data_outfile $src_file; fi");
		ninja.WriteLine ("  description = [AOT] $src_file -> $outfile");
		ninja.WriteLine ("rule assemble");
		ninja.WriteLine ($"  command = clang -isysroot $sysroot -m{version_min} -arch {clang_arch} -c -o $out $in");
		ninja.WriteLine ("  description = [ASM] $in -> $out");
		ninja.WriteLine ("rule assemble-cached");
		ninja.WriteLine ($"  command = if ! test -f $out; then clang -isysroot $sysroot -m{version_min} -arch {clang_arch} -c -o $out $in; fi");
		ninja.WriteLine ("  description = [ASM] $in -> $out");
		ninja.WriteLine ("rule cp");
		ninja.WriteLine ("  command = cp $in $out");
		ninja.WriteLine ("  description = [CP] $in -> $out");
		ninja.WriteLine ("rule cp-recursive");
		ninja.WriteLine ("  command = cp -r $in $out");
		ninja.WriteLine ("rule cpifdiff");
		ninja.WriteLine ("  command = if cmp -s $in $out ; then : ; else cp $in $out ; fi");
		ninja.WriteLine ("  restat = true");
		ninja.WriteLine ("rule plutil");
		ninja.WriteLine ("  command = cp $in $out; plutil -convert binary1 $out");
		ninja.WriteLine ("rule codesign");
		ninja.WriteLine ("  command = codesign -v --force --sign \"$signing_identity\" --entitlements $entitlements --timestamp=none $in");
		ninja.WriteLine ("rule codesign-sim");
		ninja.WriteLine ("  command = codesign --force --sign - --timestamp=none $in");
		ninja.WriteLine ("rule mkdir");
		ninja.WriteLine ("  command = mkdir -p $out");
		ninja.WriteLine ("rule compile-objc");
		ninja.WriteLine ($"  command = clang -isysroot $sysroot -m{version_min} -arch {clang_arch} -c -o $out $in");
		ninja.WriteLine ("rule gen-exe");
		ninja.WriteLine ("  command = mkdir $appdir");
		if (iswatch)
			ninja.WriteLine ($"  command = clang -ObjC -isysroot $sysroot -m{version_min} -arch {clang_arch} -framework Foundation {linker_args} -o $appdir/{bundle_executable + "\\ WatchKit\\ Extension"} $in -liconv -lz $forcelibs");
		else
			ninja.WriteLine ($"  command = clang -ObjC -isysroot $sysroot -m{version_min} -arch {clang_arch} -framework Foundation {linker_args} -o $appdir/{bundle_executable} $in -liconv -lz $forcelibs");
	
		var ofiles = "";
		var assembly_names = new List<string> ();
		var cultures = CultureInfo.GetCultures (CultureTypes.AllCultures).Where (x => !String.IsNullOrEmpty (x.IetfLanguageTag)).Select (x => x.IetfLanguageTag);
		foreach (var assembly in assemblies) {
			string filename = Path.GetFileName (assembly);
			var filename_noext = Path.GetFileNameWithoutExtension (filename);

			File.Copy (assembly, Path.Combine (builddir, filename), true);
			if (aotdir != null && !File.Exists (Path.Combine (aotdir, filename)))
				/* Don't overwrite to avoid changing the timestamp */
				File.Copy (assembly, Path.Combine (aotdir, filename), false);

			if (!iswatch)
				/* FIXME!!!! */
				ninja.WriteLine ($"build $appdir/{filename}: cpifdiff $builddir/{filename}");

			var assembly_dir = Path.GetDirectoryName (assembly);
			var resource_filename = filename.Replace (".dll", ".resources.dll");
			foreach (var culture in cultures) {
				var resource_assembly = Path.Combine (assembly_dir, culture, resource_filename);
				if (!File.Exists(resource_assembly)) continue;

				Directory.CreateDirectory (Path.Combine (builddir, culture));
				File.Copy (resource_assembly, Path.Combine (builddir, culture, resource_filename), true);
				ninja.WriteLine ($"build $appdir/{culture}/{resource_filename}: cpifdiff $builddir/{culture}/{resource_filename}");
			}

			if (isinterpany && filename_noext != "mscorlib")
				continue;

			if (isdev || iswatch) {
				string destdir = null;
				string srcfile = null;
				string assemble_rule = null;
				if (aotdir != null) {
					destdir = "$aotdir";
					srcfile = Path.Combine (aotdir, filename);
					assemble_rule = "assemble-cached";
				} else {
					destdir = "$builddir";
					srcfile = $"{filename}";
					assemble_rule = "assemble";
				}
				string outputs = $"{destdir}/{filename}.s {destdir}/{filename_noext}.aotdata";
				if (isllvm)
					outputs += $" {destdir}/{filename}.llvm.s";
				if (aotdir != null)
					ninja.WriteLine ($"build {outputs}: aot-cached {srcfile}");
				else
					ninja.WriteLine ($"build {outputs}: aot {srcfile}");
				ninja.WriteLine ($"  src_file={srcfile}");
				ninja.WriteLine ($"  outfile={destdir}/{filename}.s");
				ninja.WriteLine ($"  data_outfile={destdir}/{filename_noext}.aotdata");
				ninja.WriteLine ($"  mono_path={destdir}");
				if (isllvm)
					ninja.WriteLine ($"  llvm_outfile={destdir}/{filename}.llvm.s");
				ninja.WriteLine ($"build {destdir}/{filename}.o: {assemble_rule} {destdir}/{filename}.s");
				if (isllvm)
					ninja.WriteLine ($"build {destdir}/{filename}.llvm.o: {assemble_rule} {destdir}/{filename}.llvm.s");
				ninja.WriteLine ($"build $appdir/{filename_noext}.aotdata: cp {destdir}/{filename_noext}.aotdata");

				ofiles += " " + ($"{destdir}/{filename}.o");
				if (isllvm)
					ofiles += " " + ($"{destdir}/{filename}.llvm.o");
			}
			var aname = AssemblyName.GetAssemblyName (assembly);
			assembly_names.Add (aname.Name);
		}

		ninja.WriteLine ("build $appdir: mkdir");

		if (isdev || iswatch) {
			string libprefix = null;
			if (isdev)
				libprefix = "$mono_sdkdir/ios-target64-release/lib/";
			else if (iswatch)
				libprefix = "$mono_sdkdir/ios-targetwatch64_32-release/lib/";

			string libs = libprefix + "libmonosgen-2.0.a";
			if (isinterpany) {
				libs += " " + libprefix + "libmono-ee-interp.a";
				libs += " " + libprefix + "libmono-icall-table.a";
				libs += " " + libprefix + "libmono-ilgen.a";
			}

			if (isdev) {
				ninja.WriteLine ($"build $appdir/{bundle_executable}: gen-exe {ofiles} $builddir/main.o " + libs + " $monoios_dir/libmonoios.a");
				ninja.WriteLine ($"    forcelibs = -force_load {libprefix}libmono-native-unified.a");
			} else if (iswatch) {
				ninja.WriteLine ($"build $appdir/{bundle_executable + "$ WatchKit$ Extension"}: gen-exe {ofiles} $builddir/main.o " + libs + " $monoios_dir/libmonowatch.a");
			}

			ninja.WriteLine ("build $builddir/main.o: compile-objc $builddir/main.m");
		} else {
			ninja.WriteLine ($"build $appdir/{bundle_executable}: cp $monoios_dir/runtime");
		}
		ninja.WriteLine ("build $builddir/Info.plist.binary: plutil $builddir/Info.plist");
		ninja.WriteLine ("build $appdir/Info.plist: cpifdiff $builddir/Info.plist.binary");
		ninja.WriteLine ("build $appdir/config.json: cpifdiff $builddir/config.json");
		ninja.WriteLine ("build $builddir/Entitlements.xcent: cpifdiff $monoios_dir/Entitlements.xcent");
		if (iswatch) {
			ninja.WriteLine ($"build $builddir/embedded.mobileprovision: cp {profile_watchos_appex}");
			ninja.WriteLine ($"build $appdir/embedded.mobileprovision: cp $builddir/embedded.mobileprovision");
		} else {
			if (profile != null) {
				ninja.WriteLine ($"build $builddir/embedded.mobileprovision: cp {profile}");
				ninja.WriteLine ($"build $appdir/embedded.mobileprovision: cp $builddir/embedded.mobileprovision");
			}
		}
		if (isdev)
			ninja.WriteLine ($"build $appdir/_CodeSignature: codesign $appdir/{bundle_executable} | $builddir/Entitlements.xcent");
		else if (iswatch)
			ninja.WriteLine ($"build $appdir/_CodeSignature: codesign $appdir/{bundle_executable + "$ WatchKit$ Extension"} | $builddir/Entitlements.xcent");
		else
			ninja.WriteLine ($"build $appdir/_CodeSignature: codesign-sim $appdir/{bundle_executable} | $builddir/Entitlements.xcent");
		ninja.WriteLine ("  entitlements=$builddir/Entitlements.xcent");

		if (iswatch) {
			/* handle container part */
			ninja.WriteLine ($"build $appdir_container/_WatchKitStub: mkdir");
			ninja.WriteLine ($"build $appdir_container/_WatchKitStub/WK: cp $sysroot/Library/Application$ Support/WatchKit/WK");
			ninja.WriteLine ($"build $appdir_container/{bundle_executable + "$ WatchKit$ App"}: cp $sysroot/Library/Application$ Support/WatchKit/WK");

			ninja.WriteLine ($"build $builddir_container/embedded.mobileprovision: cp {profile_watchos_container}");
			ninja.WriteLine ($"build $appdir_container/embedded.mobileprovision: cp $builddir_container/embedded.mobileprovision");

			ninja.WriteLine ("build $builddir_container/Info.plist.binary: plutil $builddir_container/Info.plist");
			ninja.WriteLine ("build $appdir_container/Info.plist: cpifdiff $builddir_container/Info.plist.binary");

			ninja.WriteLine ($"build $appdir_container/_CodeSignature: codesign $appdir_container/{bundle_executable + "$ WatchKit$ App"} | $builddir/Entitlements.xcent");
			ninja.WriteLine ("  entitlements=$builddir/Entitlements.xcent");

			ninja.WriteLine ("build $appdir_container/Base.lproj: cp-recursive $monoios_dir/Base.watch.lproj");
		} else {
			ninja.WriteLine ("build $appdir/Base.lproj: cp-recursive $monoios_dir/Base.lproj");
		}

		ninja.Close ();

		GenMain (builddir, assembly_names, isinterpany);
	}
}
