/*!	@file
	@brief メモリバッファクラスへのファイル入力クラス

	@author Moca
	@date 2002/08/30 新規作成
*/
/*
	Copyright (C) 2002, Moca, genta
	Copyright (C) 2003, Moca, ryoji
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CFILELOAD_B9B7A22E_8C14_4913_8B92_3B5ABA6FC0DB_H_
#define SAKURA_CFILELOAD_B9B7A22E_8C14_4913_8B92_3B5ABA6FC0DB_H_
#pragma once

#include <Windows.h>
#include "CStream.h" //CError_FileOpen
#include "charset/CCodeBase.h"
#include "charset/CCodePage.h"
#include "util/design_template.h"

struct SEncodingConfig;
class CCodeBase;

/*!
	文字コードを変換してデータを行単位で取得するためのクラス
	@note 明示的にFileOpenメンバを呼び出さないと使えない
		ファイルポインタを共有すると困るので、クラスのコピー禁止
*/
class CFileLoad
{
public:
	static bool IsLoadableSize(ULONGLONG size, bool ignoreLimit = false);
	static ULONGLONG GetLimitSize();
	static std::wstring GetSizeStringForHuman(ULONGLONG size); //!< 人にとって見やすいサイズ文字列を作る (例: "2 GB", "10 GB", "400 MB", "32 KB")

public:
	CFileLoad() : CFileLoad( SEncodingConfig{} ) {};
	CFileLoad( const SEncodingConfig& encode );
	~CFileLoad( void );

	void Prepare( const CFileLoad& other, size_t nOffsetBegin, size_t nOffsetEnd );

	//	Jul. 26, 2003 ryoji BOM引数追加
	ECodeType FileOpen( LPCWSTR, bool bBigFile, ECodeType, int, bool* pbBomExist = NULL );		// 指定文字コードでファイルをオープンする
	void FileClose( void );					// 明示的にファイルをクローズする

	//! 1行データをロードする 順アクセス用
	EConvertResult ReadLine(
		CNativeW*	pUnicodeBuffer,	//!< [out] UNICODEデータ受け取りバッファ
		CEol*		pcEol			//!< [i/o]
	);

//	未実装関数郡
//	cosnt char* ReadAtLine( int, int*, CEol* ); // 指定行目をロードする
//	cosnt wchar_t* ReadAtLineW( int, int*, CEol* ); // 指定行目をロードする(Unicode版)
//	bool ReadIgnoreLine( void ); // 1行読み飛ばす

	//! ファイルの日時を取得する
	BOOL GetFileTime( FILETIME* pftCreate, FILETIME* pftLastAccess, FILETIME* pftLastWrite ); // inline

	//	Jun. 08, 2003 Moca
	//! 開いたファイルにはBOMがあるか？
	bool IsBomExist( void ){ return m_bBomExist; }

	//! 現在の進行率を取得する(0% - 100%) 若干誤差が出る
	int GetPercent( void );

	//! ファイルサイズを取得する
	inline LONGLONG GetFileSize( void ){ return m_nFileSize; }

	//! 指定オフセットから末尾に向かって最初に現れる行頭のオフセットを取得
	size_t GetNextLineOffset( size_t nOffset );

protected:
	// GetLextLine の 文字コード考慮版
	const char* GetNextLineCharCode(const char*	pData, size_t nDataLen, size_t* pnLineLen, size_t* pnBgn, CEol* pcEol, size_t* pnEolLen);
	EConvertResult ReadLine_core(CNativeW* pUnicodeBuffer, CEol* pcEol);

	/* メンバオブジェクト */
	const SEncodingConfig* m_pEencoding;

	//! 文字コード自動検出のために読み込む最大サイズ(byte)
	static constexpr LONGLONG m_nAutoDetectReadLen = 32768LL;

//	LPWSTR	m_pszFileName;	// ファイル名
	HANDLE	m_hFile;		// ファイルハンドル
	HANDLE	m_hFileMapping = nullptr;	// メモリマップドファイルハンドル
	LONGLONG	m_nFileSize;	// ファイルサイズ(64bit)
	LONGLONG	m_nFileDataLen;	// ファイルデータ長からBOM長を引いたバイト数
	int		m_nLineIndex;	// 現在ロードしている論理行(0開始)
	ECodeType	m_CharCode;		// 文字コード
	CCodeBase*	m_pCodeBase;	////
	EEncodingTrait	m_encodingTrait;
	CMemory			m_memEols[3];
	bool	m_bEolEx;		//!< CR/LF以外のEOLが有効か
	int		m_nMaxEolLen;	//!< EOLの長さ
	bool	m_bBomExist;	// ファイルのBOMが付いているか Jun. 08, 2003 Moca 
	int		m_nFlag;		// 文字コードの変換オプション
	//	Jun. 13, 2003 Moca
	//	状態をenumとしてわかりやすく．
	enum enumFileLoadMode{
		FLMODE_CLOSE = 0, //!< 初期状態
		FLMODE_OPEN, //!< ファイルオープンのみ
		FLMODE_READY, //!< 順アクセスOK
		FLMODE_READBUFEND //!<ファイルの終端までバッファに入れた
	};
	enumFileLoadMode	m_eMode;		// 現在の読み込み状態

	// 読み込みバッファ系
	const char*	m_pReadBufTop = nullptr;	// 読み込みバッファの先頭を指すポインタ
	size_t m_nReadBufOffsetBegin = 0;
	size_t m_nReadBufOffsetEnd = 0;
	size_t m_nReadBufOffsetCurrent = 0;		// 読み込みバッファ中のオフセット(次の行頭位置)
	CMemory m_cLineBuffer;
	CNativeW m_cLineTemp;
	int		m_nReadOffset2;
	EConvertResult m_nTempResult;

	DISALLOW_COPY_AND_ASSIGN(CFileLoad);
}; // class CFileLoad

// インライン関数郡

// public
inline BOOL CFileLoad::GetFileTime( FILETIME* pftCreate, FILETIME* pftLastAccess, FILETIME* pftLastWrite ){
	return ::GetFileTime( m_hFile, pftCreate, pftLastAccess, pftLastWrite );
}
#endif /* SAKURA_CFILELOAD_B9B7A22E_8C14_4913_8B92_3B5ABA6FC0DB_H_ */
