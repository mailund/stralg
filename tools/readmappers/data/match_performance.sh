#!/bin/bash

## Modify here to add or remove mappers or change options
## =============================================================

# max edit distance to explore
d=2
# number of time measurements to do
N=5



# Mappers
declare -a mappers=(
    BWA
    BWT
)
declare -a preprocess_commands=(
    'bwa index ${ref}'
    '../bwt_readmapper/bwt_readmapper -p ${ref}'
)
declare -a mapper_commands=(
    'bwa mem ${ref} ${reads}'
    '../bwt_readmapper/bwt_readmapper -d $d ${ref} ${reads}'
)

# Reference genomes
declare -a references=(
    genomes/hg38-10000.fa
    genomes/hg38-100000.fa
    #genomes/hg38-1000000.fa
)

# Reference genomes
declare -a read_files=(
    reads/reads-100-10-0.fq
    reads/reads-100-10-1.fq
    reads/reads-100-100-1.fq
    reads/reads-100-100-2.fq
    reads/reads-1000-100-1.fq
    reads/reads-1000-100-2.fq
    reads/reads-1000-200-1.fq
)




## =============================================================

## It shouldn't be necessary to touch any of the code below.
## If it is, let me know, so I can adapt it such that it will
## not be necessary in the future.

report_file=readmapping-report.txt
log_file=readmapping.log

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

printf "\n$(tput bold)PREPROCESSING$(tput sgr0)\n\n"


for i in ${!references[@]}; do
    ref=${references[$i]}
    printf "$(tput bold)Preprocessing $(tput setaf 4)${ref}$(tput sgr0)\n"
    for j in ${!mappers[@]}; do
        name=${mappers[$j]}
        mapper=`eval echo ${preprocess_commands[$j]}`
        printf "   • Mapper $(tput setaf 2)${name}$(tput sgr0) "
        echo "${mapper}" >> ${log_file}
        ${mapper} >> $log_file 2>> $log_file
        if [ $? -ne 0 ]; then
            failure_tick "Preprocessing failed. Check $(tput setaf 4)$(tput bold)`basename ${log_file}`$(tput sgr0) for further information."
        fi
        success
    done
    echo
done

printf "\n$(tput bold)MAPPING$(tput sgr0)\n\n"

for i in ${!references[@]}; do
    ref=${references[$i]}
    printf "$(tput bold)Reference $(tput setaf 4)${ref}$(tput sgr0)\n"
    for k in ${!read_files[@]}; do
        reads=${read_files[$k]}
        printf "   • Reads $(tput setaf 3)${reads}$(tput sgr0)\n"
        for j in ${!mappers[@]}; do
            name=${mappers[$j]}
            mapper=`eval echo ${mapper_commands[$j]}`
            printf "      • Mapper $(tput setaf 2)${name}$(tput sgr0) "
            echo "${mapper}" >> ${log_file}

            for ((i = 0; i < $N; i++)); do
                #{ time -p "${mapper}" 1&>2 2> /dev/null ; } 2> _time.txt
                { time -p ${mapper} > /dev/null 2> /dev/null; } 2> _time.txt
                if [ $? -ne 0 ]; then
                    failure_tick "Mapping failed. Check $(tput setaf 4)$(tput bold)`basename ${log_file}`$(tput sgr0) for further information."
                fi
                walltime=$(awk < _time.txt '/^real/ { print $2 }')
                rm _time.txt

                
                echo -n .
                
                echo ${name} ${ref} ${reads} ${walltime} >> $report_file
                
            done
            success
        done
        echo
    done
done
