#!/bin/bash
# Transcode extracted game assets: png->pngquant, ogg->vorbis q0,
# jpg->mjpeg q5, ngs->mpeg1video 2.5M/mp2 96k. In-place via temp file.
W=${1:?usage: compress_assets.sh <extracted-dir>}

find "$W/cg" -name '*.png' -print0 | xargs -0 -P 8 -I{} sh -c '
  pngquant --quality=60-85 --speed 3 --force --output "{}.q" "{}" 2>/dev/null \
    && mv "{}.q" "{}" || rm -f "{}.q"' &
PNG_PID=$!

find "$W/voice" "$W/sound" -name '*.ogg' -print0 | xargs -0 -P 8 -I{} sh -c '
  ffmpeg -v error -y -i "{}" -c:a libvorbis -q:a 0 "{}.t.ogg" 2>/dev/null \
    && mv "{}.t.ogg" "{}" || rm -f "{}.t.ogg"' &
OGG_PID=$!

find "$W/cg" -name '*.jpg' -print0 | xargs -0 -P 8 -I{} sh -c '
  ffmpeg -v error -y -i "{}" -c:v mjpeg -q:v 5 "{}.t.jpg" 2>/dev/null \
    && mv "{}.t.jpg" "{}" || rm -f "{}.t.jpg"' &
JPG_PID=$!

find "$W/dx" -name '*.ngs' -print0 | xargs -0 -P 4 -I{} sh -c '
  ffmpeg -v error -y -i "{}" -c:v mpeg1video -b:v 2500k -c:a mp2 -b:a 96k -f mpeg "{}.t.ngs" 2>/dev/null \
    && mv "{}.t.ngs" "{}" || rm -f "{}.t.ngs"' &
NGS_PID=$!

wait $PNG_PID $OGG_PID $JPG_PID $NGS_PID
echo "transcode done"
