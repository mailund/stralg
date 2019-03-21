#!/bin/bash

## Modify here to add or remove mappers or change options
## =============================================================

# make sure the mappers are in your path
export PATH=../mappers_src:$PATH

# list of read-mappers to evaluate
mappers="bwa match_readmapper ac_readmapper bw_readmapper"

# file name for report
report_file=../evaluation-report-exact.txt
log_file=../evaluation-exact.log

# max edit distance to explore
d=0

# number of time measurements to do
N=5

# Reference genome
reference=../data/gorGor3-small-noN.fa

# Reads
reads=../data/sim-reads-d2-tiny.fq

## =============================================================

## It shouldn't be necessary to touch any of the code below.
## If it is, let me know, so I can adapt it such that it will
## not be necessary in the future.

## IO code
mapper_field_length=10
for mapper in $mappers; do
	if [[ $mapper_field_length -lt ${#mapper} ]]; then
		mapper_field_length=${#mapper}
	fi
done

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

## Test that the data is there
printf "Testing that the reference $(tput setaf 4)$(tput bold)${reference}$(tput sgr0) exists "
if [ -e $reference ]; then
	success
else
	failure_tick "Could not find the reference genome file. "
fi
printf "Testing that the reads file $(tput setaf 4)$(tput bold)${reads}$(tput sgr0) exists "
if [ -e $reads ]; then
	success
else
	failure_tick "Could not find the reads file. "
fi

## Run evaluation of all mappers...
if [ -e $report_file ]; then
	mv $report_file{,.bak}
fi
if [ -e $log_file ]; then
	rm $log_file
fi
printf "%-${mapper_field_length}s %10s\n" mapper time > $report_file

touch $log_file

for mapper in $mappers; do
	echo
	echo "Evaluating $(tput setaf 4)$(tput bold)${mapper}$(tput sgr0) : "

	### Preprocessing --------------------------------------------------------------------------------
	if [ -x ${mapper}.preprocess ]; then
		printf "   • Preprocessing $(tput setaf 4)$(tput bold)evaluation/${mapper}.preprocess$(tput sgr0) "
		./${mapper}.preprocess  ${reference} >> $log_file 2>&1
		if [ $? -eq 0 ]; then
    		success
		else
    		failure_tick "Preprocessing failed. Check $(tput setaf 4)$(tput bold)`basename ${log_file}`$(tput sgr0) for further information."
		fi

	else
		if [ -e ${mapper}.preprocess ]; then
			failure "The file $(tput setaf 4)$(tput bold)evaluation/${mapper}.preprocess$(tput sgr0) exists but is not executable!"
		fi
	fi

	### Measuring running time -----------------------------------------------------------------------
	if [ -x ${mapper}.run ]; then
		printf "   • Read-mapping using $(tput setaf 4)$(tput bold)evaluation/${mapper}.run$(tput sgr0) "
		mapper_cmd=./${mapper}.run
	elif [ -e ${mapper}.run ]; then
			failure "The file $(tput setaf 4)$(tput bold)evaluation/${mapper}.run$(tput sgr0) exists but is not executable!"
	else
		# if we don't have a run script we call the read-mapper directly
		printf "   • Read-mapping using $(tput setaf 4)$(tput bold)mappers_src/${mapper}$(tput sgr0) "
		mapper_cmd=../mappers_src/${mapper}
	fi
	for ((i = 0; i < $N; i++)); do
		walltime=`command time -p ${mapper_cmd} -d $d ${reference} ${reads} 2>&1 1> /dev/null | awk '/^real/ { print $2 }'`
		printf "%-${mapper_field_length}s %10s\n" ${mapper} ${walltime} >> $report_file
	done
	success

	printf "   • DONE "
	success
done

header=`head -n 1 $report_file`
major_rule=`printf "%${#header}s" |tr " " "="`
minor_rule=`printf "%${#header}s" |tr " " "-"`

echo
echo "$(tput setaf 2)$(tput bold)Results of the evaluation:$(tput sgr0)"
echo "$(tput setaf 4)$(tput bold)${major_rule}$(tput sgr0)"
echo "$(tput setaf 4)$(tput bold)${header}$(tput sgr0)"
echo "$(tput setaf 4)$(tput bold)${minor_rule}$(tput sgr0)"
tail -n +2 $report_file
echo "$(tput setaf 4)$(tput bold)${minor_rule}$(tput sgr0)"
echo

## If R is installed, we make a plot of the results.
if type Rscript > /dev/null; then
	echo "$(tput setaf 4)$(tput bold)Analysing report with R$(tput sgr0)"
	Rscript analyse-report.R $report_file
	echo -n "Results plotted in $(tput setaf 2)$(tput bold)`basename $report_file`.png$(tput sgr0) "
	success
else
	printf "You do not have R installed, so the report plot is not updated. $(tput setaf 1)$(tput bold)✘$(tput sgr0)\n"
fi
echo
