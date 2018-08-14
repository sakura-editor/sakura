import hashlib
import os
import sys

def getSha256(file):
	hash = hashlib.sha256()
	with open(file, 'rb') as f:
		while True:
			chunk = f.read(hash.block_size)
			if chunk:
				hash.update(chunk)
			else:
				break

	checksum = hash.hexdigest()
	return checksum

def writeHash(hashFile, topdir, extensions):
	def isMatch(file, extensions):
		base, ext = os.path.splitext(file)
		if extensions:
			for extension in extensions:
				if ext == extension or ext == "." + extension:
					return True
			return False
		else:
			return True

	with open(hashFile, "w") as fout:
		checksums = {}
		for root, dirs, files in os.walk(topdir):
			for file in files:
				full_path = os.path.join(root, file)
				if isMatch(file, extensions):
					checksum = getSha256(full_path)
					if checksum not in checksums:
						checksums[checksum] = []
					checksums[checksum].append(full_path)
					
		for key, valueList in checksums.items():
			for value in valueList:
				fout.write(key + "\t" + value + "\n")
				print (key + "\t" + value)

if __name__ == '__main__':
	if len(sys.argv) < 3:
		print ("usage: " + sys.argv[0] + " <output hash file name> <search directory>")
		print ("usage: " + sys.argv[0] + " <output hash file name> <search directory> [extension1] [extension2]")
		sys.exit(1)

	hashFile = sys.argv[1]
	topdir   = sys.argv[2]
	extensions = []
	if len(sys.argv) > 3:
		extensions = sys.argv[3:]
	writeHash(hashFile, topdir, extensions)
