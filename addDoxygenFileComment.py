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

# 引数で指定したファイルに対して @file コメントがあるかチェックする
def hasFileComment(fileName):
	with codecs.open(fileName, "r", "utf_8_sig") as fin:
		for line in fin:
			match = re.search(r'(\\|@)file\b', line)
			if match:
				return True
				
	return False

# 引数で指定したファイルに対して @file コメントをつける
def addFileComment(fileName):
	fileComment = "/*! @file */"
	endOfLine   = "\r\n"

	tmp_file = fileName + ".tmp"
	with codecs.open(tmp_file, "w", "utf_8_sig") as fout:
		with codecs.open(fileName, "r", "utf_8_sig") as fin:
			# ファイル先頭に @file コメントをつける
			fout.write(fileComment + endOfLine)

			for line in fin:
				text = clipEndOfLine(line)
				fout.write(text + endOfLine)
	os.remove(fileName)
	os.rename(tmp_file, fileName)

# 対象のファイルをすべて処理する
# (@file コメントをつける)
def processFiles(files):
	for fileName in files:
		if not hasFileComment(fileName):
			print ("processing " + fileName)
			addFileComment(fileName)

if __name__ == '__main__':
	if len(sys.argv) < 2:
		print ("usage: " + os.path.basename(sys.argv[0]) + " <top dir>")
		sys.exit(1)

	processFiles(checkAll(sys.argv[1]))
