#include <mono/jit/jit.h>
#include <mono/metadata/environment.h>
#include <mono/metadata/object.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/utils/mono-publib.h>
#include <stdlib.h>

/*
 * Very simple mono embedding example.
 * Compile with: 
 * 	gcc -o teste teste.c `pkg-config --cflags --libs mono-2` -lm
 * 	mcs test.cs
 * Run with:
 * 	./teste test.exe
 */

static MonoAssembly *assembly;

typedef void (*VoidFtn) (void);

static MonoString*
gimme () {
	MonoImage *image = mono_assembly_get_image (assembly);
	MonoClass *klass = mono_class_from_name (image, "", "MonoEmbed");
	MonoMethodDesc *mdesc = mono_method_desc_new (":ThrowException()", 1 != 0);
	MonoMethod *throwing = mono_method_desc_search_in_class (mdesc, klass);
	VoidFtn throwing_ftn = (void (*) (VoidFtn)) (mono_compile_method (throwing));
	throwing_ftn ();
	return mono_string_new (mono_domain_get (), "All your monos are belong to us!");
}

extern int mono_use_interpreter;

static void main_function (MonoDomain *domain, const char *file, int argc, char** argv)
{
	assembly = mono_domain_assembly_open (domain, file);
	if (!assembly)
		exit (2);
	/*
	 * mono_jit_exec() will run the Main() method in the assembly.
	 * The return value needs to be looked up from
	 * System.Environment.ExitCode.
	 */
	mono_jit_exec (domain, assembly, argc, argv);
}

static int malloc_count = 0;

static void* custom_malloc(size_t bytes)
{
	++malloc_count;
	return malloc(bytes);
}

int 
main(int argc, char* argv[]) {
	MonoDomain *domain;
	const char *file;
	int retval;
	
	if (argc < 2){
		fprintf (stderr, "Please provide an assembly to load\n");
		return 1;
	}
	file = argv [1];

	MonoAllocatorVTable mem_vtable = {custom_malloc};
	mono_set_allocator_vtable (&mem_vtable);

	int i = mono_enable_interp (NULL);

	/*
	 * Load the default Mono configuration file, this is needed
	 * if you are planning on using the dllmaps defined on the
	 * system configuration
	 */
	mono_config_parse (NULL);
	/*
	 * mono_jit_init() creates a domain: each assembly is
	 * loaded and run in a MonoDomain.
	 */
	domain = mono_jit_init (file);
	/*
	 * We add our special internal call, so that C# code
	 * can call us back.
	 */
	mono_add_internal_call ("MonoEmbed::gimme", gimme);

	main_function (domain, file, argc - 1, argv + 1);
	
	retval = mono_environment_exitcode_get ();
	
	mono_jit_cleanup (domain);

	fprintf (stdout, "custom malloc calls = %d\n", malloc_count);

	return retval;
}

