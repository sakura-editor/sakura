/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CCODEBASE_1AB194FB_933C_495E_A3A3_62E117C72644_H_
#define SAKURA_CCODEBASE_1AB194FB_933C_495E_A3A3_62E117C72644_H_
#pragma once

#include <cstddef>
#include <map>
#include <string>
#include <string_view>

#include "mem/CNativeW.h"
#include "charset/charset.h"
#include "CEol.h"

//定数
enum EConvertResult{
	RESULT_COMPLETE, //!< データを失うことなく変換が完了した。
	RESULT_LOSESOME, //!< 変換が完了したが、一部のデータが失われた。
	RESULT_FAILURE,  //!< 何らかの原因により失敗した。
};

struct CommonSetting_Statusbar;

//! 変換元バイナリシーケンスを表す型。
using BinarySequenceView = std::basic_string_view<std::byte>;

//! 復元後バイナリシーケンスを表す型。
using BinarySequence = std::basic_string<std::byte>;

/*!
	文字コード基底クラス。
	
	ここで言う「特定コード」とは、
	CCodeBaseを継承した子クラスが定める、一意の文字コードのことです。
*/
class CCodeBase{
public:
	virtual ~CCodeBase() noexcept = default;

	/*!
		特定コードをUnicodeにエンコードする

		@param [in] cSrc 変換対象のバイナリシーケンス
		@param [out,opt] pResult 変換結果を受け取る変数
		@returns サクラエディタ仕様のUnicode文字列
	 */
	virtual CNativeW CodeToUnicode( BinarySequenceView cSrc, bool* pResult = nullptr )
	{
		CMemory cmemSrc( cSrc.data(), cSrc.size() );
		CNativeW cDest;
		auto result = CodeToUnicode( cmemSrc, &cDest );
		if( pResult ){
			*pResult = result == RESULT_COMPLETE;
		}
		return cDest;
	}

	/*!
		Unicodeを特定コードにデコードする

		@param [in] cSrc 変換対象のUnicodeシーケンス
		@param [out,opt] pResult 変換結果を受け取る変数
		@returns バイナリシーケンス
	 */
	virtual BinarySequence UnicodeToCode( const CNativeW& cSrc, bool* pResult = nullptr )
	{
		CMemory cDest;
		auto result = UnicodeToCode( cSrc, &cDest );
		if( pResult ){
			*pResult = result == RESULT_COMPLETE;
		}
		return BinarySequence( static_cast<std::byte*>(cDest.GetRawPtr()), cDest.GetRawLength() );
	}

	//文字コード変換
	virtual EConvertResult CodeToUnicode(const CMemory& cSrc, CNativeW* pDst)=0;	//!< 特定コード → UNICODE    変換
	virtual EConvertResult UnicodeToCode(const CNativeW& cSrc, CMemory* pDst)=0;	//!< UNICODE    → 特定コード 変換
	//! UNICODE    → 特定コード 変換
	virtual EConvertResult UnicodeToCode(const CStringRef& cSrc, CMemory* pDst){
		CNativeW mem(cSrc.GetPtr(), cSrc.GetLength());
		return UnicodeToCode(mem, pDst);
	}

	//ファイル形式
	[[nodiscard]] virtual BinarySequence GetBomDefinition();
	void GetBom( CMemory* pcmemBom );
	[[nodiscard]] virtual std::map<EEolType, BinarySequence> GetEolDefinitions();
	void GetEol( CMemory* pcmemEol, EEolType eEolType );

	// 文字コードの16進表示
	virtual std::wstring CodeToHex(const CNativeW& cSrc, const CommonSetting_Statusbar& sStatusbar, bool bUseFallback = true);
	// 文字コード表示用		2008/6/9 Uchi
	virtual EConvertResult UnicodeToHex(const wchar_t* cSrc, const int iSLen, WCHAR* pDst, const CommonSetting_Statusbar* psStatusbar);			//!< UNICODE → Hex 変換

	// 変換エラー処理（１バイト <-> U+D800 から U+D8FF）
	static int BinToText(const unsigned char *pSrc, const int nLen, unsigned short *pDst);
	static int TextToBin(const unsigned short cSrc);

	// MIME Header デコーダ
	static bool MIMEHeaderDecode(const char* pSrc, const int nSrcLen, CMemory* pcMem, const ECodeType eCodetype);
};

/*!
	バイナリ１バイトを U+DC00 から U+DCFF までに対応付ける
*/
inline int CCodeBase::BinToText( const unsigned char *pSrc, const int nLen, unsigned short *pDst )
{
	int i;

	for( i = 0; i < nLen; ++i ){
		pDst[i] = static_cast<unsigned short>(pSrc[i]) + 0xdc00;
	}

	return i;
}

/*!
	U+DC00 から U+DCFF からバイナリ1バイトを復元
*/
inline int CCodeBase::TextToBin( const unsigned short cSrc )
{
	return static_cast<int>((cSrc - 0xdc00) & 0x00ff);
}
#endif /* SAKURA_CCODEBASE_1AB194FB_933C_495E_A3A3_62E117C72644_H_ */
