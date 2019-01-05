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

	match = re.search('^\s*}\s*(;\s*)?$', line)
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

# 冗長な空行を削除する
def removeRedundantBlack(fileName):
	endOfLine   = "\r\n"
	
	# 各行のデータを覚えておくバッファ
	# 行のデータと各行の種類の判定結果を記憶する
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
				#	以下のような場合に現在の行が空行の場合、現在の行を捨てる
				#		prevLines[-1]	leftBracket		{
				#		現在の行		blankLine		<空行>
				if prevLines[-1]['type'] == leftBracket:
					if lineType == blankLine:
						continue

				# 現在の行も前の行も空行の場合、現在の行を捨てる
				#		prevLines[-1]	blankLine		<空行>
				#		現在の行		blankLine		<空行>
				if prevLines[-1]['type'] == blankLine:
					if lineType == blankLine:
						continue

				#	以下のような場合に prevLines[-1] から <空行> 以外を見つけるまで
				#	prevLines の要素を後ろから消していく
				#		prevLines[-3]	otherLine		<その他の行>
				#		prevLines[-2]	blankLine		<空行>
				#		prevLines[-1]	blankLine		<空行>
				#		現在の行		rightBracket	}
				#
				if lineType == rightBracket:
					# 直前の空行をバッファから消す
					while prevLines and prevLines[-1]['type'] == blankLine:
						prevLines.pop(-1)
			else:
				# ファイルの先頭行が空行のとき、現在の行を捨てる
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
