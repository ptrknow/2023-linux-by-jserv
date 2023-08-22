#!/bin/bash

STREE_PROG=s-tree.out
RBTREE_PROG=rbtree.out
AVLTREE_PROG=avltree.out

if [ ! -x ${STREE_PROG} ] || [ ! -x ${AVLTREE_PROG} ] || [ ! -x ${RBTREE_PROG} ]; then
    echo "At least one of executables are missing! Please run \`make\` again!"
    exit 1
fi

############################################################################################
# This script runs the benchmarking mode of the following 3 search tree implementations:
# 1) S-tree
# 2) Red-black tree
# 3) AVL tree
#
# For each of the tree, total insertion delay, total search delay and total deletion delay 
# will be measured on the following size of nodes:
# 100, 1000, 5000, 10000, 50000, 100000, 500000, 1000000, 5000000, 10000000
#
# Each total delay will then be averaged over 10 iterations
############################################################################################

declare -a NNODES_ARR=("100" "1000" "5000" "10000" "50000" "100000" "500000" "1000000" "5000000" "10000000")
ITER="10"

echo "==========="
echo "Test S-Tree"
echo "==========="
echo

for i in "${NNODES_ARR[@]}"
do
    echo
    echo "  <----- Size = ${i}, Iteration = ${ITER} ----->"
    ./${STREE_PROG} -n ${i} -l ${ITER} -b
done

echo
echo "==================="
echo "Test Red-Black Tree"
echo "==================="
echo

for i in "${NNODES_ARR[@]}"
do
    echo
    echo "  <----- Size = ${i}, Iteration = ${ITER} ----->"
    ./${RBTREE_PROG} -n ${i} -l ${ITER} -b
done

echo
echo "=============="
echo "Test AVL Tree"
echo "=============="
echo

for i in "${NNODES_ARR[@]}"
do
    echo
    echo "  <----- Size = ${i}, Iteration = ${ITER} ----->"
    ./${AVLTREE_PROG} --size=${i} --repeat=${ITER} --insert=random --delete=same --test=benchmark
    ./${AVLTREE_PROG}  --size=${i} --repeat=${ITER} --insert=ascending --delete=same --test=benchmark
    ./${AVLTREE_PROG}  --size=${i} --repeat=${ITER} --insert=descending --delete=same --test=benchmark
    ./${AVLTREE_PROG}  --size=${i} --repeat=${ITER} --insert=zigzag --delete=same --test=benchmark
done
