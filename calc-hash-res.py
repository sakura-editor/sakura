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

def writeHash(hashFile, topdir):
	with open(hashFile, "w") as fout:
		checksums = {}
		for root, dirs, files in os.walk(topdir):
			for file in files:
				full_path = os.path.join(root, file)
				base, ext = os.path.splitext(file)
				if ext == '.res':
					checksum = getSha256(full_path)
					if checksum not in  checksums:
						checksums[checksum] = []
					checksums[checksum].append(full_path)
					
		for key, valueList in checksums.items():
			for value in valueList:
				fout.write(key + "\t" + value + "\n")
				print (key + "\t" + value)

if __name__ == '__main__':
	if len(sys.argv) < 3:
		print ("usage: " + sys.argv[0] + " <output hash file name> <search directory>")
		sys.exit(1)

	hashFile = sys.argv[1]
	topdir   = sys.argv[2]
	writeHash(hashFile, topdir)
