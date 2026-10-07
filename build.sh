#!/bin/env bash

input="$1"
output="${input%.*}"

if [[ "$#" -eq 2 ]]; then
    lib="$2"
fi

cmd=()

cc=("gcc")
optimize=("-O2")
debug=("-Wall" "-Wpedantic")
path_include=("-I/usr/local/include")
path_lib=("-L/usr/local/lib")
linker=("-lraylib" "-lm" "-lGL" "-lX11" "-lpthread")

if [[ -f "$input" ]]; then
    cmd+=("${cc[@]}" "-o" "$output")

    if [[ "${#lib[@]}" -gt 0 ]]; then
	cmd+=("$input" "$lib")
    else
	cmd+=("$input")
    fi

    cmd+=("${optimize[@]}")
    cmd+=("${debug[@]}")
    cmd+=("${path_include[@]}")
    cmd+=("${path_lib[@]}")
    cmd+=("${linker[@]}")

    echo
    echo "########################"
    echo "cmd: ${cmd[@]}"
    echo

    if "${cmd[@]}"; then
	echo
	[[ -f "$output" ]] && echo "status: Compilation finished"
	echo "########################"
	echo
	./"$output"
    else
	echo
	echo "status: Compilation failed"
	echo "########################"
	echo
	exit 1
    fi
else
    echo
    echo "error: $input: not found"
    echo "########################"
    echo
    exit 1
fi
