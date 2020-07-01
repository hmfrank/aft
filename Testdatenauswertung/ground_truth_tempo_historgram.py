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

	tempi = sum((find_all_tempi(gt_fname) for gt_fname in args.ground_truth), [])

	fig, axes = plt.subplots(1, 1, figsize=(7.5, 6))
	sb.distplot(
		tempi, ax=axes,
		hist=True, kde=False,
		bins=[5 * i for i in range(51)]
	)
	for i in range(1, 51):
		c = 'red'
		if i in range(8, 64):  # [40, 320) BPM
			c = 'orange'
		if i in range(16, 32):  # [80, 160) BPM
			c = 'green'
		axes.get_children()[i].set_color(c)
	axes.set_xlim(0, 250)
	axes.set_ylim(0, 1200)
	axes.set_xlabel(
		f'Tempo (BPM)   '
		f'Ø = {statistics.mean(tempi):.4g},   '
		f'σ = {statistics.stdev(tempi):.4g}')
	axes.set_title('', fontsize=16)

	fig.suptitle('Datensatz Tempoverteilung', fontsize=30)
	fig.savefig(
		args.output,
		dpi=args.dpi,
		pad_inches=0
	)
	logging.info(f'Created PNG file {args.output}')

	logging.info(
		f'Percentage Red: '
		f'{sum(1 for t in tempi if (t < 40) or (t > 320)) / len(tempi) * 100:.2f} %'
	)
	logging.info(
		f'Percentage Yellow: '
		f'{sum(1 for t in tempi if (t >= 40 and t < 80) or (t >= 160 and t < 320)) / len(tempi) * 100:.2f} %'
	)
	logging.info(
		f'Percentage Green: '
		f'{sum(1 for t in tempi if t >= 80 and t < 160) / len(tempi) * 100:.2f} %'
	)


def assert_args(args: argparse.Namespace):
	for fname in args.ground_truth:
		assert os.path.isfile(fname),\
			f"Can't file ground truth file {fname}"

	assert args.dpi > 0, 'DPI must be a positive number'


def find_all_tempi(gt_file: str) -> List[float]:
	with open(gt_file, mode='r') as file:
		beat_positions = [
			float(line.strip())
			for line in file.readlines()
			if len(line.strip()) > 0
		]

	return [
		60.0 / (end - start)
		for start, end in zip(beat_positions, beat_positions[1:])
	]


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
