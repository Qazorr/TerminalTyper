#!/bin/bash

# Set the name of CMake build directory
BUILD_DIR="build"
EXECUTABLE_NAME="TerminalTyper"
LOGS_DIR="logs"

function build() {
    # Change to the build directory
    cd $BUILD_DIR || exit

    # Generate the build files using CMake
    cmake ..

    # Build the project
    make

    cd ..
}

function rebuild() {
    rm -rf $BUILD_DIR

    # Create the build directory if it doesn't exist
    mkdir -p $BUILD_DIR

    build
}

function run() {
    # Check if the build directory exists
    if [ -d "$BUILD_DIR" ]; then
        # Change to the build directory
        cd $BUILD_DIR || exit

        # Check if the executable exists
        if [ -x "$EXECUTABLE_NAME" ]; then
            # Run the executable
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

function usage() {
    echo "Usage: $0 [OPTION]"
    echo "Options:"
    echo "  --build        Build the project"
    echo "  --rebuild      Clean and rebuild the project"
    echo "  --no-logs      Delete logs after a successful run"
    echo "  --help         Show usage instructions"
}

# Handle command line arguments
if [ "$1" == "--rebuild" ]; then
    rebuild
    shift
fi

if [ "$1" == "--build" ]; then
    build
    shift
fi

if [ "$1" == "--format" ]; then
    format
    shift
fi

if [ "$1" == "--no-logs" ]; then
    run "--no-logs"
    shift
fi

if [ "$1" == "--help" ]; then
    usage
    exit 0
fi

run
