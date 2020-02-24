#!/bin/bash

echo "warning for dotnet/runtime noobs. to rebuild, do"
echo '   > $ ./mono.sh'
echo '   > $ make -C src/mono/netcore patch-mono-dotnet'
echo 'in the repository root'

set -e
set -x

ILASM=../../../artifacts/bin/coreclr/Linux.x64.Debug/ilasm
ILDASM=../../../artifacts/bin/coreclr/Linux.x64.Debug/ildasm
CSC=csc
MONODOTNET=../../../.dotnet-mono/dotnet

: "${NAME:=Sample1}"
OUT=here

if [[ ! -f $ILASM ]]; then
	echo "please build everything by calling ./build.sh. we need ilasm which is part of CoreCLR";
	exit 1;
fi


$CSC $NAME.cs

$ILDASM $NAME.exe > $NAME.il
rm $NAME.exe

ILASM_ARGS=""
for arg in ${NAME}_v?.il; do
	ILASM_ARGS="$ILASM_ARGS -ENC=$arg"
done

$ILASM -DEBUG -OUT=$OUT.exe $NAME.il $ILASM_ARGS

cp template.runtimeconfig.json $OUT.runtimeconfig.json

MONO_VERBOSE_METHOD=Do MONO_ENV_OPTIONS='--debug --interpreter' lldb -- $MONODOTNET $OUT.exe
