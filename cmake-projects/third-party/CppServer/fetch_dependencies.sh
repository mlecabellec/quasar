#!/bin/bash

# Exit on any error
set -e

GITLINKS_FILE="${1:-.gitlinks}"

if [ ! -f "$GITLINKS_FILE" ]; then
    echo "Error: $GITLINKS_FILE not found"
    exit 1
fi

BASE_DIR="$(dirname "$GITLINKS_FILE")"

echo "Fetching dependencies directly from $GITLINKS_FILE avoiding .git submodules..."

# Read .gitlinks line by line, ignoring comments and empty lines
grep -v '^#' "$GITLINKS_FILE" | grep -v '^$' | while read -r line; do
    # Split the line by spaces
    read -r name target url branch subpath1 subpath2 <<< "$line"

    echo "Processing $name..."

    # Special case for swagger-ui which has a subpath arg
    if [ -n "$subpath1" ] && [ -n "$subpath2" ]; then
        echo "  Cloning $name into temporary directory..."
        TEMP_DIR=$(mktemp -d)
        git clone --depth 1 --branch "$branch" "$url" "$TEMP_DIR"
        
        echo "  Copying $subpath1 to $subpath2..."
        mkdir -p "$(dirname "$subpath2")"
        cp -r "$TEMP_DIR/$subpath1" "$subpath2"
        
        echo "  Cleaning up temporary directory..."
        rm -rf "$TEMP_DIR"
    else
        echo "  Cloning $name ($branch) into $BASE_DIR/$target..."
        # If the target directory already exists, clear it
        if [ -d "$BASE_DIR/$target" ]; then
            rm -rf "$BASE_DIR/$target"
        fi
        
        # Clone heavily shallow for speed since we don't need history
        git clone --depth 1 --branch "$branch" "$url" "$BASE_DIR/$target"
        
        # Remove the .git directory so it acts as a plain folder
        echo "  Removing .git directory from $BASE_DIR/$target..."
        rm -rf "$BASE_DIR/$target/.git"
    fi
done

echo "All dependencies fetched successfully."
