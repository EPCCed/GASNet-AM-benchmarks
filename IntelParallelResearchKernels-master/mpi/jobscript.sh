#!/bin/bash --login

#PBS -N mpi_stencil
#PBS -l select=16
#PBS -l walltime=01:00:00
#PBS -A y14

export PBS_O_WORKDIR=$(readlink -f $PBS_O_WORKDIR)
cd $PBS_O_WORKDIR

MAX_NUMBER=19
ARRAY_BASE_SIZE=1000
CORES=()
ARRAY_SIZES=()

for i in {1..19}; do
    CORES+=("$(( i * i ))")
    ARRAY_SIZES+=("$(( i * ARRAY_BASE_SIZE ))")
done

echo "cores  : ${CORES[@]}"
echo "lengths: ${ARRAY_SIZES[@]}"
echo ""

for i in {0..18}; do
    echo -e "${CORES[$i]}\t$(aprun -n ${CORES[$i]} ./stencil 1000 ${ARRAY_SIZES[$i]} | grep MFlops)"
done
