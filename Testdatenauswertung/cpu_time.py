#!/usr/bin/env python
import argparse
from common import *
import csv
import logging
import os.path


def main(args: argparse.Namespace):
	tr_file_groups = group_by_system(args.test_results)

	for system, tr_fnames in tr_file_groups.items():
		cpu_time, total_time, ratio = cpu_time_per_sec(tr_fnames)
		print(f'{system}: CPU Time: {cpu_time:.5g}, Total Time: {total_time:.5g}, Ratio: {ratio:.5g}')


def cpu_time_per_sec(tr_fnames: List[str]) -> Tuple[float, float, float]:
	total_time = 0.0
	cpu_time = 0.0

	for tr_fname in tr_fnames:
		with open(tr_fname, mode='r') as file:
			reader = csv.reader(file, delimiter=',')

			for line in reader:
				if line[0][0] == '#':
					parts = line[0].split('=')

					if parts[0] == '# total_time':
						total_time += float(parts[1].strip())

					elif parts[0] == '# total_cpu_time':
						cpu_time += float(parts[1].strip())

	return cpu_time, total_time, cpu_time / total_time


def assert_args(args: argparse.Namespace):
	for fname in args.test_results:
		assert os.path.isfile(fname), \
			f"Can't file test result file {fname}"


def parse_args() -> argparse.Namespace:
	parser = argparse.ArgumentParser(
		description=''
	)

	parser.add_argument(
		'-t', '--test-results',
		nargs='+',
		required=True,
		help='list of test result files, produced by the ./test program'
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
