# -*- coding: utf-8 -*-
import os
import sys
import re
import codecs
import subprocess
clang_format = r'"C:\Program Files\LLVM\bin\clang-format.exe"'

def processFiles(topdir):
	for workdir, dis, files in os.walk(topdir):
		for file in files:
			fullPath = os.path.join(workdir, file)
			if fullPath.endswith('.cpp') or fullPath.endswith('.h'):
				command = clang_format + ' -style=file -i ' + fullPath
				print(command)
				subprocess.call(command, shell=False)


if __name__ == '__main__':
	processFiles('.')
