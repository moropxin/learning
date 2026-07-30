#!/bin/bash
set -e
DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$DIR"

echo "Compiling..."
gcc -Wall -O2 stack.c -o stack
gcc -Wall -O2 queue.c -o queue
gcc -Wall -O2 parentheses.c -o parentheses
gcc -Wall -O2 recursion.c -o recursion

echo "Running parentheses..."
./parentheses

echo "Running stack..."
./stack

echo "Running queue..."
./queue

echo "Running recursion..."
./recursion

echo "All tests ran successfully."
