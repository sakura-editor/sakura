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
def checkExtension(fileName):
	base, ext = os.path.splitext(fileName)
	return (ext in extensions)

# 引数で指定したフォルダ以下のすべての対象ファイルを yield で返す
def checkAll(topDir):
	for rootdir, dirs, files in os.walk(topDir):
		for fileName in files:
			if checkExtension(fileName):
				full = os.path.join(rootdir, fileName)
				yield full

# 引数で指定した文字列から改行コードを取り除く
def clipEndOfLine(line):
	return line.rstrip('\r\n')

# 引数で指定したファイルに対して @file コメントをつける
def removeRedundantBlack(fileName):
	endOfLine   = "\r\n"
	prevLines   = []

	tmp_file = fileName + ".tmp"
	with codecs.open(fileName, "r", "utf_8_sig") as fin:
		for line in fin:
			text     = clipEndOfLine(line)
			lineType = parseLine(text)
			elem = {}
			elem['text'] = text
			elem['type'] = lineType
			
			if prevLines:
				# 最初の行ではないとき
				if prevLines[-1]['type'] == leftBracket:
					if type == blankLine:
						continue

				if prevLines[-1]['type'] == blankLine:
					if lineType == blankLine:
						continue

				if lineType == rightBracket:
					# 直前の空行をバッファから消す
					while prevLines and prevLines[-1]['type'] == blankLine:
						prevLines.pop(-1)
			else:
				# 最初の行が空白のとき
				if lineType == blankLine:
					continue

			# 出力すべきバッファにキャッシュする
			prevLines.append(elem)

	with codecs.open(tmp_file, "w", "utf_8_sig") as fout:
		for elem in prevLines:
			fout.write(elem['text'] + endOfLine)

	os.remove(fileName)
	os.rename(tmp_file, fileName)

# 対象のファイルをすべて処理する
def processFiles(files):
	for fileName in files:
		print ("processing " + fileName)
		removeRedundantBlack(fileName)

if __name__ == '__main__':
	if len(sys.argv) < 2:
		print ("usage: " + os.path.basename(sys.argv[0]) + " <top dir>")
		sys.exit(1)

	processFiles(checkAll(sys.argv[1]))
