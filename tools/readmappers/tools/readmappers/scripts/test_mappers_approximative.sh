#!/bin/bash

## Modify here to add or remove mappers or change options
## =============================================================

# Set this to make sure the mappers are in your path
export PATH=../mappers_src:$PATH

# The mapper we use as the goal to hit
ref_mapper=ac_readmapper

# list of read-mappers to evaluate
mappers="bw_readmapper"

# file name for report
report_file=../test-report.txt
log_file=../test.log

# max edit distance to explore
d=1

# Reference genome
reference=../data/gorGor3-small-noN.fa

# Reads
reads=../data/sim-reads-d2-tiny.fq

## =============================================================

## It shouldn't be necessary to touch any of the code below.
## If it is, let me know, so I can adapt it such that it will
## not be necessary in the future.

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
	rm $report_file
fi
if [ -e $log_file ]; then
	rm $log_file
fi
touch $report_file
touch $log_file

## Reference
echo
echo "Building reference SAM file using $(tput setaf 4)$(tput bold)${ref_mapper}$(tput sgr0) : "
if [ -e ${ref_mapper}-approx.sam ] && [ ${ref_mapper}-approx.sam -nt ../mappers_src/${ref_mapper} ]; then
	printf "   • SAM file $(tput setaf 4)$(tput bold)evaluation/${ref_mapper}-approx.sam$(tput sgr0) already exists "
	success
else
	### Preprocessing --------------------------------------------------------------------------------
	if [ -x ${ref_mapper}.preprocess ]; then
		printf "   • Preprocessing $(tput setaf 4)$(tput bold)evaluation/${ref_mapper}.preprocess$(tput sgr0) "
		./${ref_mapper}.preprocess ${reference} >> $log_file 2>&1
		if [ $? -eq 0 ]; then
   			success
		else
   			failure_tick "Preprocessing failed. Check $(tput setaf 4)$(tput bold)`basename ${log_file}`$(tput sgr0) for further information."
				cat ${log_file}
				exit 1
		fi
	else
		if [ -e ${ref_mapper}.preprocess ]; then
			failure "The file $(tput setaf 4)$(tput bold)evaluation/${ref_mapper}.preprocess$(tput sgr0) exists but is not executable!"
			exit 1
		fi
	fi
	### Constructing reference SAM --------------------------------------------------------------------
	if [ -x ${ref_mapper}.run ]; then
		printf "   • Read-mapping using $(tput setaf 4)$(tput bold)evaluation/${ref_mapper}.run$(tput sgr0) "
		./${ref_mapper}.run -d $d ${reference} ${reads} 2> $log_file | sort > ${ref_mapper}-approx.sam
		if [ $? -eq 0 ]; then
   			success
		else
   			failure_tick "Read-mapping failed. Check $(tput setaf 4)$(tput bold)`basename ${log_file}`$(tput sgr0) for further information."
				cat ${log_file}
				exit 1
		fi
	elif [ -e ${mapper}.run ]; then
			failure "The file $(tput setaf 4)$(tput bold)evaluation/${ref_mapper}.run$(tput sgr0) exists but is not executable!"
			exit 1
	else
		# if we don't have a run script we call the read-mapper directly
		printf "   • Read-mapping using $(tput setaf 4)$(tput bold)mappers_src/${ref_mapper}$(tput sgr0) "
		${ref_mapper} -d $d ${reference} ${reads} 2> $log_file | sort > ${ref_mapper}-approx.sam
		if [ $? -eq 0 ]; then
			success
		else
   			failure_tick "Read-mapping failed. Check $(tput setaf 4)$(tput bold)`basename ${log_file}`$(tput sgr0) for further information."
				cat ${log_file}
				exit 1
		fi
	fi
fi
printf "   • DONE "
success

for mapper in $mappers; do
	echo "Building SAM file using $(tput setaf 4)$(tput bold)${mapper}$(tput sgr0) : "
	if [ -e ${mapper}-approx.sam ] && [ ${mapper}-approx.sam -nt ../mappers_src/${mapper} ]; then
		printf "   • SAM file $(tput setaf 4)$(tput bold)evaluation/${mapper}-approx.sam$(tput sgr0) already exists "
		success
	else
		### Preprocessing --------------------------------------------------------------------------------
		if [ -x ${mapper}.preprocess ]; then
			printf "   • Preprocessing $(tput setaf 4)$(tput bold)evaluation/${mapper}.preprocess$(tput sgr0) "
			./${mapper}.preprocess ${reference} >> $log_file 2>&1
			if [ $? -eq 0 ]; then
   				success
			else
   				failure_tick "Preprocessing failed. Check $(tput setaf 4)$(tput bold)`basename ${log_file}`$(tput sgr0) for further information."
					cat ${log_file}
					exit 1
			fi
		else
			if [ -e ${mapper}.preprocess ]; then
				failure "The file $(tput setaf 4)$(tput bold)evaluation/${mapper}.preprocess$(tput sgr0) exists but is not executable!"
				exit 1
			fi
		fi
		### Constructing reference SAM --------------------------------------------------------------------
		if [ -x ${mapper}.run ]; then
			printf "   • Read-mapping using $(tput setaf 4)$(tput bold)evaluation/${mapper}.run$(tput sgr0) "
			./${mapper}.run -d $d ${reference} ${reads}  2> $log_file | sort > ${mapper}-approx.sam
			if [ $? -eq 0 ]; then
   				success
			else
   				failure_tick "Read-mapping failed. Check $(tput setaf 4)$(tput bold)`basename ${log_file}`$(tput sgr0) for further information."
					cat ${log_file}
					exit 1
			fi
		elif [ -e ${mapper}.run ]; then
				failure "The file $(tput setaf 4)$(tput bold)evaluation/${mapper}.run$(tput sgr0) exists but is not executable!"
				exit 1
		else
			# if we don't have a run script we call the read-mapper directly
			printf "   • Read-mapping using $(tput setaf 4)$(tput bold)mappers_src/${mapper}$(tput sgr0) "
			${mapper} -d $d ${reference} ${reads}  2> $log_file | sort > ${mapper}-approx.sam
			if [ $? -eq 0 ]; then
				success
			else
   				failure_tick "Read-mapping failed. Check $(tput setaf 4)$(tput bold)`basename ${log_file}`$(tput sgr0) for further information."
					cat ${log_file}
					exit 1
			fi
		fi
	fi
	## Compare to reference results
	printf "   • Comparing $(tput setaf 4)$(tput bold)${mapper}$(tput sgr0) to $(tput setaf 4)$(tput bold)${ref_mapper}$(tput sgr0) "
	if ( cmp ${ref_mapper}-approx.sam ${mapper}-approx.sam ); then
		success
	else
		failure "$(tput bold)${mapper}$(tput sgr0) differs from $(tput setaf 4)$(tput bold)${ref_mapper}$(tput sgr0)"
		exit 1
	fi
	printf "   • DONE "
	success
done

echo -n "All tests passed! "
success
