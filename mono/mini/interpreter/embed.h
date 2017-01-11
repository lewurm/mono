#include <mono/metadata/object.h>
#include <mono/metadata/appdomain.h>

void mono_interp_init ();

int
mono_interp_exec(MonoDomain *domain, MonoAssembly *assembly, int argc, char *argv[]);

void        
mono_interp_cleanup(MonoDomain *domain);

