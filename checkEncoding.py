from chardet.universaldetector import UniversalDetector
import chardet
import os
import sys
import subprocess
import site

#########################################################
# 定数
#########################################################
patternUTF8_BOM   = (
	"utf-8-sig",
	"ascii"
)
patternUTF8_NoBOM = (
	"utf-8"
)
expectEncoding = {
	".cpp" : patternUTF8_BOM,
	".h"   : patternUTF8_BOM,
	".rc"  : patternUTF8_NoBOM,
}

# チェック対象の拡張子リスト
extensions = expectEncoding.keys()

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
def checkExtension(fileName):
	base, ext = os.path.splitext(fileName)
	return (ext in extensions)

# origin/master が存在するか確認する
# 戻り値
# origin/master が有効な場合 → 0
# origin/master が無効な場合 → 0以外
def checkOriginMaster():
	retCode = 0
	try:
		output = subprocess.check_output('git show -s origin/master --')
	except subprocess.CalledProcessError as gitcode:
		retCode = gitcode.returncode
	return retCode

def getMergeBase():
	output = subprocess.check_output('git show-branch --merge-base origin/master HEAD')
	outputDec = output.decode()
	mergeBase = outputDec.splitlines()
	return mergeBase[0]

# ベースとの差分をチェック
def getDiffFiles():
	mergeBase = getMergeBase()

	output = subprocess.check_output('git diff ' + mergeBase + ' --name-only --diff-filter=dr')
	outputDec = output.decode()
	diffFiles = outputDec.splitlines()
	for fileName in diffFiles:
		if checkExtension(fileName):
			yield fileName
		else:
			print ("skip " + fileName)

# デバッグ用
# すべてのファイルを対象にチェック対象の拡張子のファイルの文字コードを調べてチェックする
def checkAll():
	for rootdir, dirs, files in os.walk('.'):
		for fileName in files:
			if checkExtension(fileName):
				full = os.path.join(rootdir, fileName)
				yield full

# 指定したファイルの文字コードが期待通りか確認する
def checkEncodingResult(fileName, encoding):
	base, ext = os.path.splitext(fileName)
	encoding = encoding.lower()
	if encoding in expectEncoding.get(ext, ()):
		return True
	return False

# 指定されたファイルリストに対して文字コードが適切かチェックする
# (条件に満たないファイル数を返す。)
def processFiles(files):
	# 条件に満たないファイル数
	count = 0

	for fileName in files:
		print ("checking " + fileName)
		encoding = check_encoding(fileName)
		if not checkEncodingResult(fileName, encoding):
			print ("NG", encoding, fileName)
			count = count + 1
		else:
			print ("OK", encoding, fileName)
	return count

if __name__ == '__main__':
	user_scripts = os.path.join(site.USER_BASE, "Scripts")
	sys.path.append(user_scripts)
	print ("adding " + user_scripts + " to PATH")

	count = 0
	if len(sys.argv) > 1 and sys.argv[1] == "all":
		count = processFiles(checkAll())
	else:
		retCode = checkOriginMaster()
		if retCode == 0:
			count = processFiles(getDiffFiles())
		else:
			print ("skip. origin/master doesn't exist." + " retCode = " + str(retCode))

	if count > 0:
		print ("return 1")
		sys.exit(1)
	else:
		print ("return 0")
		sys.exit(0)
