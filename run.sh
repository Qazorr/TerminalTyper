#!/usr/bin/bash

ENTRY=main.cpp
TYPER=typer.cpp
GENERATOR=generator.cpp
EXE=main.x

function compile() {
    echo "Compiling $ENTRY with $TYPER and $GENERATOR"
    g++ $ENTRY $TYPER $GENERATOR -o $EXE
}

function run() {
    ./$EXE
    rm -f $EXE
}

compile
run