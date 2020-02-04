import sys
import random
import argparse


DNA = 'ACGT'
LINE_WIDTH = 60

def parser_fasta(f):
	record = {}
	lines = [line.strip() for line in f.readlines()]
	header = lines[0][1:]
	seqs = []
	for line in lines[1:]:
		if line[0] == '>':
			# begining new record...
			record[header] = ''.join(seqs)
			header = line.split()[1]
			seqs = []
		else:
			seqs.append(line.upper())
	record[header] = ''.join(seqs)
	return record

def format_sequence(seq):
	m = len(seq)
	lines = []
	for i in range(0, m, LINE_WIDTH):
		lines.append(''.join(seq[i:(i+LINE_WIDTH)]))
	return '\n'.join(lines)


if __name__ == '__main__':
	parser = argparse.ArgumentParser(prog='randomize-N', usage='%(prog)s reference',
									 description="Replace N with a random A, C, G, or T in a reference genome")
	#parser.add_argument("reference", nargs=1, type=argparse.FileType('r'), help="Reference genome to sample from.")
	parser.add_argument('reference', nargs='?', type=argparse.FileType('r'),  default=sys.stdin)

	args = parser.parse_args()
	ref = parser_fasta(args.reference)

	for seq_id in ref:
		seq = list(ref[seq_id])
		for idx, nuc in enumerate(seq):
			if nuc == 'N':
				seq[idx] = random.choice(DNA)

		print(">", seq_id)
		print(format_sequence(seq))
