#!/bin/sh

DISABLED_COMPILER_WARNINGS="
-Wno-c++11-compat-deprecated-writable-strings 
-Wno-missing-braces 
-Wno-unused-function 
-Wno-unused-variable 
-Wno-null-dereference
"

clang -g -O0 -Wall -Wshadow ${DISABLED_COMPILER_WARNINGS} -std=c99 -o osx_main osx_main.c