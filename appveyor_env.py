# -*- coding: utf-8 -*-
import sys
import re
import os

def getURLJoinFunc():
	# for python 2
	try:
		import urlparse

		def joinFunc2(base, path):
			if not base.endswith('/'):
				base = base + '/'
		
			return urlparse.urljoin(base, path)
			
		return joinFunc2

	except ImportError:
		pass

	# for python 3
	try:
		import urllib.parse

		def joinFunc3(base, path):
			if not base.endswith('/'):
				base = base + '/'
		
			return urllib.parse.urljoin(base, path)
			
		return joinFunc3

	except ImportError:
		pass

	raise Exception

class AppveyorEnv():
	keysEnv = [
		"APPVEYOR_BUILD_FOLDER",
		"APPVEYOR_REPO_PROVIDER", 				# "gitHub" or etc
		"APPVEYOR_REPO_NAME",					# ex. "sakura-editor/sakura"
		"APPVEYOR_REPO_COMMIT",
		"APPVEYOR_PULL_REQUEST_NUMBER",
		"APPVEYOR_PULL_REQUEST_HEAD_COMMIT",
		"APPVEYOR_URL",							
		"APPVEYOR_ACCOUNT_NAME",
		"APPVEYOR_PROJECT_SLUG",
		"APPVEYOR_BUILD_VERSION",
		"APPVEYOR_BUILD_NUMBER",
	]

	def __init__(self):
		self.prefixGitHub = "https://github.com"
		self.env = {}
		self.var = {}
		self.gitHubCommitURL       = ""
		self.gitHubCommitURLPRHead = ""
		self.joinFunc = getURLJoinFunc()
		
		# 必要な環境変数をクラスの変数に保存
		for key in self.keysEnv:
			if key in os.environ:
				self.env[key] = os.environ[key]

		#	set APPVEYOR_SHORTHASH=%APPVEYOR_REPO_COMMIT:~0,8%
		if "APPVEYOR_REPO_COMMIT" in self.env:
			commitHash = self.env["APPVEYOR_REPO_COMMIT"]
			if len(commitHash) >= 8:
				self.var["APPVEYOR_SHORTHASH"] = commitHash[0:8]

		#	set APPVEYOR_SHORTHASH_PR_HEAD=%APPVEYOR_PULL_REQUEST_HEAD_COMMIT:~0,8%
		if "APPVEYOR_PULL_REQUEST_HEAD_COMMIT" in self.env:
			commitHashPRHead = self.env["APPVEYOR_PULL_REQUEST_HEAD_COMMIT"]
			if len(commitHashPRHead) >= 8:
				self.var["APPVEYOR_SHORTHASH_PR_HEAD"] = commitHashPRHead[0:8]

		if "APPVEYOR_URL" in self.env and "APPVEYOR_ACCOUNT_NAME" in self.env:
			if "APPVEYOR_PROJECT_SLUG" in self.env and "APPVEYOR_BUILD_VERSION" in self.env:
				temp = [
					self.env["APPVEYOR_URL"],
					"project",
					self.env["APPVEYOR_ACCOUNT_NAME"],
					self.env["APPVEYOR_PROJECT_SLUG"],
					"build",
					self.env["APPVEYOR_BUILD_VERSION"],
				]
				self.var["APPVEYOR_BUILD_URL"] = '/'.join(temp)

		if "APPVEYOR_REPO_PROVIDER" in self.env:
			if self.env["APPVEYOR_REPO_PROVIDER"] == "gitHub":
				if "APPVEYOR_REPO_NAME" in self.env:
					# GitHub のプロジェクト URL 作成
					temp = [
						self.prefixGitHub,
						self.env["APPVEYOR_REPO_NAME"],
					]
					repoURL = '/'.join(temp)

					# appveyor の commit Hash
					if "APPVEYOR_REPO_COMMIT" in self.env:
						# GITHUB_COMMIT_URL
						temp = [
							repoURL,
							"commit",
							self.env["APPVEYOR_REPO_COMMIT"],
						]
						self.var["GITHUB_COMMIT_URL"] = '/'.join(temp)

						# GITHUB_BLOB_ROOT_URL
						temp = [
							repoURL,
							"blob",
							self.env["APPVEYOR_REPO_COMMIT"],
						]
						self.var["GITHUB_BLOB_ROOT_URL"] = '/'.join(temp)

					# appveyor の PR の最新の commit Hash
					if "APPVEYOR_PULL_REQUEST_NUMBER" in self.env and "APPVEYOR_PULL_REQUEST_HEAD_COMMIT" in self.env:
						# GITHUB_COMMIT_URL_PR_HEAD
						temp = [
							repoURL,
							"pull",
							self.env["APPVEYOR_PULL_REQUEST_NUMBER"],
							"commits",
							self.env["APPVEYOR_PULL_REQUEST_HEAD_COMMIT"],
						]
						self.var["GITHUB_COMMIT_URL_PR_HEAD"] = '/'.join(temp)

	# msbuild のログに現れる小文字に変換されてしまったファイルパスを
	# もともとの大文字小文字を保存したパスに変換する。
	# os.walk() でファイルシステムを走査して、大文字小文字を区別しないで
	# 比較して一致したらそのパスを返す。
	def convertRealPath(self, path):
		pathLower = path.lower()
		dirname = os.path.dirname(path)
		for rootdir, dirs, files in os.walk(dirname):
			for file in files:
				work_path = os.path.join(rootdir, file)
				if work_path.lower() == pathLower:
					return work_path
		return path

	def getBlobURL(self, path):
		if "APPVEYOR_BUILD_FOLDER" in self.env:
			relpath = os.path.relpath(path, self.env["APPVEYOR_BUILD_FOLDER"])
		else:
			relpath = path
			
		relpath = self.convertRealPath(relpath)
		print (relpath)
		relpath = relpath.replace('\\', '/')

		blobURL = ""
		if "GITHUB_BLOB_ROOT_URL" in self.var:
			blobURL = self.joinFunc(self.var["GITHUB_BLOB_ROOT_URL"], relpath)
		return (blobURL, relpath)

	def getBlobURLWithLine(self, path, startLine):
		(blobURL, relpath) = self.getBlobURL(path)
		if blobURL:
			targetURL = blobURL + "#" + "L" + str(startLine)
			return (targetURL, relpath)
		else:
			return ("", relpath)

	def getBlobURLWithLines(self, path, startLine, endLine):
		(blobURL, relpath) = self.getBlobURL(path)
		if blobURL:
			targetURL = blobURL + "#" + "L" + str(startLine) + "-"  + "L" + str(endLine)
			return (targetURL, relpath)
		else:
			return ("", relpath)

	def printAll(self):
		for key in self.env.keys():
			print (key, self.env[key])
		for key in self.var.keys():
			print (key, self.var[key])

	# Appveyor 関連の環境変数をバッチファイルの形で保存する
	def saveEnvAsBat(self, file):
		with open(file, "w") as fout:
			for key in self.keysEnv:
				fout.write("set " + key + "=" + os.environ.get(key, "") + "\n")
		print ("wrote: " + file)

def main(fileBat):
	appveyor = AppveyorEnv()
	appveyor.printAll()
	
	file = r"appveyor_env.py"
	if os.path.exists(file):
		print (appveyor.getBlobURL(file))
		print (appveyor.getBlobURLWithLine(file, 1))
		print (appveyor.getBlobURLWithLines(file, 9, 15))
	
	file = r"sakura\preBuild.bat"
	if os.path.exists(file):
		print (appveyor.getBlobURL(file))
		print (appveyor.getBlobURLWithLine(file, 1))
		print (appveyor.getBlobURLWithLines(file, 9, 15))

	appveyor.saveEnvAsBat(fileBat)

if __name__ == '__main__':
	fileBat = "set_appveyor_env.bat"
	if len(sys.argv) > 1:
		fileBat = sys.argv[1]
	main(fileBat)
