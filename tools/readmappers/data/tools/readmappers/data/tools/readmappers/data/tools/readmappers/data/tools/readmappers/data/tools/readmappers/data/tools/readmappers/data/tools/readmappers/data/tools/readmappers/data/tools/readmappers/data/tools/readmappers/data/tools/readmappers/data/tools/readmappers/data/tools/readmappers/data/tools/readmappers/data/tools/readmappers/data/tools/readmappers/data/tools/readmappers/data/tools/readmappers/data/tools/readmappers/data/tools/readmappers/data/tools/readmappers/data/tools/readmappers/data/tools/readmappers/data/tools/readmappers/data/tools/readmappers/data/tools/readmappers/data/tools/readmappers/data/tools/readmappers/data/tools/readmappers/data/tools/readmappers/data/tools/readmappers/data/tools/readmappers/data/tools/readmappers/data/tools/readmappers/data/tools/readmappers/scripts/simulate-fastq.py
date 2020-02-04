"""
Program for simulating fastq files.
"""

import random
import argparse


DNA = 'ACGT'


def parser_fasta(f):
	record = {}
	lines = [line.strip() for line in f.readlines()]
	header = lines[0][1:]
	seqs = []
	for line in lines[1:]:
		if line[0] == '>':
			# beginning new record...
			record[header] = ''.join(seqs)
			header = line.split()[1]
			seqs = []
		else:
			seqs.append(line)
	record[header] = ''.join(seqs)
	return record


def sample_sequence(seqs, record, m, d):
	seq_id = random.choice(seqs)
	seq = record[seq_id]
	idx = random.randrange(len(seq) - m)
	sample = seq[idx:(idx+m)]
	return (seq_id, idx + 1, sample, mutate(sample, d)) # + 1 for 1-indexing


def mutate(seq, d):
	seq = list(seq)
	for _ in range(d):
		mutation = random.randrange(3)
		position = random.randrange(len(seq))
		if mutation == 0:
			seq[position] = random.choice(DNA)
		elif mutation == 1:
			del seq[position]
		else:
			seq = seq[:position] + [random.choice(DNA)] + seq[position:]
	return ''.join(seq)


if __name__ == '__main__':
	parser = argparse.ArgumentParser(prog='simulate-fastq', usage='%(prog)s [options] reference',
									 description="Simulate random sequences and output them in FASTA format.")
	parser.add_argument("reference", nargs=1, type=argparse.FileType('r'), help="Reference genome to sample from.")
	parser.add_argument('-n', nargs='?', type=int, help='Number of sequences, default 10', default=10)
	parser.add_argument('-m', nargs='?', type=int, help='The length of the sequences, default 100', default=100)
	parser.add_argument('-d', nargs='?', type=int, help='Maximum edit distance from reference, default 0', default=0)
	parser.add_argument('-l', "--log", nargs='?', type=argparse.FileType('w'), help="Log file to write samples and sample positions to.")

	args = parser.parse_args()

	ref = parser_fasta(args.reference[0])

	seqs = list(ref.keys())

	for i in range(args.n):
		seq_id, idx, sample, mutated_sample = sample_sequence(seqs, ref, args.m, args.d)
		if args.log is not None:
			print('\t'.join([seq_id, str(idx), sample, mutated_sample]), file=args.log)
		print("@read{}".format(i))
		print(mutated_sample)
		print("+")
		print("~" * len(mutated_sample))
