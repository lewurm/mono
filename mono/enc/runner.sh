
echo "warning for dotnet/runtime noobs. to rebuild, do"
echo '   > $ ./mono.sh'
echo '   > $ make -C src/mono/netcore patch-mono-dotnet'
echo 'in the repository root'

set -e
set -x

if [ x`uname -s` = xDarwin ]; then
    PLAT=OSX
else
    PLAT=Linux
fi
DNILASM=artifacts/bin/coreclr/${PLAT}.x64.Debug/ilasm
DNILDASM=artifacts/bin/coreclr/${PLAT}.x64.Debug/ildasm

d=`pwd`
m=${d}/../..
ILASM=${d}/ilasm
ILDASM=${d}/ildasm
CSC=csc
MONODOTNET=../../../.dotnet-mono/dotnet
MONOMONO=${m}/mono/mini/mono

MONO_LOG_MASK=metadata-update MONO_LOG_LEVEL=debug MONO_VERBOSE_METHOD=Do MONO_ENV_OPTIONS='--debug --interpreter' MONO_PATH=${m}/mcs/class/lib/net_4_x MONO_CONFIG=${m}/runtime/etc/mono/config lldb -- $MONOMONO "$@"
