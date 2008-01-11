/*!	@file
	@brief メモリバッファクラスへのファイル入力クラス

	@author Moca
	@date 2002/08/30 新規作成
*/
/*
	Copyright (C) 2002, Moca, genta
	Copyright (C) 2003, Moca, ryoji

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose, 
	including commercial applications, and to alter it and redistribute it 
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such, 
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/
#include <windows.h>
#include "CMemory.h"

#ifndef _CFILELOAD_H_
#define _CFILELOAD_H_

// VC6添付のヘッダで定義されてません
#ifndef INVALID_SET_FILE_POINTER
 #define INVALID_SET_FILE_POINTER 0xFFFFFFFF
#endif // INVALID_SET_FILE_POINTER


// ファイルのオープンに失敗したときに送出する例外
#ifndef DEF_CERROR_FILEOPEN

#define DEF_CERROR_FILEOPEN
class CError_FileOpen
{
};

#endif // DEF_CERROR_FILEOPEN

//! ファイルの読み込みに失敗したときに放出する例外
class CError_FileRead
{
};

/*!
	文字コードを変換してデータを行単位で取得するためのクラス
	@note 明示的にFileOpenメンバを呼び出さないと使えない
		ファイルポインタを共有すると困るので、クラスのコピー禁止
*/
class SAKURA_CORE_API CFileLoad
{
public:

	CFileLoad( void );
	~CFileLoad( void );

	//	Jul. 26, 2003 ryoji BOM引数追加
	ECodeType FileOpen( LPCTSTR, ECodeType, int, BOOL* pbBomExist = NULL );		// 指定文字コードでファイルをオープンする
	void FileClose( void );					// 明示的にファイルをクローズする

	//! 1行データをロードする 順アクセス用
	const wchar_t* ReadLine(
		CNativeW*	pUnicodeBuffer,	//!< [out] UNICODEデータ受け取りバッファ
		int*		pnLineLen,		//!< [out] 改行コード長を含む一行のデータ長。文字単位。
		CEOL*		pcEol			//!< [i/o]
	);

//	未実装関数郡
//	cosnt char* ReadAtLine( int, int*, CEOL* ); // 指定行目をロードする
//	cosnt wchar_t* ReadAtLineW( int, int*, CEOL* ); // 指定行目をロードする(Unicode版)
//	bool ReadIgnoreLine( void ); // 1行読み飛ばす


//	void SetReadBufAlloc( int );	// バッファサイズの変更

	//! ファイルの日時を取得する
	BOOL GetFileTime( FILETIME*, FILETIME*, FILETIME* ); // inline

	//	Jun. 08, 2003 Moca
	//! 開いたファイルにはBOMがあるか？
	BOOL IsBomExist( void ){ return m_bBomExist; }

	//! 現在の進行率を取得する(0% - 100%) 若干誤差が出る
	int GetPercent( void );

	//! ファイルサイズを取得する
	inline int GetFileSize( void ){ return m_nFileSize; }

	static const int gm_nBufSizeDef; // ロード用バッファサイズの初期値
//	static const int gm_nBufSizeMin; // ロード用バッファサイズの設定可能な最低値

protected:

	// コピーの禁止
	CFileLoad( const CFileLoad& ){}
	CFileLoad& operator= ( const CFileLoad& ){ return *this; }

	// Oct. 19, 2002 genta スペルミス修正
//	void SeekBegin( void );		// ファイルの先頭位置に移動する(BOMを考慮する)
	void Buffering( void );		// バッファにデータをロードする
	void ReadBufEmpty( void );	// バッファを空にする

	// GetLextLine の 文字コード考慮版
	const char* GetNextLineCharCode( const char*, int, int*, int*, CEOL*, int* );

	int Read( void*, size_t ); // inline
	DWORD FilePointer( DWORD, DWORD ); // inline

	/* メンバオブジェクト */

//	LPTSTR	m_pszFileName;	// ファイル名
	HANDLE	m_hFile;		// ファイルハンドル
	int		m_nFileSize;	// ファイルサイズ
	int		m_nFileDataLen;	// ファイルデータ長からBOM長を引いたバイト数
	int		m_nReadLength;	// 現在までにロードしたデータの合計バイト数(BOM長を含まない)
	int		m_nLineIndex;	// 現在ロードしている論理行(0開始)
	ECodeType	m_CharCode;		// 文字コード
	BOOL	m_bBomExist;	// ファイルのBOMが付いているか Jun. 08, 2003 Moca 
	int		m_nFlag;		// 文字コードの変換オプション
	//	Jun. 13, 2003 Moca
	//	状態をenumとしてわかりやすく．
	enum enumFileLoadMode{
		FLMODE_CLOSE = 0, //!< 初期状態
		FLMODE_OPEN, //!< ファイルオープンのみ
		FLMODE_REDY, //!< 順アクセスOK
		FLMODE_READBUFEND //!<ファイルの終端までバッファに入れた
	};
	enumFileLoadMode	m_eMode;		// 現在の読み込み状態

	// 読み込みバッファ系
	char*	m_pReadBuf;			// 読み込みバッファへのポインタ
	int		m_nReadBufSize;		// 読み込みバッファの実際に確保しているサイズ
	int		m_nReadDataLen;		// 読み込みバッファの有効データサイズ
	int		m_nReadBufOffSet;	// 読み込みバッファ中のオフセット(次の行頭位置)
//	int		m_nReadBufSumSize;	// 今までにバッファに読み込んだデータの合計サイズ

}; // class CFileLoad

// インライン関数郡

// public
inline BOOL CFileLoad::GetFileTime( FILETIME* pftCreate, FILETIME* pftLastAccess, FILETIME* pftLastWrite ){
	return ::GetFileTime( m_hFile, pftCreate, pftLastAccess, pftLastWrite );
}

// protected
inline int CFileLoad::Read( void* pBuf, size_t nSize )
{
	DWORD ReadSize;
	if( FALSE == ::ReadFile( m_hFile, pBuf, nSize, &ReadSize, NULL ) )
		throw CError_FileRead();
	return (int)ReadSize;
}

// protected
inline DWORD CFileLoad::FilePointer( DWORD offset, DWORD origin )
{
	DWORD fp;
	if( INVALID_SET_FILE_POINTER == ( fp = ::SetFilePointer( m_hFile, offset, NULL, FILE_BEGIN ) ) )
		throw CError_FileRead();
	return fp;
}


#endif /* _CFILELOAD_H_ */

/*[EOF]*/
