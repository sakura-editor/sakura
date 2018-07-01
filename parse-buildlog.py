import sys
import re
import os
import csv

infile = sys.argv[1]
outfile = infile + '.csv'

regLilePath  = r'(?P<filePath>[a-zA-Z]:([^(]+))'
regLineNumer = r'\((?P<lineNumber>\d+)\)'
regError     = r'\s*(?P<type>\w+)\s+(?P<code>\w+)\s*'
regMessage   = r'(?P<message>.+)$'
regEx        = regLilePath + regLineNumer + r':' + regError + r':' + regMessage
regFromTo    = r"from '(?P<source>[^']+)' to '(?P<dest>[^']+)'"

data = []
with open(infile, "r") as fin:
	print ("open " + infile)
	for line in fin:
		text = line.replace('\n','')
		text = text.replace('\r','')

		match = re.search(regEx, text)
		if match:
			path       = match.group('filePath')
			lineNumber = match.group('lineNumber')
			type       = match.group('type')
			code       = match.group('code')
			message    = match.group('message')

			entry = {}
			entry['type'] = type
			entry['code'] = code

			match2 = re.search(regFromTo, text)
			if match2:
				source  = match2.group('source')
				dest    = match2.group('dest')
				entry['source'] = source
				entry['dest']   = dest
			else:
				entry['source'] = r''
				entry['dest']   = r''

			entry['path']       = path
			entry['lineNumber'] = lineNumber
			entry['message']    = message

			keys = [
				'type',
				'code',
				'source',
				'dest',
				'path',
				'lineNumber',
				'message',
			]

			temp = []
			for key in keys:
				temp.append(entry[key])
			entry['key'] = ' '.join(temp)
			data.append(entry)

from operator import itemgetter
data = sorted(data, key=itemgetter('key'))

with open(outfile, "w") as fout:
	fieldnames = [
		'type',
		'code',
		'source',
		'dest',
		'path',
		'lineNumber',
		'message',
	]
	writer = csv.writer(fout, lineterminator='\n')
	writer.writerow(fieldnames)

	for entry in data:
		temp = []
		for key in keys:
			temp.append(entry[key])
		writer.writerow(temp)

	print ("wrote " + outfile)
