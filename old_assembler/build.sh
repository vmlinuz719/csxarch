#!/bin/bash

cc="gcc"
copt="-O3"
include="include"
src="src"
out="../obj/csxasm"

$cc $copt -I$include $src/*.c -o$out

