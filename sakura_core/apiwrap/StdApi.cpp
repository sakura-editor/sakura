/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include <vector>
#include "StdApi.h"
#include "charset/charcode.h"

namespace ApiWrap{

	/*!
		MakeSureDirectoryPathExists の UNICODE 版。
		szDirPath で指定されたすべてのディレクトリを作成します。
		ディレクトリの記述は、ルートから開始します。

		@param DirPath
			有効なパス名を指定する、null で終わる文字列へのポインタを指定します。
			パスの最後のコンポーネントがファイル名ではなくディレクトリである場合、
			文字列の最後に円記号（\）を記述しなければなりません。 

		@returns
			関数が成功すると、TRUE が返ります。
			関数が失敗すると、FALSE が返ります。

		@note
			指定された各ディレクトリがまだ存在しない場合、それらのディレクトリを順に作成します。
			一部のディレクトリのみを作成した場合、この関数は FALSE を返します。

		@author
			kobake

		@date
			2007.10.15
	*/
	BOOL MakeSureDirectoryPathExistsW(LPCWSTR szDirPath)
	{
		const wchar_t* p=szDirPath-1;
		for (;;) {
			p=wcschr(p+1,L'\\');
			if(!p)break; //'\\'を走査し終わったので終了

			//先頭からpまでの部分文字列 -> szBuf
			wchar_t szBuf[_MAX_PATH];
			::wcsncpy_s(szBuf, szDirPath,p-szDirPath);

			//存在するか
			int nAcc = _waccess(szBuf,0);
			if(nAcc==0)continue; //存在するなら、次へ

			//ディレクトリ作成
			int nDir = _wmkdir(szBuf);
			if(nDir==-1)return FALSE; //エラーが発生したので、FALSEを返す
		}
		return TRUE;
	}
}
