#include "stdafx.h"
#include "CFile.h"
#include <io.h>

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               コンストラクタ・デストラクタ                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CFile::CFile()
: m_nFileShareModeOld( SHAREMODE_NOT_EXCLUSIVE )
, m_hLockedFile( INVALID_HANDLE_VALUE )
{
}

CFile::~CFile()
{
	DoFileUnLock();
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         各種判定                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CFile::IsFileExist() const
{
	return fexist(GetFilePath());
}

bool CFile::HasWritablePermission() const
{
	return -1 != _taccess( GetFilePath(), 2 );
}

bool CFile::IsFileWritable() const
{
	//書き込めるか検査
	HANDLE hFile = CreateFile(
		this->GetFilePath(),			//ファイル名
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

bool CFile::IsFileReadable() const
{
	HANDLE hTest = CreateFile(
		this->GetFilePath(),
		GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_SEQUENTIAL_SCAN,
		NULL
	);
	if(hTest==INVALID_HANDLE_VALUE){
		// 読み込みアクセス権がない
		return false;
	}
	CloseHandle( hTest );
	return true;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          ロック                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! ファイルの排他ロック解除
void CFile::DoFileUnLock()
{
	//クローズ
	if( m_hLockedFile != INVALID_HANDLE_VALUE ){
		::CloseHandle( m_hLockedFile );
		m_hLockedFile = INVALID_HANDLE_VALUE;
	}
	// ファイルの排他制御モード
	m_nFileShareModeOld = SHAREMODE_NOT_EXCLUSIVE;
}


//! ファイルの排他ロック
bool CFile::DoFileLock( EShareMode eShareMode )
{
	// ロック解除
	DoFileUnLock();

	// ファイルの存在チェック
	if( !this->IsFileExist() ){
		return false;
	}

	// モード設定
	if(eShareMode==SHAREMODE_NOT_EXCLUSIVE)return true;
	
	//フラグ
	DWORD dwShareMode=0;
	switch(eShareMode){
	case SHAREMODE_NOT_EXCLUSIVE:	return true;										break; //排他制御無し
	case SHAREMODE_DENY_READWRITE:	dwShareMode = 0;									break; //読み書き禁止→共有無し
	case SHAREMODE_DENY_WRITE:		dwShareMode = FILE_SHARE_READ;						break; //書き込み禁止→読み込みのみ認める
	default:						dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;	break; //禁止事項なし→読み書き共に認める
	}

	//オープン
	m_hLockedFile = CreateFile(
		this->GetFilePath(),			//ファイル名
		GENERIC_READ | GENERIC_WRITE,	//読み書きタイプ
		dwShareMode,					//共有モード
		NULL,							//既定のセキュリティ記述子
		OPEN_EXISTING,					//ファイルが存在しなければ失敗
		FILE_ATTRIBUTE_NORMAL,			//特に属性は指定しない
		NULL							//テンプレート無し
	);

	//結果
	if( INVALID_HANDLE_VALUE == m_hLockedFile ){
		const TCHAR*	pszMode;
		switch( eShareMode ){
		case SHAREMODE_DENY_READWRITE:	pszMode = _T("読み書き禁止モード"); break;
		case SHAREMODE_DENY_WRITE:		pszMode = _T("書き込み禁止モード"); break;
		default:						pszMode = _T("未定義のモード（問題があります）"); break;
		}
		WarningBeep();
		TopWarningMessage(
			CEditWnd::Instance()->GetHwnd(),
			_T("%ls\nを%lsでロックできませんでした。\n現在このファイルに対する排他制御は無効となります。"),
			GetFilePathClass().IsValidPath() ? GetFilePath() : _T("（無題）"),
			pszMode
		);
		return false;
	}
	m_nFileShareModeOld = eShareMode;

	return true;
}


