#!/usr/bin/env bash
set -o pipefail

if [ $# -ne 2 ]; then
    echo "Usage: $0 <decoder_path> <asm_folder>"
    exit 1
fi

DECODER="$1"
ASM_DIR=$(readlink -m $2)

if [ ! -x "$DECODER" ]; then
    echo "Error: decoder not found or not executable: $DECODER"
    exit 1
fi

if [ ! -d "$ASM_DIR" ]; then
    echo "Error: asm folder not found: $ASM_DIR"
    exit 1
fi


shopt -s nullglob

ASM_FILES=( "$ASM_DIR"/*.asm )

if [ ${#ASM_FILES[@]} -eq 0 ]; then
    echo "No .asm files found in $ASM_DIR"
    exit 1
fi

for ASM in "${ASM_FILES[@]}"; do
    echo "== Testing $ASM =="

    touch "$ASM.bin"
    nasm -f bin "$ASM" -o "$ASM.bin"

    if diff "$ASM" <("$DECODER" "$ASM.bin"); then
        echo "SUCCESS"
    else
        echo "ERROR"
    fi
done

