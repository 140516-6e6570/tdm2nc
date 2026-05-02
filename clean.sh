#!/bin/bash

# Define source and destination
SOURCE_DIR="work"
DEST_DIR="tdm.pk3dir"

# Create the destination directory
rm -r "$DEST_DIR/default.cfg"
rm -r "$DEST_DIR/af"
rm -r "$DEST_DIR/def"
rm -r "$DEST_DIR/fx"
rm -r "$DEST_DIR/guis"
rm -r "$DEST_DIR/prefabs"
rm -r "$DEST_DIR/script"
rm -r "$DEST_DIR/skins"
rm -r "$DEST_DIR/sound/*.sndshd"
rm -r "$DEST_DIR/strings"
rm -r "$DEST_DIR/subtitles"
rm -r "$DEST_DIR/video"
rm -r "$DEST_DIR/xdata"
