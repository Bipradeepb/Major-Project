#!/bin/bash
################## This script rebuilds the Debian pkg ####################

# Base source and destination directories
SRC_BASE=~/Desktop/Major-Project/go-back-n
DEST_BASE=/home/prtihijit/Desktop/Major-Project/go-back-n/pkg_debian/udp-gbn-suite/opt/gobackn
PKG_DIR=/home/prtihijit/Desktop/Major-Project/go-back-n/pkg_debian
PKG_NAME=udp-gbn-suite

# Iterate through each directory in the source base
for dir in "$SRC_BASE"/*; do
    if [[ -d "$dir/build" ]]; then
        folder_name=$(basename "$dir")
        src_build="$dir/build"
        dest_build="$DEST_BASE/$folder_name/build"

        mkdir -p "$dest_build"

        cp "$src_build"/*_exe "$dest_build" 2>/dev/null

        if [[ -f "$src_build/ClientUI" ]]; then
            cp "$src_build/ClientUI" "$dest_build"
            echo "Copied ClientUI from $src_build to $dest_build"
        fi

        echo "Copied *_exe files from $src_build to $dest_build"
    fi
done

# Fix permissions before building the package
DEBIAN_DIR="$PKG_DIR/$PKG_NAME/DEBIAN"
chmod 755 "$DEBIAN_DIR"

# Fix control and maintainer script permissions
find "$DEBIAN_DIR" -type f -name 'control' -exec chmod 644 {} \;
find "$DEBIAN_DIR" -type f \( -name 'postinst' -o -name 'prerm' -o -name 'postrm' -o -name 'preinst' \) -exec chmod 755 {} \;

# Optional: fix permissions for binaries/scripts in opt (755 is generally sufficient)
find "$PKG_DIR/$PKG_NAME/opt" -type f -exec chmod 755 {} \;

# Build Debian package
cd "$PKG_DIR" || exit 1
echo "Building the Debian package in $PKG_DIR..."
dpkg-deb --build "./$PKG_NAME"
