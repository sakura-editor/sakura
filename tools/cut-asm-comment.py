# -*- coding: utf-8 -*-
import os
import sys
import re
import codecs

def cutcomment(fileName):
	if fileName.endswith('.asm'):
		print (fileName)
		tmp_file = fileName + ".tmp"
		with codecs.open(tmp_file, "w", "utf_8_sig") as fout:
			with codecs.open(fileName, "r", "utf_8_sig") as fin:
				for line in fin:
					line = re.sub(r'\s*;.*$', '', line)
					fout.write(line)
		os.remove(fileName)
		os.rename(tmp_file, fileName)

def processFiles(topdir):
	for workdir, dis, files in os.walk(topdir):
		for file in files:
			fullPath = os.path.join(workdir, file)
			cutcomment(fullPath)

if __name__ == '__main__':
	processFiles('.')
