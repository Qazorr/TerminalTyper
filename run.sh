#!/usr/bin/bash

ENTRY=main.cpp
HELPER=typer.cpp
EXE=main.x

function compile() {
    echo "Compiling $ENTRY with $HELPER"
    g++ $ENTRY $HELPER -o $EXE
}

function run() {
    echo "Running..."
    ./$EXE
    rm -f $EXE
}

compile
run