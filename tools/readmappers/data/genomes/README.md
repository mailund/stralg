# Simulated genomes

These fast files are modified prefixes of the human chromosome one.

File

	hg38-n.fa

was generated with the command

	$ head -n n hg38.fa | python scripts/randomize-N.py > hg38-n.fa
