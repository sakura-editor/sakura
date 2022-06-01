﻿/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

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
#include "StdAfx.h"
#include "CCodeBase.h"

#include "charset/CCodeFactory.h"
#include "convert/convert_util2.h"
#include "charset/codechecker.h"
#include "CEol.h"
#include "env/CommonSetting.h"

/*!
	文字コードの16進表示

	ステータスバー表示用に文字を16進表記に変換する

	@param [in] cSrc 変換する文字
	@param [in] sStatusbar 共通設定 ステータスバー
	@param [in,opt] bUseFallback cSrcが特定コードで表現できない場合にフォールバックするかどうか
 */
std::wstring CCodeBase::CodeToHex(const CNativeW& cSrc, const CommonSetting_Statusbar& sStatusbar, bool bUseFallback /* = true */)
{
	std::wstring buff(32, L'\0');
	if (const auto ret = UnicodeToHex(cSrc.GetStringPtr(), cSrc.GetStringLength(), buff.data(), &sStatusbar);
		ret != RESULT_COMPLETE && bUseFallback) {
		// うまくコードが取れなかった(Unicodeで表示)
		return CCodeFactory::CreateCodeBase(CODE_UNICODE)->CodeToHex(cSrc, sStatusbar, false);
	}
	return buff;
}

// 表示用16進表示	UNICODE → Hex 変換	2008/6/9 Uchi
EConvertResult CCodeBase::UnicodeToHex(const wchar_t* cSrc, const int iSLen, WCHAR* pDst, const CommonSetting_Statusbar* psStatusbar)
{
	if (IsUTF16High(cSrc[0]) && iSLen >= 2 && IsUTF16Low(cSrc[1])) {
		// サロゲートペア
		if (psStatusbar->m_bDispSPCodepoint) {
			auto_sprintf( pDst, L"U+%05X", 0x10000 + ((cSrc[0] & 0x3FF)<<10) + (cSrc[1] & 0x3FF));
		}
		else {
			auto_sprintf( pDst, L"%04X%04X", cSrc[0], cSrc[1]);
		}
	}
	else {
		auto_sprintf( pDst, L"U+%04X", cSrc[0] );
	}

	return RESULT_COMPLETE;
}

/*!
	MIME デコーダー

	@param[out] pcMem デコード済みの文字列を格納
*/
bool CCodeBase::MIMEHeaderDecode( const char* pSrc, const int nSrcLen, CMemory* pcMem, const ECodeType eCodetype )
{
	ECodeType ecodetype;
	int nskip_bytes;

	// ソースを取得
	pcMem->AllocBuffer( nSrcLen );
	char* pdst = reinterpret_cast<char*>( pcMem->GetRawPtr() );
	if( pdst == NULL ){
		pcMem->SetRawData( "", 0 );
		return false;
	}

	CMemory cmembuf;
	int i = 0;
	int j = 0;
	while( i < nSrcLen ){
		if( pSrc[i] != '=' ){
			pdst[j] = pSrc[i];
			++i;
			++j;
			continue;
		}
		nskip_bytes = _DecodeMimeHeader( &pSrc[i], nSrcLen-i, &cmembuf, &ecodetype );
		if( nskip_bytes < 1 ){
			pdst[j] = pSrc[i];
			++i;
			++j;
		}else{
			if( ecodetype == eCodetype ){
				// eChartype が ecodetype と一致している場合にだけ、
				// 変換結果をコピー
				memcpy( &pdst[j], cmembuf.GetRawPtr(), cmembuf.GetRawLength() );
				i += nskip_bytes;
				j += cmembuf.GetRawLength();
			}else{
				memcpy( &pdst[j], &pSrc[i], nskip_bytes );
				i += nskip_bytes;
				j += nskip_bytes;
			}
		}
	}

	pcMem->_SetRawLength( j );
	return true;
}

/*!
	BOMデータ取得

	ByteOrderMarkに対する特定コードによるバイナリ表現を取得する。
	マルチバイトなUnicode文字セットのバイト順を識別するのに使う。
 */
[[nodiscard]] BinarySequence CCodeBase::GetBomDefinition()
{
	const CNativeW cBom( L"\xFEFF" );

	bool bComplete = false;
	auto converted = UnicodeToCode( cBom, &bComplete );
	if( !bComplete ){
		converted.clear();
	}

	return converted;
}

/*!
	BOMデータ取得

	ByteOrderMarkに対する特定コードによるバイナリ表現を取得する。
	マルチバイトなUnicode文字セットのバイト順を識別するのに使う。
 */
void CCodeBase::GetBom( CMemory* pcmemBom )
{
	if( pcmemBom != nullptr ){
		if( const auto bom = GetBomDefinition(); 0 < bom.length() ){
			pcmemBom->SetRawData( bom.data(), bom.length() );
		}else{
			pcmemBom->Reset();
		}
	}
}


/*!
	改行データ取得

	各種行終端子に対する特定コードによるバイナリ表現のセットを取得する。
	特定コードで利用できない行終端子については空のバイナリ表現が返る。
 */
[[nodiscard]] std::map<EEolType, BinarySequence> CCodeBase::GetEolDefinitions()
{
	constexpr struct {
		EEolType type;
		std::wstring_view str;
	}
	aEolTable[] = {
		{ EEolType::cr_and_lf,				L"\x0d\x0a",	},
		{ EEolType::line_feed,				L"\x0a",		},
		{ EEolType::carriage_return,		L"\x0d",		},
		{ EEolType::next_line,				L"\x85",		},
		{ EEolType::line_separator,			L"\u2028",		},
		{ EEolType::paragraph_separator,	L"\u2029",		},
	};

	std::map<EEolType, BinarySequence> map;
	for( auto& eolData : aEolTable ){
		bool bComplete = false;
		const auto& str = eolData.str;
		auto converted = UnicodeToCode( CNativeW( str.data(), str.length() ), &bComplete );
		if( !bComplete ){
			converted.clear();
		}
		map.try_emplace( eolData.type, std::move(converted) );
	}

	return map;
}

/*!
	改行データ取得

	指定した行終端子に対する特定コードによるバイナリ表現を取得する。
	コードポイントとバイナリシーケンスが1対1に対応付けられる文字コードの改行を検出するのに使う。
 */
void CCodeBase::GetEol( CMemory* pcmemEol, EEolType eEolType )
{
	if( pcmemEol != nullptr ){
		const auto map = GetEolDefinitions();
		if( auto it = map.find( eEolType ); it != map.end() ){
			const auto& bin = it->second;
			pcmemEol->SetRawData( bin.data(), bin.length() );
		}else{
			pcmemEol->Reset();
		}
	}
}
