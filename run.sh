#!/bin/bash
 
SRC_PATH=src
GENERATOR=generator
TYPER=typer
DELETE_AFTER=1
 
function usage() {
echo "$0 [e] [h]"
echo '-e     delete binary file after'
echo '-h     show this message'
}
 
while getopts "eh" arg
do
case $arg in
    e)
        echo 'Delete after finish flag is set'
        DELETE_AFTER=0
        ;;
    h)
        usage
        exit 0    
        ;;    
esac
done
 
# Cleanup
function do_clean() {
    echo 'Cleaning up..'
    rm -rf $SRC_PATH/*.obj *.obj 2>/dev/null
}
 
# Check if files exist
function check() {
    if !([ -d "$SRC_PATH" ])
    then
        echo 'No src'
        exit 1        
    fi 
 
    if !([ -f "$SRC_PATH/$GENERATOR.cpp" ]) || !([ -f "$SRC_PATH/$TYPER.cpp" ])
    then
        echo 'No .cpp files'
        cleanup
        exit 1
    fi
}
 
# Run executable and delete if flag was given
function run_and_delete() {
    echo 'Running main.x'
    ./main.x
    if [ $DELETE_AFTER -eq 0 ]
    then
        rm -rf main.x 2>/dev/null
    fi
}
 
# Enumerate the files and compile them
function compile() {
    for cpp_file in $SRC_PATH/$GENERATOR $SRC_PATH/$TYPER main
    do
        echo "Compiling $cpp_file.cpp"
        g++ -std=c++17 -Wall -pedantic -c $cpp_file.cpp -o $cpp_file.obj 
        if [ $? -ne 0 ]
        then
            echo -e "Error/warning while compiling the file: $cpp_file.cpp" 
            do_clean
            exit 1
        fi
    done
    g++ $SRC_PATH/$GENERATOR.obj $SRC_PATH/$TYPER.obj main.obj -o main.x
    do_clean
}
check
compile
run_and_delete $DELETE_AFTER