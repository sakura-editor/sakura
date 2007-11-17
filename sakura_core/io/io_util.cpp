#include "stdafx.h"
#include "io_util.h"

//!指定ファイルに書き込めるかどうか調べる
bool IsFileWritable(LPCTSTR tszFilePath)
{
	//書き込めるか検査
	HANDLE hFile = CreateFile(
		tszFilePath,					//ファイル名
		GENERIC_READ | GENERIC_WRITE,	//読み書きモード
		0,								//共有無し
		NULL,							//既定のセキュリティ記述子
		OPEN_EXISTING,					//ファイルが存在しなければ失敗
		FILE_ATTRIBUTE_NORMAL,			//特に属性は指定しない
		NULL							//テンプレート無し
	);
	if(hFile==INVALID_HANDLE_VALUE){
		return false;
	}
	CloseHandle(hFile);
	return true;
}

//!指定の共有モードでファイルを開く
HANDLE OpenFile_InShareMode(LPCTSTR tszFilePath, EShareMode eShareMode)
{
	DWORD dwShareMode=0;

	//フラグを算出する
	switch(eShareMode){
	case SHAREMODE_DENY_READWRITE:	// 読み書き禁止
		dwShareMode = 0; //共有無し
		break;
	case SHAREMODE_DENY_WRITE:	// 書き込み禁止
		dwShareMode = FILE_SHARE_READ; //読み込みのみ認める
		break;
	default: //禁止事項なし
		dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE; //読み書き共に認める
		break;
	}

	//API呼び出し
	HANDLE hFile = CreateFile(
		tszFilePath,					//ファイル名
		GENERIC_READ | GENERIC_WRITE,	//読み書きタイプ
		dwShareMode,					//共有モード
		NULL,							//既定のセキュリティ記述子
		OPEN_EXISTING,					//ファイルが存在しなければ失敗
		FILE_ATTRIBUTE_NORMAL,			//特に属性は指定しない
		NULL							//テンプレート無し
	);

	return hFile;
}
