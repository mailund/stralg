#!/bin/bash

# Script input
if (($# != 5)); then
	echo "This script requires five arguments: the number of repeats of each command,"
	echo "the max edit distance, the reference genome, the reads file, and the output file"
	exit
fi

reps=$1
edits=$2
reference=$3
reads=$4
outfile=$5

# Binaries
exact=./exact_readmapper/exact_readmapper
bwt=./bwt_readmapper/bwt_readmapper

# Preprocessing commands.
declare -a names=(
	#Exact
	BWT
)
declare -a preprocess_cmds=(
	#true
	"$bwt -p $reference"
)
declare -a map_cmds=(
	#"$exact $reference $reads"
	"$bwt $reference $reads"
)

## IO code
function success() {
	printf "$(tput setaf 2)$(tput bold)✔$(tput sgr0)\n"
}
function failure() {
	err_msg=$1
	echo "$(tput setaf 1)$(tput bold)↪$(tput sgr0) " $err_msg
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

# Prepare dir for logs
[[ -d logs ]] || mkdir logs
# Get rid of the old report file if it exists and set headers
echo "Mapper Operation Editdist Time" > $outfile

### PREPROCESSING ######################################################
header "Preprocessing reference genomes"
# BWA doesn't take the same options as the others (are expected to take)
# so I have to make a special case for it
printf "   • Preprocessing: $(tput setaf 4)$(tput bold)BWA$(tput sgr0) "
cmd="bwa index $reference"
log_file=logs/bwa.preprocess.log
for ((n=0;n<$reps;n++)); do
	echo -n . # feed back of progress...
	start=`date +%s`
	$cmd &> $log_file
	if [ ! $? -eq 0 ]; then
		failure_tick "Preprocessing failed. Check $(tput setaf 4)$(tput bold)${log_file}$(tput sgr0) for further information."
		exit
	fi
	end=`date +%s`
	runtime=$((end-start))
	echo BWA Preprocessing NA $runtime >> $outfile
done
success

for i in ${!names[@]};
do
	cmd=${preprocess_cmds[$i]}
	name=${names[$i]}
	log_file=logs/$name.preprocess.log

	printf "   • Preprocessing: $(tput setaf 4)$(tput bold)${name}$(tput sgr0) "
	for ((n=0;n<$reps;n++)); do
		echo -n . # feedback for long runs...
		start=`date +%s`
		$cmd &> $log_file
		if [ ! $? -eq 0 ]; then
			failure_tick "Preprocessing failed. Check $(tput setaf 4)$(tput bold)${log_file}$(tput sgr0) for further information."
		exit
		fi
		end=`date +%s`
		runtime=$((end-start))
		echo $name Preprocessing NA $runtime >> $outfile
	done
	success
done

### READMAPPING ######################################################
header "Running mappers"
# BWA doesn't take the same options as the others (are expected to take)
# so I have to make a special case for it
printf "   • Mapping: $(tput setaf 4)$(tput bold)BWA$(tput sgr0) "
cmd="bwa mem $reference $reads"
log_file=logs/bwa.map.log
for ((n=0;n<$reps;n++)); do
	echo -n . # feedback for long runs...
	start=`date +%s`
	$cmd &> $log_file
	if [ ! $? -eq 0 ]; then
		failure_tick "Mapping failed. Check $(tput setaf 4)$(tput bold)${log_file}$(tput sgr0) for further information."
		exit
	fi
	end=`date +%s`
	runtime=$((end-start))
	echo BWA Mapping NA $runtime >> $outfile
done
success

for i in ${!names[@]}; do
	name=${names[$i]}
	log_file=logs/$name.map.log
	samfile=logs/$name.sam

	printf "   • Mapping: $(tput setaf 4)$(tput bold)${name}$(tput sgr0) "
	for ((j=0;j<=$edits;j++)); do
		cmd="${map_cmds[$i]} -d $j"
		for ((n=0;n<$reps;n++)); do
			echo -n . # feedback for long runs...
			start=`date +%s`
			$cmd &> $log_file
			if [ ! $? -eq 0 ]; then
				failure_tick "Mapping failed. Check $(tput setaf 4)$(tput bold)${log_file}$(tput sgr0) for further information."
			exit
			fi
			end=`date +%s`
			runtime=$((end-start))
			echo $name Mapping $j $runtime >> $outfile
		done
	done
	success
done

