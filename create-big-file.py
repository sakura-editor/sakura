
# 指定したファイルサイズのファイルを作る
def create_test_file(filename, size):
	data_size_one_line = 500

	# 指定したサイズの文字列データを作る
	def get_string_data(size):
		base_data = ''
		for x in range(size):
			data  = str(x % 10)
			base_data = base_data + data
		return base_data

	# 最後の行用に指定したサイズの文字列データを作る
	def get_last_data(size):
		base_data = ''
		if size > 1:
			base_data = base_data + '-' * (size-1)
		base_data = base_data + '#'
		return base_data
		
	# 書き込み状況をコンソールに出力します。
	def print_status(filename, percent, total, size):
		elements = []
		elements.append("wrote")
		elements.append(filename)
		elements.append(":")
		elements.append("{0:3d}".format(percent))
		elements.append("%")
		elements.append("[")
		elements.append(str(total))
		elements.append("/")
		elements.append(str(size))
		elements.append("]")
		print (' '.join(elements))

	with open(filename, "wb") as fout:
		total = 0
		base_data = get_string_data(data_size_one_line) + "\r\n"
		percent       = 0
		prev_percent  = 0

		while True:
			if total >= size:
				print_status(filename, 100, total, size)
				break
			elif size - total >= len(base_data):
				data = base_data
			else:
				data = get_last_data(size - total)
		
			percent = int(total * 100 / size)
			if percent > prev_percent:
				print_status(filename, percent, total, size)

			writensize = fout.write(data.encode('ascii'))
			total = total + writensize

			# 次回ループで比較用に保存する
			prev_percent = percent

		print ("done " + filename)

if __name__ == '__main__':
	sizes = [
		(1 * 1024 * 1024 * 1024    , "1G" ),
		(4 * 1024 * 1024 * 1024 - 1, "4G_minus1" ),
		(4 * 1024 * 1024 * 1024    , "4G" ),
		(4 * 1024 * 1024 * 1024 + 1, "4G_plus1"  ),
		(5 * 1024 * 1024 * 1024    , "5G" ),
	]

	for entry in sizes:
		size   = entry[0]
		suffix = entry[1]
		filename = "testData_" + suffix + ".txt"

		create_test_file(filename, size)
