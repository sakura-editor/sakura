# -*- coding: utf-8 -*-
import os
import sys
import re
import codecs


def tweek_source(fin, fout):
	for line in fin:
		# すでに挿入済みの #line を削除
		match = re.search(r'^#line', line)
		if match:
			continue
		
		# assert_warning, static_assert, assert を削除
		assert_lines = []
		pattern = r'\s*(assert_warning|static_assert|assert)\s*\(.*'
		match = re.search(pattern, line)
		if match:
			match = re.search(r'^\s*#\s*define\s+', line)
			if not match:
				while True:
					assert_lines.append(line)

					# セミコロンの有無をチェック
					match = re.search(r';', line)
					if match:
						break
					line = fin.readline()

				one_line = ''.join(assert_lines)
				assert_lines = []

				one_line = re.sub(r'(\s*)(assert_warning|static_assert|assert)\s*\(.*\);', r'\1;', one_line, flags=re.MULTILINE)
				fout.write(one_line)
				continue

		# malloc の前の行に #line を挿入
		match = re.search(r'\b(malloc|realloc|malloc_char)\b\s*\(', line)
		if match:
			fout.write("#line 999\r\n")
			fout.write(line)
			continue
	
		# _wcsdup の前の行に #line を挿入
		match = re.search(r'\b(_wcsdup)\b\s*\(', line)
		if match:
			fout.write("#line 1000\r\n")
			fout.write(line)
			continue
	
		# _wtempnam の前の行に #line を挿入
		match = re.search(r'\b(_wtempnam)\b\s*\(', line)
		if match:
			fout.write("#line 1200\r\n")
			fout.write(line)
			continue

		# そのまま出力
		fout.write(line)

def cutcomment(fileName):
	if fileName.endswith('.cpp') or fileName.endswith('.h'):
		print (fileName)
		tmp_file = fileName + ".tmp"
		with codecs.open(tmp_file, "w", "utf_8_sig") as fout:
			with codecs.open(fileName, "r", "utf_8_sig") as fin:
				tweek_source(fin, fout)
		os.remove(fileName)
		os.rename(tmp_file, fileName)

def processFiles(topdir):
	for workdir, dis, files in os.walk(topdir):
		for file in files:
			fullPath = os.path.join(workdir, file)
			cutcomment(fullPath)

if __name__ == '__main__':
	processFiles('.')
