/*! @file */
/*
	2008.05.18 kobake CShareData から分離
*/
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"

#include "DLLSHAREDATA.h"
#include "CFileNameManager.h"
#include "charset/CCodePage.h"
#include "util/module.h"
#include "util/os.h"
#include "util/shell.h"
#include "util/string_ex2.h"
#include "util/file.h"
#include "util/window.h"
#include "CSelectLang.h"
#include "String_define.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      ファイル名管理                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!	共有データの設定に従ってパスを縮小表記に変換する
	@param pszSrc   [in]  ファイル名
	@param pszDest  [out] 変換後のファイル名の格納先
	@param nDestLen [in]  終端のNULLを含むpszDestのWCHAR単位の長さ _MAX_PATH まで
	@date 2003.01.27 Moca 新規作成
	@note 連続して呼び出す場合のため、展開済みメタ文字列をキャッシュして高速化している。
*/
LPWSTR CFileNameManager::GetTransformFileNameFast( LPCWSTR pszSrc, LPWSTR pszDest, int nDestLen, HDC hDC, bool bFitMode, int cchMaxWidth )
{
	int i;
	WCHAR szBuf[_MAX_PATH + 1];

	if( -1 == m_nTransformFileNameCount ){
		TransformFileName_MakeCache();
	}

	int nPxWidth = -1;
	if( m_pShareData->m_Common.m_sFileName.m_bTransformShortPath && cchMaxWidth != -1 ){
		if( cchMaxWidth == 0 ){
			cchMaxWidth = m_pShareData->m_Common.m_sFileName.m_nTransformShortMaxWidth;
		}
		CTextWidthCalc calc(hDC);
		nPxWidth = calc.GetTextWidth(L"x") * cchMaxWidth;
	}

	if( 0 < m_nTransformFileNameCount ){
		GetFilePathFormat( pszSrc, pszDest, nDestLen,
			m_szTransformFileNameFromExp[0],
			m_pShareData->m_Common.m_sFileName.m_szTransformFileNameTo[m_nTransformFileNameOrgId[0]]
		);
		for( i = 1; i < m_nTransformFileNameCount; i++ ){
			wcscpy( szBuf, pszDest );
			GetFilePathFormat( szBuf, pszDest, nDestLen,
				m_szTransformFileNameFromExp[i],
				m_pShareData->m_Common.m_sFileName.m_szTransformFileNameTo[m_nTransformFileNameOrgId[i]] );
		}
		if( nPxWidth != -1 ){
			wcscpy( szBuf, pszDest );
			GetShortViewPath( pszDest, nDestLen, szBuf, hDC, nPxWidth, bFitMode );
		}
	}else if( nPxWidth != -1 ){
		GetShortViewPath( pszDest, nDestLen, pszSrc, hDC, nPxWidth, bFitMode );
	}else{
		// 変換する必要がない コピーだけする
		wcsncpy( pszDest, pszSrc, nDestLen - 1 );
		pszDest[nDestLen - 1] = '\0';
	}
	return pszDest;
}

/*!	展開済みメタ文字列のキャッシュを作成・更新する
	@retval 有効な展開済み置換前文字列の数
	@date 2003.01.27 Moca 新規作成
	@date 2003.06.23 Moca 関数名変更
*/
int CFileNameManager::TransformFileName_MakeCache( void ){
	int i;
	int nCount = 0;
	for( i = 0; i < m_pShareData->m_Common.m_sFileName.m_nTransformFileNameArrNum; i++ ){
		if( L'\0' != m_pShareData->m_Common.m_sFileName.m_szTransformFileNameFrom[i][0] ){
			if( ExpandMetaToFolder( m_pShareData->m_Common.m_sFileName.m_szTransformFileNameFrom[i],
			 m_szTransformFileNameFromExp[nCount], _MAX_PATH ) ){
				// m_szTransformFileNameToとm_szTransformFileNameFromExpの番号がずれることがあるので記録しておく
				m_nTransformFileNameOrgId[nCount] = i;
				nCount++;
			}
		}
	}
	m_nTransformFileNameCount = nCount;
	return nCount;
}

/*!	ファイル・フォルダー名を置換して、簡易表示名を取得する
	@date 2002.11.27 Moca 新規作成
	@note 大小文字を区別しない。nDestLenに達したときは後ろを切り捨てられる
*/
LPCWSTR CFileNameManager::GetFilePathFormat( std::wstring_view strSrc, LPWSTR pszDest, size_t nDestLen, std::wstring_view strFrom, std::wstring_view strTo )
{
	auto it = strSrc.cbegin();
	wchar_t* pDest = pszDest;
	const wchar_t* pEnd = pszDest + nDestLen;
	while( it < strSrc.cend() && pDest + 1 < pEnd ){
		if( strFrom.length() <= static_cast<size_t>(strSrc.cend() - it) && 0 == ::_wcsnicmp( &*it, strFrom.data(), strFrom.length() ) ){
			if( strTo.length() < static_cast<size_t>(pEnd - pDest) ){
				::wcsncpy_s( pDest, pEnd - pDest, strTo.data(), strTo.length() );
			}else{
				::wcsncpy_s( pDest, pEnd - pDest, strTo.data(), _TRUNCATE );
				return pszDest;
			}
			pDest += ::wcsnlen( pDest, pEnd - pDest );
			it += strFrom.length();
		}else{
			::wcsncpy_s( pDest, pEnd - pDest, &*it, 1 );
			++pDest;
			++it;
		}
	}
	return pszDest;
}

/*!	%MYDOC%などのパラメータ指定を実際のパス名に変換する

	@param pszSrc  [in]  変換前文字列
	@param pszDes  [out] 変換後文字列
	@param nDesLen [in]  pszDesのNULLを含むWCHAR単位の長さ
	@retval true  正常に変換できた
	@retval false バッファが足りなかった，またはエラー。pszDesは不定
	@date 2002.11.27 Moca 作成開始
*/
bool CFileNameManager::ExpandMetaToFolder( LPCWSTR pszSrc, LPWSTR pszDes, int nDesLen )
{
	LPCWSTR ps;
	LPWSTR  pd, pd_end;

#define _USE_META_ALIAS
#ifdef _USE_META_ALIAS
	struct MetaAlias{
		LPCWSTR szAlias;
		int nLenth;
		LPCWSTR szOrig;
	};
	static const MetaAlias AliasList[] = {
		{  L"COMDESKTOP", 10, L"Common Desktop" },
		{  L"COMMUSIC", 8, L"CommonMusic" },
		{  L"COMVIDEO", 8, L"CommonVideo" },
		{  L"MYMUSIC",  7, L"My Music" },
		{  L"MYVIDEO",  7, L"Video" },
		{  L"COMPICT",  7, L"CommonPictures" },
		{  L"MYPICT",   6, L"My Pictures" },
		{  L"COMDOC",   6, L"Common Documents" },
		{  L"MYDOC",    5, L"Personal" },
		{ NULL, 0 , NULL }
	};
#endif

	pd_end = pszDes + ( nDesLen - 1 );
	for( ps = pszSrc, pd = pszDes; L'\0' != *ps; ps++ ){
		if( pd_end <= pd ){
			if( pd_end == pd ){
				*pd = L'\0';
			}
			return false;
		}

		if( L'%' != *ps ){
			*pd = *ps;
			pd++;
			continue;
		}

		// %% は %
		if( L'%' == ps[1] ){
			*pd = L'%';
			pd++;
			ps++;
			continue;
		}

		if( L'\0' != ps[1] ){
			WCHAR szMeta[_MAX_PATH];
			WCHAR szPath[_MAX_PATH + 1];
			int   nMetaLen;
			int   nPathLen;
			bool  bFolderPath;
			LPCWSTR  pStr;
			ps++;
			// %SAKURA%
			if( 0 == wmemicmp( L"SAKURA%", ps, 7 ) ){
				// exeのあるフォルダー
				GetExedir( szPath );
				nMetaLen = 6;
			}
			// %SAKURADATA%	// 2007.06.06 ryoji
			else if( 0 == wmemicmp( L"SAKURADATA%", ps, 11 ) ){
				// iniのあるフォルダー
				GetInidir( szPath );
				nMetaLen = 10;
			}
			// メタ文字列っぽい
			else if( NULL != (pStr = wcschr( ps, L'%' ) )){
				nMetaLen = pStr - ps;
				if( nMetaLen < _MAX_PATH ){
					wmemcpy( szMeta, ps, nMetaLen );
					szMeta[nMetaLen] = L'\0';
				}
				else{
					*pd = L'\0';
					return false;
				}
#ifdef _USE_META_ALIAS
				// メタ文字列がエイリアス名なら書き換える
				const MetaAlias* pAlias;
				for( pAlias = &AliasList[0]; nMetaLen < pAlias->nLenth; pAlias++ )
					; // 読み飛ばす
				for( ; nMetaLen == pAlias->nLenth; pAlias++ ){
					if( 0 == wmemicmp( pAlias->szAlias, szMeta ) ){
						wcscpy( szMeta, pAlias->szOrig );
						break;
					}
				}
#endif
				// 直接レジストリで調べる
				szPath[0] = L'\0';
				bFolderPath = ReadRegistry( HKEY_CURRENT_USER,
					L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",
					szMeta, szPath, _countof( szPath ) );
				if( false == bFolderPath || L'\0' == szPath[0] ){
					bFolderPath = ReadRegistry( HKEY_LOCAL_MACHINE,
						L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",
						szMeta, szPath, _countof( szPath ) );
				}
				if( false == bFolderPath || L'\0' == szPath[0] ){
					pStr = _tgetenv( szMeta );
					// 環境変数
					if( NULL != pStr ){
						nPathLen = wcslen( pStr );
						if( nPathLen < _MAX_PATH ){
							wcscpy( szPath, pStr );
						}else{
							*pd = L'\0';
							return false;
						}
					}
					// 未定義のメタ文字列は 入力された%...%を，そのまま文字として処理する
					else if(  pd + ( nMetaLen + 2 ) < pd_end ){
						*pd = L'%';
						wmemcpy( &pd[1], ps, nMetaLen );
						pd[nMetaLen + 1] = L'%';
						pd += nMetaLen + 2;
						ps += nMetaLen;
						continue;
					}else{
						*pd = L'\0';
						return false;
					}
				}
			}else{
				// %...%の終わりの%がない とりあえず，%をコピー
				*pd = L'%';
				pd++;
				ps--; // 先にps++してしまったので戻す
				continue;
			}

			// ロングファイル名にする
			nPathLen = wcslen( szPath );
			LPWSTR pStr2 = szPath;
			if( nPathLen < _MAX_PATH && 0 != nPathLen ){
				if( FALSE != GetLongFileName( szPath, szMeta ) ){
					pStr2 = szMeta;
				}
			}

			// 最後のフォルダー区切り記号を削除する
			// [A:\]などのルートであっても削除
			for(nPathLen = 0; pStr2[nPathLen] != L'\0'; nPathLen++ ){
#ifdef _MBCS
				if( _IS_SJIS_1( (unsigned char)pStr2[nPathLen] ) && _IS_SJIS_2( (unsigned char)pStr2[nPathLen + 1] ) ){
					// SJIS読み飛ばし
					nPathLen++; // 2003/01/17 sui
				}else
#endif
				if( L'\\' == pStr2[nPathLen] && L'\0' == pStr2[nPathLen + 1] ){
					pStr2[nPathLen] = L'\0';
					break;
				}
			}

			if( pd + nPathLen < pd_end && 0 != nPathLen ){
				wmemcpy( pd, pStr2, nPathLen );
				pd += nPathLen;
				ps += nMetaLen;
			}else{
				*pd = L'\0';
				return false;
			}
		}else{
			// 最後の文字が%だった
			*pd = *ps;
			pd++;
		}
	}
	*pd = L'\0';
	return true;
}

/* static */ WCHAR CFileNameManager::GetAccessKeyByIndex(int index, bool bZeroOrigin)
{
	if( index < 0 ) return 0;
	int accKeyIndex = ((bZeroOrigin? index: index+1) % 36);
	WCHAR c = (WCHAR)((accKeyIndex < 10) ? (L'0' + accKeyIndex) : (L'A' + accKeyIndex - 10));
	return c;
}

static void GetAccessKeyLabelByIndex(WCHAR* pszLabel, bool bEspaceAmp, int index, bool bZeroOrigin)
{
	WCHAR c = CFileNameManager::GetAccessKeyByIndex(index, bZeroOrigin);
	if( c ){
		if( bEspaceAmp ){
			pszLabel[0] = L'&';
			pszLabel[1] = c;
			pszLabel[2] = L' ';
			pszLabel[3] = L'\0';
		}else{
			pszLabel[0] = c;
			pszLabel[1] = L' ';
			pszLabel[2] = L'\0';
		}
	}else{
		pszLabel[0] = L'\0';
	}
}

/*
	@param editInfo      ウィンドウ情報。NULで不明扱い
	@param index         いつも0originで指定。 -1で非表示
	@param bZeroOrigin   アクセスキーを0から振る
*/
bool CFileNameManager::GetMenuFullLabel(
	WCHAR* pszOutput, int nBuffSize, bool bEspaceAmp,
	const EditInfo* editInfo, int nId, bool bFavorite,
	int index, bool bAccKeyZeroOrigin, HDC hDC
){
	const EditInfo* pfi = editInfo;
	WCHAR szAccKey[4];
	int ret = 0;
	if( NULL == pfi ){
		GetAccessKeyLabelByIndex( szAccKey, bEspaceAmp, index, bAccKeyZeroOrigin );
		ret = auto_snprintf_s( pszOutput, nBuffSize, LS(STR_MENU_UNKOWN), szAccKey );
		return 0 < ret;
	}else if( pfi->m_bIsGrep ){
		
		GetAccessKeyLabelByIndex( szAccKey, bEspaceAmp, index, bAccKeyZeroOrigin );
		//pfi->m_szGrepKeyShort → cmemDes
		CNativeW	cmemDes;
		int nGrepKeyLen = wcslen(pfi->m_szGrepKey);
		const int GREPKEY_LIMIT_LEN = 64;
		// CSakuraEnvironment::ExpandParameter では 32文字制限
		// メニューは 64文字制限
		LimitStringLengthW( pfi->m_szGrepKey, nGrepKeyLen, GREPKEY_LIMIT_LEN, cmemDes );
		
		const WCHAR* pszKey;
		WCHAR szMenu2[GREPKEY_LIMIT_LEN*2*2+1]; // WCHAR=>ACHARで2倍、&で2倍
		if( bEspaceAmp ){
			dupamp( cmemDes.GetStringPtr(), szMenu2 );
			pszKey = szMenu2;
		}else{
			pszKey = cmemDes.GetStringPtr();
		}

		//szMenuを作る
		//	Jan. 19, 2002 genta
		//	&の重複処理を追加したため継続判定を若干変更
		//	20100729 ExpandParameterにあわせて、・・・を...に変更
		ret = auto_snprintf_s( pszOutput, nBuffSize, LS(STR_MENU_GREP),
			szAccKey, pszKey,
			( nGrepKeyLen > cmemDes.GetStringLength() ) ? L"...":L""
		);
	}else if( pfi->m_bIsDebug ){
		GetAccessKeyLabelByIndex( szAccKey, bEspaceAmp, index, bAccKeyZeroOrigin );
		ret = auto_snprintf_s( pszOutput, nBuffSize, LS(STR_MENU_OUTPUT), szAccKey );
	}else{
		return GetMenuFullLabel(pszOutput, nBuffSize, bEspaceAmp, pfi->m_szPath, nId, pfi->m_bIsModified, pfi->m_nCharCode, bFavorite,
			 index, bAccKeyZeroOrigin, hDC);
	}
	return 0 < ret;
}

bool CFileNameManager::GetMenuFullLabel(
	WCHAR* pszOutput, int nBuffSize, bool bEspaceAmp,
	const WCHAR* pszFile, int nId, bool bModified, ECodeType nCharCode, bool bFavorite,
	int index, bool bAccKeyZeroOrigin, HDC hDC
){
	WCHAR szAccKey[4];
	WCHAR szFileName[_MAX_PATH];
	WCHAR szMenu2[_MAX_PATH * 2];
	const WCHAR* pszName;

	GetAccessKeyLabelByIndex( szAccKey, bEspaceAmp, index, bAccKeyZeroOrigin );
	if( pszFile[0] ){
		this->GetTransformFileNameFast( pszFile, szFileName, _MAX_PATH, hDC );

		// szFileName → szMenu2
		//	Jan. 19, 2002 genta
		//	メニュー文字列の&を考慮
		if( bEspaceAmp ){
			dupamp( szFileName, szMenu2 );
			pszName = szMenu2;
		}else{
			pszName = szFileName;
		}
	}else{
		if( nId == -1 ){
			wsprintf( szFileName, LS( STR_NO_TITLE1 ));
		}else{
			wsprintf( szFileName, L"%s%d", LS( STR_NO_TITLE1 ), nId);
		}
		pszName = szFileName;
	}
	const WCHAR* pszCharset = L"";
	WCHAR szCodePageName[100];
	if( IsValidCodeTypeExceptSJIS(nCharCode)){
		pszCharset = CCodeTypeName(nCharCode).Bracket();
	}else if( IsValidCodeOrCPTypeExceptSJIS(nCharCode) ){
		CCodePage::GetNameBracket(szCodePageName, nCharCode);
		pszCharset = szCodePageName;
	}
	
	int ret = auto_snprintf_s( pszOutput, nBuffSize, L"%s%s%s%s%s",
		szAccKey, (bFavorite ? L"★ " : L""), pszName,
		(bModified ? L" *":L""), pszCharset
	);
	return 0 < ret;
}
