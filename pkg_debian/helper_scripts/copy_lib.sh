#!/bin/bash
# A Bash script to extract library .so file paths from an ldd output and copy them to a target folder.
#
# Usage:
#    Copy ldd <file exe> Output to <ldd_output_file>
#   ./copy_libs.sh <ldd_output_file>
#
# The script handles two main line formats:
#   1. Lines with "=>" where the file path comes after the arrow.
#      Example: libstdc++.so.6 => /lib/x86_64-linux-gnu/libstdc++.so.6 (0x...)
#   2. Lines that start with an absolute path (e.g., /lib64/ld-linux-x86-64.so.2 (0x...))
#
# Files that are not present on disk (for instance, linux-vdso.so.1 without a real path)
# are skipped.

# Target directory where all the .so files will be copied.
TARGET_DIR="/home/prtihijit/Desktop/go-back-n/pkg_debian/udp-ft-suite/opt/gobackn/lib"

# Create the target directory if it doesn't exist.
mkdir -p "$TARGET_DIR" || { echo "Error: Could not create target directory $TARGET_DIR"; exit 1; }

# Check that the user provided an ldd output file as an argument.
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <ldd_output_file>"
    exit 1
fi

INPUT_FILE="$1"

# Read the file line by line.
while IFS= read -r line; do
    # Trim leading/trailing whitespace.
    line=$(echo "$line" | sed 's/^[[:space:]]*//;s/[[:space:]]*$//')

    # Variable to hold the library file path.
    libpath=""

    # Check if the line contains "=>"
    if [[ "$line" == *"=>"* ]]; then
        # Use awk to grab the field after "=>" and before the following whitespace.
        libpath=$(echo "$line" | awk -F'=> ' '{print $2}' | awk '{print $1}')
    else
        # Else take the first token (this handles lines starting with an absolute path).
        libpath=$(echo "$line" | awk '{print $1}')
    fi

    # Check if the file name contains ".so" (this ensures we only target shared libraries)
    if [[ "$libpath" == *".so"* ]]; then
        # Check if the file exists. Some libraries (like linux-vdso.so.1) are not present on disk.
        if [ -f "$libpath" ]; then
            cp "$libpath" "$TARGET_DIR"
            echo "Copied: $libpath to $TARGET_DIR"
        else
            echo "File not found or not accessible: $libpath, skipping..."
        fi
    fi

done < "$INPUT_FILE"
