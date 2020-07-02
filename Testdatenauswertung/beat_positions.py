#!/usr/bin/env python
import argparse
from   common import *
import csv
import logging
import matplotlib.pyplot as plt
import os.path
import seaborn as sb
import statistics


EXCEPTION_SYSTEMS = {'.2001_BeatThis'}


def main(args: argparse.Namespace):
	sb.set()  # makes SB-stuff look pretty

	tr_file_groups = group_by_system(args.test_results)
	for ex in EXCEPTION_SYSTEMS:
		if ex in tr_file_groups:
			del tr_file_groups[ex]

	fig, axes = plt.subplots(
		3, len(tr_file_groups),
		figsize=(6 * len(tr_file_groups), 3 * 6)
	)

	for s, (system, tr_fnames) in enumerate(tr_file_groups.items()):
		all_pairs = []
		streak_lengths = []
		i_cnt = {0: 0, 1: 0, 2: 0}

		n_correct_beats = 0
		n_predicted_beats = 0

		for tr_fname in tr_fnames:
			gt_fname = find_matching_gt_file(tr_fname, args.ground_truth)
			gt_beats = read_gt_beats(gt_fname)
			tr_beats = read_tr_beats(tr_fname)

			best_steak = []
			best_streak_pairs = []
			best_streak_ij = (None, None)
			best_alt_gt_beats = []

			for i in range(3):
				if i == 0: alt_gt_beats = half_tempo(gt_beats)
				elif i == 1: alt_gt_beats = gt_beats[:]
				else: alt_gt_beats = double_tempo(gt_beats)

				for j in range(2):
					if j == 1: alt_gt_beats = pi_phase(alt_gt_beats)

					pairs = pair2(alt_gt_beats, tr_beats)
					streak = longest_streak(pairs, 0.35)

					if len(streak) > len(best_steak):
						best_steak = streak
						best_streak_pairs = pairs
						best_streak_ij = (i, j)
						best_alt_gt_beats = alt_gt_beats

			all_pairs.extend(best_streak_pairs)
			streak_lengths.append(len(best_steak))
			i_cnt[best_streak_ij[0]] += 1
			n_correct_beats += len(best_alt_gt_beats)
			n_predicted_beats += len(tr_beats)

		errors = [e for g, t, e in all_pairs if g is not None and t is not None]
		print(
			f'{system}\n'
			f'      # total paired beats: {len(errors):7}\n'
			f'    # total unpaired beats: {len(all_pairs) - len(errors):7}\n'
			f'           # correct beats: {n_correct_beats:7}\n'
			f'  # unpaired correct beats: {sum(1 for _, t, _ in all_pairs if t is None):7}\n'
			f'         # predicted beats: {n_predicted_beats:7}\n'
			f'# unpaired predicted beats: {sum(1 for g, _, _ in all_pairs if g is None):7}'
		)

		sb.distplot(
			errors, ax=axes[0, s],
			hist=True, kde=False,
			bins=[i / 20.0 for i in range(21)]
		)
		axes[0, s].set_xlim(0, 1)
		axes[0, s].set_xlabel(f'Normalisierter Fehler')
		axes[0, s].set_ylim(0, 1200)
		axes[0, s].set_title(SYSTEM_TO_TITLE[system], fontsize=16)

		sb.distplot(
			streak_lengths, ax=axes[1, s],
			hist=False, kde=True,
			kde_kws={'cumulative': True}
		)
		axes[1, s].set_xlim(0, 90)
		axes[1, s].invert_yaxis()
		axes[1, s].set_yticklabels(['100', '80', '60', '40', '20', '0'])
		axes[1, s].set_ylabel('Anzahl der Lieder (prozentual)')
		percs = percentiles(streak_lengths, [0.0, 0.2, 0.5, 0.8, 1.0])
		axes[1, s].set_xlabel(
			f'längste korrekte Beatfolge (1 - CDF)\n'
			f'min: {percs[0]},   '
			f'80%: {percs[1]},   '
			f'50%: {percs[2]},   '
			f'20%: {percs[3]},   '
			f'max: {percs[4]}'
		)

		sb.barplot(
			ax=axes[2, s],
			x=['halb', 'korrekt', 'doppelt'],
			y=[v for k, v in sorted(i_cnt.items())]
		)
		axes[2, s].set_ylim(0, 150)

	fig.suptitle('', fontsize=30)
	fig.savefig(
		args.output,
		dpi=args.dpi,
		pad_inches=0
	)
	logging.info(f'Created PNG file {args.output}')


def assert_args(args: argparse.Namespace):
	for fname in args.ground_truth:
		assert os.path.isfile(fname),\
			f"Can't file ground truth file {fname}"

	for fname in args.test_results:
		assert os.path.isfile(fname), \
			f"Can't file test result file {fname}"

	assert args.dpi > 0, 'DPI must be a positive number'


def double_tempo(beats: List[float]) -> List[float]:
	result = []

	for last_beat, next_beat in zip(beats, beats[1:]):
		result.append(last_beat)
		result.append((last_beat + next_beat) / 2)

	if len(beats) > 0:
		result.append(beats[-1])

	return result


def half_tempo(beats: List[float]) -> List[float]:
	return [beat for i, beat in enumerate(beats) if i % 2 == 0]


def longest_streak(pairs: List[Tuple[float, float, float]], threshold: float) \
		-> List[Tuple[float, float, float]]:
	longest = []
	current = []

	for gt_beat, tr_beat, error in pairs:
		if gt_beat is None or tr_beat is None or error >= threshold:
			if len(current) > len(longest):
				longest = current
			current = []

		else:
			current.append((gt_beat, tr_beat, error))

	return longest


def pair2(gt_beats: List[float], tr_beats: List[float]) \
	-> List[Tuple[float, float, float]]:

	gt_beats.sort()
	pairs = []

	# unpaired tr_beats that lie outside the gt_beat range
	for tr_beat in tr_beats:
		if tr_beat < gt_beats[0] or tr_beat > gt_beats[-1]:
			pairs.append((None, tr_beat, 1.0))

	# for each gt_beat, find all nearby tr_beats and pick the closest one
	for prv, cur, nxt in zip([None] + gt_beats, gt_beats, gt_beats[1:] + [None]):
		min = (cur + prv) / 2 if prv is not None else cur
		max = (cur + nxt) / 2 if nxt is not None else cur

		def error(tr_beat: float) -> float:
			if tr_beat < cur:
				return (tr_beat - cur) / ((prv - cur) / 2)
			elif tr_beat == cur:
				return 0.0
			else:  # tr_beat > cur:
				return (tr_beat - cur) / ((nxt - cur) / 2)

		tr_beat_candidates = [
			(error(tr_beat), tr_beat)
			for tr_beat in tr_beats
			if (min <= tr_beat < max) or (tr_beat == max == cur)
		]
		tr_beat_candidates.sort()

		# unpaired gt_beat
		if len(tr_beat_candidates) == 0:
			pairs.append((cur, None, 1.0))

		else:
			# actual pair
			err, tr_beat = tr_beat_candidates[0]
			pairs.append((cur, tr_beat, err))

			# unpaired tr_beats, which are unpaired,
			# because the alg. made too many predictions
			for _, tr_beat in tr_beat_candidates[1:]:
				pairs.append((None, tr_beat, 1.0))

	pairs.sort(key=lambda pair: (pair[1] if pair[0] is None else pair[0], pair[1]))

	return pairs


def pair(gt_beats: List[float], tr_beats: List[float]) \
		-> List[Tuple[float, float, float]]:
	pairs = []
	gt_beat_intervals = zip([None] + gt_beats, gt_beats + [None])
	last_gt_beat, next_gt_beat = next(gt_beat_intervals)

	for tr_beat in tr_beats:
		while next_gt_beat is not None and next_gt_beat <= tr_beat:
			last_gt_beat, next_gt_beat = next(gt_beat_intervals, (None, None))

		# next_gt_beat is None or tr_beat < next_gt_beat

		if last_gt_beat is None:
			continue  # unmatched (tr_beat is before the first gt_beat)

		if next_gt_beat is None:
			if tr_beat == last_gt_beat:
				pairs.append((last_gt_beat, tr_beat, 0))
				continue

			else:  # last_gt_beat < tr_beat
				continue  # unmatched (tr_beat is after the last gt_beat)

		beat_period = next_gt_beat - last_gt_beat

		if tr_beat < last_gt_beat:
			pass  # unpaired

		elif tr_beat < last_gt_beat + 0.5 * beat_period:
			error = (tr_beat - last_gt_beat) / (beat_period / 2)
			pairs.append((last_gt_beat, tr_beat, error))

		else:
			error = (next_gt_beat - tr_beat) / (beat_period / 2)
			pairs.append((next_gt_beat, tr_beat, error))

	for beat in set.difference(set(gt_beats), {pair[0] for pair in pairs}):
		pairs.append((beat, None, 1.0))

	for beat in set.difference(set(tr_beats), {pair[1] for pair in pairs}):
		pairs.append((None, beat, 1.0))

	pairs.sort(key=lambda pair: (pair[1] if pair[0] is None else pair[0], pair[1]))

	return pairs


def percentiles(data: List[float], percs: List[float]) -> List[float]:
	data = sorted(data)
	data_len = len(data)

	result = []

	for p in percs:
		result.append(data[min(data_len, max(0, round(p * (data_len - 1))))])

	return result


def pi_phase(beats: List[float]) -> List[float]:
	result = []

	for last_beat, next_beat in zip(beats, beats[1:]):
		result.append((last_beat + next_beat) / 2)

	return result


def read_gt_beats(gt_fname: str) -> List[float]:
	with open(gt_fname, mode='r') as file:
		return sorted(
			float(line.strip())
			for line in file.readlines()
			if len(line.strip()) > 0
		)


def read_tr_beats(tr_fname: str) -> List[float]:
	with open(tr_fname, mode='r') as file:
		reader = csv.reader(file, delimiter=',')
		header = next(reader)
		idx_ts_sec = header.index('timestamp_sec')
		idx_beat = header.index('beat')
		return sorted(
			float(line[idx_ts_sec])
			for line in reader
			if line[0][0] != '#' and line[idx_beat].strip().lower() == 'yes'
		)


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
