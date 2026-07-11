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
patternUTF16 = (
	"utf-16"
)
expect_encoding = {
	".cpp" : patternUTF8_BOM,
	".h"   : patternUTF8_BOM,
	".rc"  : patternUTF16,
	".rc2" : patternUTF16
}

# チェック対象の拡張子リスト
extensions = expect_encoding.keys()

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
def check_extension(file_name):
	_, ext = os.path.splitext(file_name)
	return (ext in extensions)

# origin/master が存在するか確認する
# 戻り値
# origin/master が有効な場合 → 0
# origin/master が無効な場合 → 0以外
def check_origin_master():
	ret_code = 0
	try:
		subprocess.check_output('git show -s origin/master --')
	except subprocess.CalledProcessError as gitcode:
		ret_code = gitcode.returncode
	return ret_code

def get_merge_base():
	output = subprocess.check_output('git show-branch --merge-base origin/master HEAD')
	output_dec = output.decode()
	merge_base = output_dec.splitlines()
	return merge_base[0]

# ベースとの差分をチェック
def get_diff_files():
	merge_base = get_merge_base()

	output = subprocess.check_output('git diff ' + merge_base + ' --name-only --diff-filter=dr')
	output_dec = output.decode()
	diff_files = output_dec.splitlines()
	for file_name in diff_files:
		if check_extension(file_name):
			yield file_name
		else:
			print ("skip " + file_name)

# デバッグ用
# すべてのファイルを対象にチェック対象の拡張子のファイルの文字コードを調べてチェックする
def check_all():
	for rootdir, dirs, files in os.walk('.'):
		for file_name in files:
			if check_extension(file_name):
				full = os.path.join(rootdir, file_name)
				yield full

# 指定したファイルの文字コードが期待通りか確認する
def check_encoding_result(file_name, encoding):
	_, ext = os.path.splitext(file_name)
	encoding = encoding.lower()
	if encoding in expect_encoding.get(ext, ()):
		return True
	return False

# 指定されたファイルリストに対して文字コードが適切かチェックする
# (条件に満たないファイル数を返す。)
def process_files(files):
	# 条件に満たないファイル数
	count = 0

	for file_name in files:
		print ("checking " + file_name)
		encoding = check_encoding(file_name)
		if not check_encoding_result(file_name, encoding):
			print ("NG", encoding, file_name)
			count = count + 1
		else:
			print ("OK", encoding, file_name)
	return count

if __name__ == '__main__':
	user_scripts = os.path.join(site.USER_BASE, "Scripts")
	sys.path.append(user_scripts)
	print ("adding " + user_scripts + " to PATH")

	count = 0
	if len(sys.argv) > 1 and sys.argv[1] == "all":
		count = process_files(check_all())
	else:
		ret_code = check_origin_master()
		if ret_code == 0:
			count = process_files(get_diff_files())
		else:
			print ("skip. origin/master doesn't exist." + " ret_code = " + str(ret_code))

	if count > 0:
		print ("return 1")
		sys.exit(1)
	else:
		print ("return 0")
		sys.exit(0)
