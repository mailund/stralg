#!/bin/bash

## Modify here to add or remove mappers or change options
## =============================================================

# Mappers
declare -a mappers=(
    BWA
    BWTv2
)
declare -a mapper_commands=(
    "bwa index"
    "../bwt_readmapper/bwt_readmapper -p"
)

# Reference genomes
declare -a references=(
    genomes/hg38-1000.fa
    genomes/hg38-10000.fa
    genomes/hg38-100000.fa
    #genomes/hg38-1000000.fa
)

# max edit distance to explore
d=3

# number of time measurements to do
N=5



## =============================================================

## It shouldn't be necessary to touch any of the code below.
## If it is, let me know, so I can adapt it such that it will
## not be necessary in the future.

report_file=preprocessing-report.txt
log_file=preprocessing.log

## IO code
function success() {
	printf "$(tput setaf 2)$(tput bold)✔$(tput sgr0)\n"
}
function failure() {
	err_msg=$1
	echo "$(tput setaf 1)$(tput bold)↪$(tput sgr0) " $err_msg
	echo
	exit 1
}
function failure_tick() {
	err_msg=$1
	printf "$(tput setaf 1)$(tput bold)✘$(tput sgr0)\n\t"
	failure "${err_msg}"
}

if [ -e $report_file ]; then
    rm $report_file
fi
if [ -e $log_file ]; then
    rm $log_file
fi

for i in ${!references[@]}; do
    ref=${references[$i]}
    printf "$(tput bold)Reference $(tput setaf 4)${ref}$(tput sgr0)\n"
    for j in ${!mappers[@]}; do
        name=${mappers[$j]}
        mapper=${mapper_commands[$j]}
        printf "   • Mapper $(tput setaf 2)${name}$(tput sgr0) "
        for ((i = 0; i < $N; i++)); do
            
            echo "${mapper} ${ref}" >> ${log_file}
            { time -p ${mapper} ${ref} ${mapper} 2>&1 2>> $log_file ; } 2> _time.txt
            
            if [ $? -ne 0 ]; then
                rm _time.txt
                failure_tick "Preprocessing failed. Check $(tput setaf 4)$(tput bold)`basename ${log_file}`$(tput sgr0) for further information."
            fi
            
            walltime=$(awk < _time.txt '/^real/ { print $2 }')
            rm _time.txt
            # this is potentially dangerous but should remove
            # preprocessed files that the mapper might check for
            # and not preprocess if they exist
            rm ${ref}.*
            
            echo ${name} ${ref} ${walltime} >> $report_file
            echo -n .
        done
        success
    done
    echo
done
