# -*- coding: utf-8 -*-
import os
import sys
import re
import codecs

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

# 引数で指定したファイルに対して @file コメントがあるかチェックする
def hasFileComment(file):
	with codecs.open(file, "r", "utf_8_sig") as fin:
		for line in fin:
			match = re.search(r'(\\|@)file\b', line)
			if match:
				return True
				
	return False

# 引数で指定したファイルに対して @file コメントをつける
def addFileComment(file):
	fileComment = "/*! @file */"
	endOfLine   = "\r\n"

	tmp_file = file + ".tmp"
	with codecs.open(tmp_file, "w", "utf_8_sig") as fout:
		with codecs.open(file, "r", "utf_8_sig") as fin:
			# ファイル先頭に @file コメントをつける
			fout.write(fileComment + endOfLine)

			for line in fin:
				text = clipEndOfLine(line)
				fout.write(text + endOfLine)
	os.remove(file)
	os.rename(tmp_file, file)

# 対象のファイルをすべて処理する
# (@file コメントをつける)
def processFiles(files):
	for file in files:
		if hasFileComment(file) == False:
			print ("processing " + file)
			addFileComment(file)

if __name__ == '__main__':
	if len(sys.argv) < 2:
		print ("usage: " + os.path.basename(sys.argv[0]) + " <top dir>")
		sys.exit(1)

	processFiles(checkAll(sys.argv[1]))
