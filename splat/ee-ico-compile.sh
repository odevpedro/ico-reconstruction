#!/bin/bash
# ee-ico-compile.sh — compile C to MIPS R5900 (ICO-style) via Docker
# Usage: ./ee-ico-compile.sh -S file.c   # output to stdout
#        echo 'int f(int x){return x+1;}' | ./ee-ico-compile.sh -S

IMAGE="ps2-gcc-295"
B_ARGS="-B/tmp/ps2/bin/ -B/tmp/ps2/lib/gcc-lib/mipsEEel-linux/2.95.2/"
FLAGS="-O2 -G0 -mips3 -mgp64 -msingle-float -mcpu=r5900 -fno-pic -mno-abicalls"

WORKDIR="/tmp/ico_work_$$"
mkdir -p "$WORKDIR"

if [ ! -t 0 ]; then
    cat > "$WORKDIR/input.c"
    docker run --rm -i -v "$WORKDIR:/work" "$IMAGE" \
        /tmp/ps2/bin/ee-gcc $B_ARGS $FLAGS -S /work/input.c -o /work/output.s 2>/dev/null
    cat "$WORKDIR/output.s" 2>/dev/null
    rm -rf "$WORKDIR"
else
    cp "$1" "$WORKDIR/input.c" 2>/dev/null
    shift
    docker run --rm -i -v "$WORKDIR:/work" "$IMAGE" \
        /tmp/ps2/bin/ee-gcc $B_ARGS $FLAGS -S /work/input.c -o /work/output.s 2>/dev/null
    cat "$WORKDIR/output.s" 2>/dev/null
    rm -rf "$WORKDIR"
fi
