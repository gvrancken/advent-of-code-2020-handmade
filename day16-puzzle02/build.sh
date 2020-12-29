#!/bin/sh

DISABLED_COMPILER_WARNINGS="
-Wno-c++11-compat-deprecated-writable-strings 
-Wno-missing-braces 
-Wno-unused-function 
-Wno-unused-variable 
-Wno-null-dereference
"

clang -g -O3 -Wall -Wshadow ${DISABLED_COMPILER_WARNINGS} -o osx_main osx_main.c