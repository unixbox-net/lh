#!/bin/bash

echo "Starting the build process..."

REPO_URL="https://github.com/unixbox-net/loghog.git"
BUILD_DIR="$HOME/lh-build"

echo "Cleaning up old build directory..."
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo "Cloning the repository..."
git clone "$REPO_URL" loghog || { echo "Failed to clone repository. Exiting."; exit 1; }

cd loghog

echo "Checking for required packages..."
REQUIRED_PACKAGES="gcc make rpm-build readline-devel json-c-devel"
for package in $REQUIRED_PACKAGES; do
    if ! rpm -q $package >/dev/null 2>&1; then
        echo "Installing $package..."
        sudo dnf install -y $package
    fi
done

echo "Attempting to compile the program..."
make || { echo "Compilation failed due to Makefile issues. Exiting."; exit 1; }

echo "Build process completed successfully."
