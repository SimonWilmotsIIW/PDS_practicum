#! /bin/bash -l
#SBATCH --cluster=wice
#SBATCH --account=lp_h_pds_iiw
#SBATCH --ntasks=17
#SBATCH --nodes=3
#SBATCH --cpus-per-task=1
#SBATCH --error="%x.e%A"
#SBATCH --output="./outputs/%x.o%A"

source /data/leuven/303/vsc30380/slurmhooks

module purge

module load cluster/wice/batch
module load GCC/10.3.0
module load OpenMPI/4.1.1-GCC-10.3.0

mpicxx -O3 -DNDEBUG -g -Wall -I/usr/include/lam -o main3_n_17_t main.cpp

mpiexec -n 17 ./main3_n_17_t testboard.txt

/data/leuven/303/vsc30380/slurmquote.py