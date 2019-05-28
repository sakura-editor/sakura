# -*- coding: utf-8 -*-
import os
import sys
import re
import codecs

# 引数で指定した文字列から改行コードを取り除く
def clipEndOfLine(line):
	return line.rstrip('\r\n')

# 行からコメントを削除する
def clipCommet(line):
	return re.sub(r'//.*', r'', line)

# ファイルからコメントを削除する
def removeComment(inFile, outFile):
	with codecs.open(inFile, "r", "utf_8_sig") as fin:
		with codecs.open(outFile, "w", "utf_8_sig") as fout:
			for line in fin:
				text = clipEndOfLine(line)
				text = clipCommet(text)
				fout.write(text + "\r\n")

if __name__ == '__main__':
	if len(sys.argv) < 3:
		print ("usage: " + os.path.basename(sys.argv[0]) + " <src file> <dst file>")
		sys.exit(1)
		
	if not os.path.exists(sys.argv[1]):
		print (sys.argv[1] + " doesn't exist")
		sys.exit(1)

	removeComment(sys.argv[1], sys.argv[2])
