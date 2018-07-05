# -*- coding: utf-8 -*-
import sys
import re
import os
import csv

if len(sys.argv) < 2:
	print ("usage: " + sys.argv[0] + " <logfile name>")
	sys.exit(1)

infile = sys.argv[1]
outfile = infile + '.csv'

# ファイル名に対する正規表現: 例 c:\hogehoge\hoge.c
regLilePath  = r'(?P<filePath>[a-zA-Z]:([^(]+))'

# 行番号に対する正規表現: 例 (100)
regLineNumer = r'\((?P<lineNumber>\d+)\)'

# エラーコードに対する正規表現: 例 warning C4267
regError     = r'\s*(?P<type>\w+)\s+(?P<code>\w+)\s*'

# エラーメッセージ: 例 'argument': conversion from 'size_t' to 'int', possible loss of data [C:\projects\sakura\sakura\sakura.vcxproj]
regMessage   = r'(?P<message>.+)$'

# 解析対象の正規表現
# 例
#   c:\projects\sakura\sakura_core\basis\cmystring.h(39): warning C4267: 'argument': conversion from 'size_t' to 'int', possible loss of data [C:\projects\sakura\sakura\sakura.vcxproj]
regEx        = regLilePath + regLineNumer + r':' + regError + r':' + regMessage

# 型変換に対する警告メッセージ部分に対する正規表現: 例 'size_t' to 'int'
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

# ソート対象のハッシュ配列で 'key' というキーを元にソートする。
from operator import itemgetter
data = sorted(data, key=itemgetter('key'))

# 解析結果を CSV ファイルに出力する
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
