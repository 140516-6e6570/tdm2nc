#!/bin/bash

# Run this inside the specific texture folder you want to flatten.
# It will process ALL files in subdirectories, regardless of extension.

# Find all files recursively, ignoring files already in the root (mindepth 2)
find . -mindepth 2 -type f -print0 | while IFS= read -r -d '' file; do

    # Get the filename (e.g., "image.tga")
    filename=$(basename "$file")

    # Get the directory path relative to current dir (e.g., "./rug/coarse")
    dirpath=$(dirname "$file")

    # Clean up the path:
    # 1. Remove leading "./"
    # 2. Replace "/" with "_"
    # Result: "rug_coarse"
    prefix=$(echo "$dirpath" | sed 's@^\./@@; s@/@_@g')

    # Construct new filename (e.g., "rug_coarse_image.tga")
    new_name="${prefix}_${filename}"

    # Move and rename the file to the current directory
    # -n prevents overwriting if a file with the same name already exists
    # -v enables verbose output
    mv -nv "$file" "./$new_name"
done

# Optional: Clean up empty directories left behind
find . -type d -empty -delete
