#!/usr/bin/env python
import argparse
from   common import *
import csv
import logging
import matplotlib.pyplot as plt
import os.path
import seaborn as sb
import statistics


def main(args: argparse.Namespace):
	sb.set()  # makes SB-stuff look pretty

	tr_file_groups = group_by_system(args.test_results)
	fig, axes = plt.subplots(
		2, len(tr_file_groups),
		figsize=(6 * len(tr_file_groups), 2 * 6)
	)

	for i, (system, tr_fnames) in enumerate(tr_file_groups.items()):
		errors = []
		error_triplet_idx_cnt = {0: 0, 1: 0, 2: 0}

		for tr_fname in tr_fnames:
			gt_fname = find_matching_gt_file(tr_fname, args.ground_truth)

			if gt_fname is None:
				logging.warning(f"Can't find mathing ground truth file for {tr_fname}")
				continue

			song_errors, triplet_idx = analyse_tempi(gt_fname, tr_fname)
			errors.extend(map(lambda et: et[triplet_idx], song_errors))
			error_triplet_idx_cnt[triplet_idx] += 1

		sb.distplot(
			errors, ax=axes[0, i],
			hist=True, kde=False,
			bins=[5 * i - 100 for i in range(41)]
		)
		axes[0, i].set_xlim(-100, 100)
		axes[0, i].set_ylim(0, 1200)
		axes[0, i].set_xlabel(
			f'Tempo Estimation Error (BPM)\n'
			f'mean = {statistics.mean(errors):.4g},   '
			f'σ = {statistics.stdev(errors):.4g}')
		axes[0, i].set_title(SYSTEM_TO_TITLE[system], fontsize=16)

		sb.barplot(
			ax=axes[1, i],
			x=['half', 'correct', 'double'],
			y=[v for k, v in sorted(error_triplet_idx_cnt.items())]
		)
		axes[1, i].set_ylim(0, 200)

	fig.suptitle('Tempo Estimation Errors', fontsize=30)
	fig.savefig(
		args.output,
		dpi=args.dpi,
		pad_inches=0
	)
	logging.info(f'Created PNG file {args.output}')


def analyse_tempi(gt_fname: str, tr_fname: str) \
		-> Tuple[List[Tuple[float, float, float]], int]:
	"""
	Returns a list of error triplets and an index from 0 to 2, inclusive.
	One error triplet per tempo prediction that falls between the first and last
		beat.
	Error Triplets: (
			error to half of the correct tempo
			error to the correct tempo,
			error to double the correct tempo
		)
	Everything in seconds.
	The returned index indicates the triplet index with the lowest square error.
	"""

	with open(gt_fname, mode='r') as file:
		gt_beat_positions = [
			float(line.strip())
			for line in file.readlines()
			if len(line.strip()) > 0
		]

	with open(tr_fname, mode='r') as file:
		reader = csv.reader(file, delimiter=',')
		header = next(reader)
		idx_ts_sec = header.index('timestamp_sec')
		idx_tempo = header.index('tempo_bpm')
		tr_tempi = [
			(float(line[idx_ts_sec]), float(line[idx_tempo]))
			for line in reader
			if line[0][0] != '#' and len(line[idx_tempo]) > 0
		]

	# the files should already be sorted, but just to make sure ...
	gt_beat_positions.sort()
	tr_tempi.sort()

	errors = []
	last_tempo = None

	# for each interval between two consecutive beats
	for curr_beat, next_beat in zip(gt_beat_positions, gt_beat_positions[1:]):
		# correct tempo is determined by the size of the current interval
		correct_tempo = 60.0 / (next_beat - curr_beat)

		# list of all test result tempo estimations in the current interval
		tempi = []

		while len(tr_tempi) > 0 and tr_tempi[0][0] < next_beat:
			tr_tempo = tr_tempi.pop(0)[1]
			tempi.append(tr_tempo)
			last_tempo = tr_tempo

		# If there is no new tempo estimation in the current interval, take the
		#     last one.
		# It there is no last one, skip this interval.
		if len(tempi) == 0:
			if last_tempo is None:
				continue

			tempi.append(last_tempo)

		avg_tempo = sum(tempi) / len(tempi)

		errors.append((
			avg_tempo - correct_tempo / 2,
			avg_tempo - correct_tempo,
			avg_tempo - correct_tempo * 2
		))

	summed_error_squares = [sum(e[i] ** 2 for e in errors) for i in range(3)]

	return errors, summed_error_squares.index(min(summed_error_squares))


def assert_args(args: argparse.Namespace):
	for fname in args.ground_truth:
		assert os.path.isfile(fname),\
			f"Can't file ground truth file {fname}"

	for fname in args.test_results:
		assert os.path.isfile(fname), \
			f"Can't file test result file {fname}"

	assert args.dpi > 0, 'DPI must be a positive number'


def parse_args() -> argparse.Namespace:
	parser = argparse.ArgumentParser(
		description=''
	)

	parser.add_argument(
		'-g', '--ground-truth',
		nargs='+',
		required=True,
		help='list of SMC MIREX annotation files'
	)

	parser.add_argument(
		'-t', '--test-results',
		nargs='+',
		required=True,
		help='list of test result files, produced by the ./test program'
	)

	parser.add_argument(
		'-o', '--output',
		required=True,
		help='file name of the output PNG-file'
	)

	parser.add_argument(
		'-d', '--dpi',
		required=False,
		default='100',
		type=int,
		help='dots per inch of the output file (the plot is 18x12 inches) '
			'defaults to 100'
	)

	parser.add_argument(
		'--log',
		default='INFO',
		help=
			'Logging level. Case insensitive. Possible value are: '
			'DEBUG, INFO, WARNING, ERROR, CRITICAL.'
	)

	args = parser.parse_args()

	assert args.log.upper() in \
		{'DEBUG', 'INFO', 'WARNING', 'ERROR', 'CRITICAL'}, \
		'The log level (--log argument) must be one of the following: ' \
		'DEBUG, INFO, WARNING, ERROR, CRITICAL.'
	args.log = getattr(logging, args.log.upper())

	return args


if __name__ == '__main__':
	_args = parse_args()

	logging.basicConfig(
		datefmt='%Y-%m-%d %I:%M:%S %p',
		format='[%(asctime)s][%(levelname)s]: %(message)s',
		level=_args.log
	)

	assert_args(_args)

	main(_args)
