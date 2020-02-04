#!/bin/bash

# Script input
if (( $# != 3)); then
	echo "This script requires three arguments: edit distance, reference genome and reads file"
	exit
fi

edits=$1
reference=$2
reads=$3

# Binaries
exact=./exact_readmapper/exact_readmapper
bwt=./bwt_readmapper/bwt_readmapper

# Reference SAM result to compare to.
refcmd="./exact_readmapper/exact_readmapper -d $edits $reference $reads"

# Preprocessing commands.
declare -a names=(
	Exact
    Aho-Corasick
	BWT
)
declare -a preprocess_cmds=(
	true
    true
	"$bwt -p $reference"
)
declare -a map_cmds=(
	"$exact -d $edits $reference $reads"
    "$ac -d $edits $reference $reads"
	"$bwt -d $edits $reference $reads"
)




## IO code
function success() {
	printf "$(tput setaf 2)$(tput bold)✔$(tput sgr0)\n"
}
function failure() {
	err_msg=$1
	echo "$(tput setaf 1)$(tput bold)↪$(tput sgr0) " "$err_msg"
	echo
}
function failure_tick() {
	err_msg=$1
	printf "$(tput setaf 1)$(tput bold)✘$(tput sgr0)\n\t"
	failure "${err_msg}"
}
function header() {
	printf "$(tput bold)$1$(tput sgr0)\n"
}

## Test that the data is there
printf "Testing that the reference $(tput setaf 4)$(tput bold)${reference}$(tput sgr0) exists "
if [ -e "$reference" ]; then
	success
else
	failure_tick "Could not find the reference genome file. "
	exit 1
fi
printf "Testing that the reads file $(tput setaf 4)$(tput bold)${reads}$(tput sgr0) exists "
if [ -e "$reads" ]; then
	success
else
	failure_tick "Could not find the reads file. "
	exit 1
fi
echo

# Prepare dir for logs
[[ -d logs ]] || mkdir logs

### PREPROCESSING ######################################################
header "Preprocessing reference genomes"
for i in ${!names[@]}; do
	cmd=${preprocess_cmds[$i]}
	name=${names[$i]}
	logfile="logs/$name.preprocess.log"

	printf "   • Preprocessing: $(tput setaf 4)$(tput bold)${name}$(tput sgr0) "
	${cmd} &> "$logfile"
	if [ $? -eq 0 ]; then
		success
	else
		failure_tick "Preprocessing failed. Check $(tput setaf 4)$(tput bold)${logfile}$(tput sgr0) for further information."
		exit
	fi
done
echo

### BUILDING REFERENCE SAM ######################################################
refsam=logs/reference.sam
logfile=logs/reference.log
header "Building reference SAM file"
printf "   • Writing reference to $(tput setaf 4)$(tput bold)${refsam}$(tput sgr0) "
"$refcmd" 2> "$logfile" | sort > "$refsam"
if [ $? -eq 0 ]; then
	success
else
	failure_tick " Check $(tput setaf 4)$(tput bold)${logfile}$(tput sgr0) for further information."
	exit 1
fi
echo

### READMAPPING ######################################################
header "Running mappers"
for i in ${!names[@]}; do
	cmd=${map_cmds[$i]}
	name=${names[$i]}
	logfile=logs/$name.map.log
	samfile=logs/$name.sam

	printf "   • Mapping: $(tput setaf 4)$(tput bold)${name}$(tput sgr0) "
	"$cmd"  2> "$logfile" | sort > "$samfile"
	if [ $? -eq 0 ]; then
		success
	else
		failure_tick "Mapping failed. Check $(tput setaf 4)$(tput bold)${logfile}$(tput sgr0) for further information."
	fi
done
echo

### COMPARING ######################################################
header "Comparing results"
for i in ${!names[@]}; do
	name=${names[$i]}
	samfile="logs/$name.sam"

	printf "   • $(tput setaf 4)$(tput bold)${name}$(tput sgr0): "
	printf "comparing $(tput setaf 4)${samfile}$(tput sgr0) to $(tput setaf 4)${refsam}$(tput sgr0) "
	cmp "$refsam" "$samfile" &> /dev/null
	if [ $? -eq 0 ]; then
		success
	else
		failure_tick "Comparison failed"
	fi
done
echo

echo -n "All tests passed! "
success
