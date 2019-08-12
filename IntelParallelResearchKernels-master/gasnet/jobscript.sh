#!/bin/bash --login

#PBS -N gasnet_stencil
#PBS -l select=128
#PBS -l walltime=00:30:00
#PBS -A y14

export PBS_O_WORKDIR=$(readlink -f $PBS_O_WORKDIR)
cd $PBS_O_WORKDIR

ARRAY_BASE_SIZE=1000

# Fill one node, than go in powers of two through nodes (3072 cores = 128 nodes)
CORES=(1 2 4 12 24 48 96 192 384 768 1536 3072)
ARRAY_SIZES=()

for cores in ${CORES[@]}; do
    ARRAY_SIZES+=("$( python -c "import math; print(int(round(math.sqrt($NUM_CORES)*$ARRAY_BASE_SIZE)))" )")
done

echo "cores  : ${CORES[@]}"
echo "lengths: ${ARRAY_SIZES[@]}"
echo ""

for i in {0..18}; do
    echo -e "${CORES[$i]}\t$(aprun -n ${CORES[$i]} ./stencil 1000 ${ARRAY_SIZES[$i]} | grep MFlops)"
done
