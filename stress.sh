#!/bin/bash
# Usage: ./stress.sh [iterations]
# Needs gen.cpp, sol.cpp (fast solution), brute.cpp (slow but correct) in the same folder.
# Edit GEN_ARGS to change the kind of graphs; the seed is appended automatically.

GEN_ARGS="type=graph nmin=2 n=8 mmin=1 m=12 weighted wl=1 wr=10"

set -e
g++ -O2 -std=c++17 -o gen gen.cpp
g++ -O2 -std=c++17 -fsanitize=address,undefined -o sol sol.cpp
g++ -O2 -std=c++17 -o brute brute.cpp
set +e

for ((i = 1; i <= ${1:-1000}; i++)); do
    ./gen seed=$i $GEN_ARGS > in.txt
    timeout 5 ./sol < in.txt > out.txt
    rc=$?
    ./brute < in.txt > ans.txt
    if [ $rc -ne 0 ]; then
        echo "Runtime error / TLE on seed $i (exit code $rc)"
        cat in.txt
        exit 1
    fi
    if ! diff -qZ out.txt ans.txt > /dev/null; then
        echo "Mismatch on seed $i"
        echo "--- input ---";  cat in.txt
        echo "--- yours ---";  cat out.txt
        echo "--- brute ---";  cat ans.txt
        exit 1
    fi
done
echo "All tests passed"
