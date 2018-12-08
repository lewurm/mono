#!/bin/bash

make -j8 -C mono
mcs repro.cs
./mono/mini/mono-sgen repro.exe
