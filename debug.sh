#!/bin/sh
# use clang without setting global vars.
CC=clang CXX=clang++ cmake -DBUILD_TYPE=Debug -DUSE_AMALGAMATION=OFF . && make -j4 && make test

