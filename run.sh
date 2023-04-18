#!/bin/bash

SRC_PATH=src
GENERATOR=generator
TYPER=typer
LOGGER=logger
DELETE_AFTER=1
DELETE_LOGS=1
DELETE_RESULT=1

function usage() {
    echo "$0 [OPTION]"
    echo '-e, --erase-binary     delete binary file after'
    echo '-l                     delete source log files'
    echo '-r                     delete results log file'
    echo '--erase-log            combined -l and -r'
    echo '-h                     show this message'
}

# handle arguments
while getopts ":elrh-:" opt; do
    case $opt in
    e | erase-binary)
        echo 'Delete after finish flag is set'
        DELETE_AFTER=0
        ;;
    l)
        echo 'Delete source log files flag is set'
        DELETE_LOGS=0
        ;;
    r)
        echo 'Delete results log file flag is set'
        DELETE_RESULT=0
        ;;
    h)
        usage
        exit 0
        ;;
    -)
        case "${OPTARG}" in
        erase-log)
            echo 'Erase all log files'
            DELETE_LOGS=0
            DELETE_RESULT=0
            ;;
        erase-binary)
            echo 'Delete after finish flag is set'
            DELETE_AFTER=0
            ;;
        *)
            echo "Invalid option: --${OPTARG}" >&2
            usage
            exit 1
            ;;
        esac
        ;;
    \?)
        echo "Invalid option: -$OPTARG" >&2
        exit 1
        usage
        ;;
    :)
        echo "Option -$OPTARG requires an argument." >&2
        exit 1
        ;;
    esac
done
shift $((OPTIND - 1))

# Cleanup
function do_clean() {
    echo 'Cleaning up..'
    rm -rf $SRC_PATH/*.obj *.obj 2>/dev/null
}

# Check if files exist
function check() {
    if !([ -d "$SRC_PATH" ]); then
        echo 'No src'
        exit 1
    fi

    if !([ -f "$SRC_PATH/$GENERATOR.cpp" ]) || !([ -f "$SRC_PATH/$TYPER.cpp" ]) || !([ -f "$SRC_PATH/$LOGGER.cpp" ]); then
        echo 'No .cpp files'
        cleanup
        exit 1
    fi
}

# Run executable and delete if flag was given
function run_and_delete() {
    if [ $DELETE_LOGS -eq 0 ]; then
        find logs/ ! -name 'results.log' -type f -delete
    fi
    if [ $DELETE_RESULT -eq 0 ]; then
        rm logs/results.log
    fi
    echo 'Running main.x'
    ./main.x
    if [ $DELETE_AFTER -eq 0 ]; then
        rm -rf main.x 2>/dev/null
    fi
}

# Enumerate the files and compile them
function compile() {
    for cpp_file in $SRC_PATH/$GENERATOR $SRC_PATH/$TYPER $SRC_PATH/$LOGGER main; do
        echo "Compiling $cpp_file.cpp"
        g++ -std=c++17 -Wall -pedantic -c $cpp_file.cpp -o $cpp_file.obj
        if [ $? -ne 0 ]; then
            echo -e "Error/warning while compiling the file: $cpp_file.cpp"
            do_clean
            exit 1
        fi
    done
    g++ $SRC_PATH/$GENERATOR.obj $SRC_PATH/$TYPER.obj $SRC_PATH/$LOGGER.obj main.obj -o main.x
    do_clean
}
check
compile
run_and_delete $DELETE_AFTER
