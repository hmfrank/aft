import os.path
from   typing import *


SYSTEM_TO_TITLE = {
	'.2001_BeatThis': '2001 Cheng, Nazer, Uppuluri, Verret',
	'.2009_DaPlSt': '2009 Stark, Davies, Plumbley',
	'.2011_PlRoSt': '2011 Robertson, Stark, Plumbley'
}


def find_matching_gt_file(tr_fname: str, gt_fnames: List[str]) \
		-> Optional[str]:
	"""
	Returns the first file in `gt_fnames` that shares the prefix with `tr_fname`.
	See the code for more details.
	If no match if found, None is returned.
	"""

	tr_basename = os.path.basename(tr_fname)
	tr_prefix = tr_basename[:tr_basename.find('.')]

	for gt_fname in gt_fnames:
		gt_basename = os.path.basename(gt_fname)
		gt_prefix = gt_basename[:len(tr_prefix)]

		if tr_prefix == gt_prefix:
			return gt_fname


def group_by_system(tr_fnames: List[str]) -> Dict[str, List[str]]:
	"""
	Groups the test result files by the beat tracking system used.
	The resulting dictionary maps the name of the beat tracking system to a list
		of test result files.
	"""

	result = dict()

	for fname in tr_fnames:
		group = os.path.splitext(os.path.splitext(fname)[0])[1]

		if group not in result:
			result[group] = []

		result[group].append(fname)

	return result
