#!/bin/bash

BUILD_DIR="build"
EXECUTABLE_NAME="TerminalTyper"
LOGS_DIR="logs"

function build() {
    cd $BUILD_DIR || exit
    cmake ..
    make
    cd ..
}

function rebuild() {
    rm -rf $BUILD_DIR
    mkdir -p $BUILD_DIR
    build
}

function run() {
    if [ -d "$BUILD_DIR" ]; then
        cd $BUILD_DIR || exit

        if [ -x "$EXECUTABLE_NAME" ]; then
            cd ..

            ./$BUILD_DIR/$EXECUTABLE_NAME
            local return_code=$?

            # Delete logs if --no-logs option is provided and the run was successful
            if [ "$return_code" -eq 0 ] && [ "$1" == "--no-logs" ]; then
                rm -rf "$LOGS_DIR"
                echo "Logs deleted."
            fi

            exit $return_code
        else
            echo "Executable '$EXECUTABLE_NAME' not found in the build directory."
            echo "Please run '$0 --build' to generate the executable."
            exit 1
        fi
    else
        echo "Build directory not found. Please run '$0 --rebuild' to generate the build files."
        exit 1
    fi
}

format_file() {
    echo "Formatting file: $1"
    clang-format -i "$1"
}

function format() {
    cd "$(dirname "$0")" || exit

    find . \( -iname "*.cpp" -o -iname "*.h" \) -not -path "./build/*" -print0 | while IFS= read -r -d '' file; do
        format_file "$file"
    done

}

function usage() {
    echo "Usage: $0 [OPTION]"
    echo "Options:"
    echo "  --build        Build the project"
    echo "  --rebuild      Clean and rebuild the project"
    echo "  --no-logs      Delete logs after a successful run"
    echo "  --format       Run clang-format for every file in the project"
    echo "  --help         Show usage instructions"
}

# Handle command line arguments
if [ "$1" == "--rebuild" ]; then
    rebuild
    exit 0
fi

if [ "$1" == "--build" ]; then
    build
    exit 0
fi

if [ "$1" == "--format" ]; then
    format
    exit 0
fi

if [ "$1" == "--no-logs" ]; then
    run "--no-logs"
    exit 0
fi

if [ "$1" == "--help" ]; then
    usage
    exit 0
fi

run
