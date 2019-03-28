# Simulated reads

I have simulated these fastq files by extracting random strings from a reference and then mutating with a max number of mutations.

File

	reads-n-m-d.fq

has n sequences of length m and with at most d edits. It was simulated as

	python scripts/simulate-fastq.py -n 10 -m 10 -d 1 data/genomes/hg38-1000000.fa
	
I have sampled from the `hg38-1000000.fa` for all the simulations.



