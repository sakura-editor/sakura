from chardet.universaldetector import UniversalDetector
import chardet
import os
import sys
import subprocess 

# 指定したファイルの文字コードを返す
def check_encoding(file_path):
	detector = UniversalDetector()
	with open(file_path, mode='rb') as f:
		for binary in f:
			detector.feed(binary)
			if detector.done:
				break
	detector.close()
	return detector.result['encoding']

# チェック対象の拡張子か判断する
def checkExtension(file):
	extensions = [
		".cpp",
		".h",
	]
	base, ext = os.path.splitext(file)
	if ext in extensions:
		return True
	else:
		return False
		
def getMergeBase():
	output = subprocess.check_output('git show-branch --merge-base origin/master HEAD')
	outputDec = output.decode()
	mergeBase = outputDec.splitlines()
	return mergeBase[0]

# ベースとの差分をチェック
def getDiffFiles():
	mergeBase = getMergeBase()

	output = subprocess.check_output('git diff ' + mergeBase + ' --name-only --diff-filter=dr')
	diffFiles = output.decode()
	return diffFiles.splitlines()

def checkEncodingResult(encoding):
	expectEncoding = [
		"utf-8",
		"utf-8-sig",
		"ascii",
	]
	encoding = encoding.lower()
	if encoding in expectEncoding:
		return True

	return False

# デバッグ用
# すべてのファイルの文字コードを調べてチェックする
def checkAll():
	for rootdir, dirs, files in os.walk('.'):
		for file in files:
			if checkExtension(file):
				full = os.path.join(rootdir, file)
				encoding = check_encoding(full)
				if not checkEncodingResult(encoding):
					print (encoding, full)

if __name__ == '__main__':
	count = 0

	files = getDiffFiles()
	for file in files:
		if checkExtension(file):
			encoding = check_encoding(file)
			if not checkEncodingResult(encoding):
				print (encoding, file)
				count = count + 1

	if count > 0:
		print ("return 1")
		sys.exit(1)
	else:
		print ("return 0")
		sys.exit(0)
