
# 指定したファイルサイズのファイルを作る
def create_test_file(filename, size):
	# 指定したサイズの文字列データを作る
	def getStringData(size):
		baseData = ''
		for x in range(size):
			data  = str(x % 10)
			baseData = baseData + data
		return baseData

	with open(filename, "wb") as fout:
		total = 0
		baseData = getStringData(500) + "\r\n"
		percent       = 0
		prev_percent  = 0

		while True:
			if total >= size:
				break
			elif size - total >= len(baseData):
				data = baseData
			else:
				data = getStringData(size - total)
		
			percent = int(total * 100 / size)
			if percent > prev_percent:
				elements = []
				elements.append("wrote")
				elements.append(filename)
				elements.append(":")
				elements.append(str(percent))
				elements.append("%")
				elements.append("[")
				elements.append(str(total))
				elements.append("/")
				elements.append(str(size))
				elements.append("]")

				print (' '.join(elements))
			writensize = fout.write(data.encode('ascii'))
			total = total + writensize
			
			prev_percent = percent

		print ("done " + filename)

if __name__ == '__main__':
	sizes = [
		(    1024 * 1024 * 1024, "1G" ),
		(4 * 1024 * 1024 * 1024, "4G" ),
		(5 * 1024 * 1024 * 1024, "5G" ),
	]

	for entry in sizes:
		size   = entry[0]
		suffix = entry[1]
		filename = "testData_" + suffix + ".txt"

		create_test_file(filename, size)
