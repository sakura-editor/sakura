/*!	@file
	@brief Complementオブジェクト

*/
/*
	Copyright (C) 2009, syat
	Copyright (C) 2011, Moca
	

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
#ifndef SAKURA_COMPLEMENTIFOBJ_H_
#define SAKURA_COMPLEMENTIFOBJ_H_

#include "macro/CWSHIfObj.h"
#include "util/ole_convert.h"

class CComplementIfObj : public CWSHIfObj {
	// 型定義
	enum FuncId {
		F_OL_COMMAND_FIRST = 0,					//↓コマンドは以下に追加する
		F_OL_FUNCTION_FIRST = F_FUNCTION_FIRST,	//↓関数は以下に追加する
		F_CM_GETCURRENTWORD,					//補完対象の文字列を取得
		F_CM_GETOPTION,							//オプションを取得
		F_CM_ADDLIST,							//候補に追加
	};

	// コンストラクタ
public:
	CComplementIfObj( std::wstring& curWord, CHokanMgr* pMgr, int option )
		: CWSHIfObj( L"Complement", false )
		, m_sCurrentWord( curWord )
		, m_pHokanMgr( pMgr )
		, m_nOption( option )
	{
	}

	// デストラクタ
public:
	~CComplementIfObj(){}

	// 実装
public:
	//コマンド情報を取得する
	MacroFuncInfoArray GetMacroCommandInfo() const{ return m_MacroFuncInfoArr; }
	//関数情報を取得する
	MacroFuncInfoArray GetMacroFuncInfo() const{ return m_MacroFuncInfoNotCommandArr; };
	//関数を処理する
	bool HandleFunction(CEditView* View, EFunctionCode ID, const VARIANT *Arguments, const int ArgSize, VARIANT &Result)
	{
		Variant varCopy;	// VT_BYREFだと困るのでコピー用

		switch(LOWORD(ID)){
		case F_CM_GETCURRENTWORD:	//補完対象の文字列を取得
			{
				SysString s( m_sCurrentWord.c_str(), m_sCurrentWord.length() );
				Wrap( &Result )->Receive( s );
			}
			return true;
		case F_CM_GETOPTION:	//オプションを取得
			{
				Wrap( &Result )->Receive( m_nOption );
			}
			return true;
		case F_CM_ADDLIST:		//候補に追加する
			{
				std::wstring keyword;
				if( variant_to_wstr( Arguments[0], keyword ) != true) return false;

				const wchar_t* word = keyword.c_str();
				int nWordLen = keyword.length();
				Wrap( &Result )->Receive( -1 );
				if( NULL == m_pHokanMgr->m_pcmemKouho ){
					m_pHokanMgr->m_pcmemKouho = new CNativeW;
					m_pHokanMgr->m_pcmemKouho->SetString( word, nWordLen );
					m_pHokanMgr->m_pcmemKouho->AppendString( L"\n" );
					m_pHokanMgr->m_nKouhoNum++;
					Wrap( &Result )->Receive( m_pHokanMgr->m_nKouhoNum );
				}else{
					int nLen;
					const wchar_t* ptr = m_pHokanMgr->m_pcmemKouho->GetStringPtr( &nLen );
					bool ret = true;
					if( nWordLen < nLen ){
						if( L'\n' == ptr[nWordLen] && 0 == auto_memcmp( ptr, word, nWordLen ) ){
							ret = false;
						}else{
							const int nPosKouhoMax = nLen - nWordLen - 1;
							for( int nPosKouho = 1; nPosKouho < nPosKouhoMax; nPosKouho++ ){
								if( ptr[nPosKouho] == L'\n' ){
									if( ptr[nPosKouho + nWordLen + 1] == L'\n' ){
										if( 0 == auto_memcmp( &ptr[nPosKouho + 1], word, nWordLen) ){
											ret = false;
											break;
										}else{
											nPosKouho += nWordLen;
										}
									}
								}
							}
						}
					}
					if( ret ){
						m_pHokanMgr->m_pcmemKouho->AppendString( word, nWordLen );
						m_pHokanMgr->m_pcmemKouho->AppendString( L"\n" );
						m_pHokanMgr->m_nKouhoNum++;
						Wrap( &Result )->Receive( m_pHokanMgr->m_nKouhoNum );
					}
				}
				return true;
			}
		}
		return false;
	}
	//コマンドを処理する
	void HandleCommand(CEditView* View, EFunctionCode ID, const WCHAR* Arguments[], const int ArgSize)
	{
	}

	// メンバ変数
private:
	std::wstring m_sCurrentWord;
	CHokanMgr* m_pHokanMgr;
	int m_nOption; // 0x01 == IgnoreCase

private:
	static MacroFuncInfo m_MacroFuncInfoArr[];
	static MacroFuncInfo m_MacroFuncInfoNotCommandArr[];
};

//コマンド情報
MacroFuncInfo CComplementIfObj::m_MacroFuncInfoArr[] = 
{
	//ID									関数名							引数										戻り値の型	m_pszData
	//	終端
	{F_INVALID,	NULL, {VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}
};

//関数情報
MacroFuncInfo CComplementIfObj::m_MacroFuncInfoNotCommandArr[] = 
{
	//ID								関数名				引数										戻り値の型	m_pszData
	{EFunctionCode(F_CM_GETCURRENTWORD),L"GetCurrentWord",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_BSTR,	NULL }, //補完対象の文字列を取得
	{EFunctionCode(F_CM_GETOPTION),		L"GetOption",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, //補完対象の文字列を取得
	{EFunctionCode(F_CM_ADDLIST),		L"AddList",			{VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, //候補に追加する
	//	終端
	{F_INVALID,	NULL, {VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}
};

#endif /* SAKURA_COMPLEMENTIFOBJ_H_ */
