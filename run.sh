#!/usr/bin/bash

ENTRY=main.cpp
HELPERS=`ls ./src/*.cpp`
EXE=main.x

function compile() {
    echo "Compiling $ENTRY with $HELPERS"
    g++ $ENTRY $HELPERS -std=c++17 -o $EXE
}

function run() {
    ./$EXE
    rm -f $EXE
}

compile
run