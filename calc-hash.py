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
		for root, dirs, files in os.walk(topdir):
			for file in files:
				full_path = os.path.join(root, file)
				checksum = getSha256(full_path)
				fout.write(checksum + "\t" + full_path + "\n")
				print checksum + "\t" + full_path

hashFile = sys.argv[1]
topdir   = sys.argv[2]
writeHash(hashFile, topdir)
