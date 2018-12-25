# -*- coding: utf-8 -*-
import os
import sys
import re
import codecs


leftBracket  = 1	# {
rightBracket = 2	# }
blankLine    = 3	# 空行
otherLine    = 4	# それ以外

# 行のタイプを解析する
def parseLine(line):
	match = re.search('^\s*{\s*$', line)
	if match:
		return leftBracket

	match = re.search('^\s*}\s*$', line)
	if match:
		return rightBracket

	match = re.search('^\s*$', line)
	if match:
		return blankLine

	return otherLine

# チェック対象の拡張子リスト
extensions = (
	".cpp",
	".h",
)

# チェック対象の拡張子か判断する
def checkExtension(file):
	base, ext = os.path.splitext(file)
	if ext in extensions:
		return True
	else:
		return False

# 引数で指定したフォルダ以下のすべての対象ファイルを yield で返す
def checkAll(topDir):
	for rootdir, dirs, files in os.walk(topDir):
		for file in files:
			if checkExtension(file):
				full = os.path.join(rootdir, file)
				yield full

# 引数で指定した文字列から改行コードを取り除く
def clipEndOfLine(line):
	return line.rstrip('\r\n')

# 引数で指定したファイルに対して @file コメントをつける
def removeRedundantBlack(file):
	endOfLine   = "\r\n"
	prevLines   = []

	tmp_file = file + ".tmp"
	with codecs.open(file, "r", "utf_8_sig") as fin:
		for line in fin:
			text = clipEndOfLine(line)
			type = parseLine(text)
			lineType = {}
			lineType['text'] = text
			lineType['type'] = type
			
			if prevLines:
				# 最初の行ではないとき
				if prevLines[-1]['type'] == leftBracket:
					if type == blankLine:
						continue

				if prevLines[-1]['type'] == blankLine:
					if type == blankLine:
						continue

				if type == rightBracket:
					# 直前の空行をバッファから消す
					while prevLines and prevLines[-1]['type'] == blankLine:
						prevLines.pop(-1)
			else:
				# 最初の行が空白のとき
				if type == blankLine:
					continue

			# 出力すべきバッファにキャッシュする
			prevLines.append(lineType)

	with codecs.open(tmp_file, "w", "utf_8_sig") as fout:
		for lineType in prevLines:
			fout.write(lineType['text'] + endOfLine)

	os.remove(file)
	os.rename(tmp_file, file)

# 対象のファイルをすべて処理する
def processFiles(files):
	for file in files:
		print ("processing " + file)
		removeRedundantBlack(file)

if __name__ == '__main__':
	if len(sys.argv) < 2:
		print ("usage: " + os.path.basename(sys.argv[0]) + " <top dir>")
		sys.exit(1)

	processFiles(checkAll(sys.argv[1]))
