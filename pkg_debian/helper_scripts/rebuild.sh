#!/bin/bash
################## This scripts rebuilds the debian pkg ####################
# Base source and destination directories
SRC_BASE=~/Desktop/go-back-n
DEST_BASE=/home/prtihijit/Desktop/go-back-n/pkg_debian/udp-ft-suite/opt/gobackn
PKG_DIR=/home/prtihijit/Desktop/go-back-n/pkg_debian
PKG_NAME=udp-ft-suite

# Iterate through each directory in the source base
for dir in "$SRC_BASE"/*; do
    # Check if it's a directory and has a build folder
    if [[ -d "$dir/build" ]]; then
        folder_name=$(basename "$dir")
        src_build="$dir/build"
        dest_build="$DEST_BASE/$folder_name/build"

        # Create the destination directory if it doesn't exist
        mkdir -p "$dest_build"

        # Copy *_exe files
        cp "$src_build"/*_exe "$dest_build" 2>/dev/null

        # Copy ClientUI if it exists
        if [[ -f "$src_build/ClientUI" ]]; then
            cp "$src_build/ClientUI" "$dest_build"
            echo "Copied ClientUI from $src_build to $dest_build"
        fi

        echo "Copied *_exe files from $src_build to $dest_build"
    fi
done

# Change directory and build Debian package
cd "$PKG_DIR" || exit 1
echo "Building the Debian package in $PKG_DIR..."
dpkg-deb --build "./$PKG_NAME"
