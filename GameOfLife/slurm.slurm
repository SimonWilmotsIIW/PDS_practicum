#! /bin/bash -l
#SBATCH --cluster=wice
#SBATCH --account=lp_h_pds_iiw
#SBATCH --nodes=1
#SBATCH --cpu-per-task=2
#SBATCH --error="%x.e%A"
#SBATCH --output="%x.o%A"

source /data/leuven/303/vsc30380/slurmhooks

module purge

module load cluster/wice/batch
module load GCC/10.3.0
module load MPICH/3.4.2-GCC-10.3.0

make main

mpiexec -n 9 ./main testboard.txt

/data/leuven/303/vsc30380/slurmquote.py
