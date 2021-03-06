#!/bin/sh
###############################################################################
###  This is a sample PBS job script for Serial C/F90 program                 #
###  1. To use GNU Compilers (Default)					      #
###     gcc hello.c -o hello-gcc					      #
###     gfortran hello.f90 -o hello-gfortran				      #
###  2. To use PGI Compilers						      #
###     module load pgi							      #
###     pgcc hello.c -o hello-pgcc					      #
###     pgf90 hello.f90 -o hello-pgf90					      #
###  3. To use Intel Compilers						      #
###     module load intel					              #
###     icc hello.c -o hello-icc        				      #
###     ifort hello.f90 -o hello-ifort					      #
###############################################################################

### Job name
#PBS -N hugewiki_handler
### Declare job non-rerunable
#PBS -r n
#PBS -k oe

###  Queue name (debug, parallel or fourday)   ################################
###    Queue debug   : Walltime can be  00:00:01 to 00:30:00                  #
###    Queue parallel: Walltime can be  00:00:01 to 24:00:00                  #
###    Queue fourday : Walltime can be  24:00:01 to 96:00:00                  #
###  #PBS -q parallel                                                         #
###############################################################################
#PBS -q hugemem

###  Wall time required. This example is 30 min  ##############################
###  #PBS -l walltime=00:30:00                   			      #
###############################################################################
#PBS -l walltime=96:00:00

###  Number of node and cpu core  #############################################
###  For serial program, 1 core is used					      #
###  #PBS -l nodes=1:ppn=1						      #
###############################################################################
#PBS -l nodes=1:ppn=40

###############################################################################
#The following stuff will be executed in the first allocated node.            #
#Please don't modify it                                                       #
#                                                                             #
echo $PBS_JOBID : `wc -l < $PBS_NODEFILE` CPUs allocated: `cat $PBS_NODEFILE`
PATH=$PBS_O_PATH
JID=`echo ${PBS_JOBID}| sed "s/.hpc2015-mgt.hku.hk//"`
###############################################################################

echo ===========================================================
echo "Job Start  Time is `date "+%Y/%m/%d -- %H:%M:%S"`"

cd $PBS_O_WORKDIR

######################################
cd build

output_mmc="1"
output_csr="1"

OUTFILE="hugewiki_handler.txt"
time ./hugewiki_handler --output_mmc $output_mmc --output_csr $output_csr >> ${OUTFILE}
mv $OUTFILE $log_file $PBS_O_WORKDIR

######################################

echo "Job Finish Time is `date "+%Y/%m/%d -- %H:%M:%S"`"
mv $HOME/${PBS_JOBNAME}.e${JID} $HOME/${PBS_JOBNAME}.o${JID} $PBS_O_WORKDIR
exit 0