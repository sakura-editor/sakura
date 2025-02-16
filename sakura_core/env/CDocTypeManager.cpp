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
#include "CDocTypeManager.h"
#include "_main/CMutex.h"
#include "CFileExt.h"
#include <Shlwapi.h>	// PathMatchSpec
#include "apiwrap/StdApi.h"
#include "env/DLLSHAREDATA.h"
#include "config/system_constants.h"

const WCHAR* CDocTypeManager::m_typeExtSeps = L" ;,";	// タイプ別拡張子 区切り文字
const WCHAR* CDocTypeManager::m_typeExtWildcards = L"*?";	// タイプ別拡張子 ワイルドカード

static CMutex g_cDocTypeMutex( FALSE, GSTR_MUTEX_SAKURA_DOCTYPE );

/*!
	ファイル名から、ドキュメントタイプ（数値）を取得する
	
	@param pszFilePath [in] ファイル名
	
	拡張子を切り出して GetDocumentTypeOfExt に渡すだけ．
	@date 2014.12.06 syat ワイルドカード対応。２重拡張子対応をやめる
*/
CTypeConfig CDocTypeManager::GetDocumentTypeOfPath( const WCHAR* pszFilePath )
{
	int		i;

	// ファイル名を抽出
	const WCHAR* pszFileName = pszFilePath;
	const WCHAR* pszSep = wcsrchr(pszFilePath, L'\\');
	if (pszSep) {
		pszFileName = pszSep + 1;
	}

	for (i = 0; i < m_pShareData->m_nTypesCount; ++i){
		const STypeConfigMini* mini = NULL;
		if( GetTypeConfigMini( CTypeConfig(i), &mini )
			&& IsFileNameMatch(mini->m_szTypeExts, pszFileName)) {
			return CTypeConfig(i);	//	番号
		}
	}
	return CTypeConfig(0);
}

/*!
	拡張子から、ドキュメントタイプ（数値）を取得する
	
	@param pszExt [in] 拡張子 (先頭の.は含まない)
	
	指定された拡張子の属する文書タイプ番号を返す．
	とりあえず今のところはタイプは拡張子のみに依存すると仮定している．
	ファイル全体の形式に対応させるときは，また考え直す．
	@date 2012.10.22 Moca ２重拡張子, 拡張子なしに対応
	@date 2014.12.06 syat GetDocumentTypeOfPathに統合
*/
CTypeConfig CDocTypeManager::GetDocumentTypeOfExt( const WCHAR* pszExt )
{
	return GetDocumentTypeOfPath(pszExt);
}

CTypeConfig CDocTypeManager::GetDocumentTypeOfId( int id )
{
	int		i;

	for( i = 0; i < m_pShareData->m_nTypesCount; ++i ){
		const STypeConfigMini* mini = NULL;
		if( GetTypeConfigMini( CTypeConfig(i), &mini )
			&& mini->m_id == id ){
			return CTypeConfig(i);
		}
	}
	return CTypeConfig(-1);	//	ハズレ
}

bool CDocTypeManager::GetTypeConfig(CTypeConfig cDocumentType, STypeConfig& type)
{
	int n = cDocumentType.GetIndex();
	if( 0 <= n && n < m_pShareData->m_nTypesCount ){
		if( 0 == n ){
			type = m_pShareData->m_TypeBasis;
			return true;
		}else{
			LockGuard<CMutex> guard( g_cDocTypeMutex );
			 if( SendMessageAny( m_pShareData->m_sHandles.m_hwndTray, MYWM_GET_TYPESETTING, (WPARAM)n, 0 ) ){
				type = m_pShareData->m_sWorkBuffer.m_TypeConfig;
				return true;
			}
		}
	}
	return false;
}

bool CDocTypeManager::SetTypeConfig(CTypeConfig cDocumentType, const STypeConfig& type)
{
	int n = cDocumentType.GetIndex();
	if( 0 <= n && n < m_pShareData->m_nTypesCount ){
		LockGuard<CMutex> guard( g_cDocTypeMutex );
		m_pShareData->m_sWorkBuffer.m_TypeConfig = type;
		if( SendMessageAny( m_pShareData->m_sHandles.m_hwndTray, MYWM_SET_TYPESETTING, (WPARAM)n, 0 ) ){
			return true;
		}
	}
	return false;
}

/*! タイプ別設定(mini)取得
	@param cDocumentType [in] ドキュメントタイプ
	@param type [out] タイプ別設定(mini)

	@retval true  正常
	@retval false 異常
*/
[[nodiscard]] bool CDocTypeManager::GetTypeConfigMini(CTypeConfig cDocumentType, const STypeConfigMini** type)
{
	int n = cDocumentType.GetIndex();
	if( 0 <= n && n < m_pShareData->m_nTypesCount ){
		*type = &m_pShareData->m_TypeMini[n];
		return true;
	}
	return false;
}

bool CDocTypeManager::AddTypeConfig(CTypeConfig cDocumentType)
{
	LockGuard<CMutex> guard( g_cDocTypeMutex );
	return FALSE != SendMessageAny( m_pShareData->m_sHandles.m_hwndTray, MYWM_ADD_TYPESETTING, (WPARAM)cDocumentType.GetIndex(), 0 );
}

bool CDocTypeManager::DelTypeConfig(CTypeConfig cDocumentType)
{
	LockGuard<CMutex> guard( g_cDocTypeMutex );
	return FALSE != SendMessageAny( m_pShareData->m_sHandles.m_hwndTray, MYWM_DEL_TYPESETTING, (WPARAM)cDocumentType.GetIndex(), 0 );
}

/*!
	タイプ別拡張子にファイル名がマッチするか
	
	@param pszTypeExts [in] タイプ別拡張子（ワイルドカードを含む）
	@param pszFileName [in] ファイル名
*/
bool CDocTypeManager::IsFileNameMatch(const WCHAR* pszTypeExts, const WCHAR* pszFileName)
{
	WCHAR szWork[MAX_TYPES_EXTS];

	wcsncpy(szWork, pszTypeExts, _countof(szWork));
	szWork[_countof(szWork) - 1] = '\0';
	WCHAR* token = _wcstok(szWork, m_typeExtSeps);
	while (token) {
		if (wcspbrk(token, m_typeExtWildcards) == NULL) {
			if (_wcsicmp(token, pszFileName) == 0) {
				return true;
			}
			const WCHAR* pszExt = wcsrchr(pszFileName, L'.');
			if (pszExt != NULL && _wcsicmp(token, pszExt + 1) == 0) {
				return true;
			}
		} else {
			if (PathMatchSpec(pszFileName, token) == TRUE) {
				return true;
			}
		}
		token = _wcstok(NULL, m_typeExtSeps);
	}
	return false;
}

/*!
	タイプ別拡張子の先頭拡張子を取得する
	
	@param pszTypeExts [in] タイプ別拡張子（ワイルドカードを含む）
	@param szFirstExt  [out] 先頭拡張子
	@param nBuffSize   [in] 先頭拡張子のバッファサイズ
*/
void CDocTypeManager::GetFirstExt(const WCHAR* pszTypeExts, WCHAR szFirstExt[], int nBuffSize)
{
	WCHAR szWork[MAX_TYPES_EXTS];

	wcsncpy(szWork, pszTypeExts, _countof(szWork));
	szWork[_countof(szWork) - 1] = '\0';
	WCHAR* token = _wcstok(szWork, m_typeExtSeps);
	while (token) {
		if (wcspbrk(token, m_typeExtWildcards) == NULL) {
			wcsncpy(szFirstExt, token, nBuffSize);
			szFirstExt[nBuffSize - 1] = L'\0';
			return;
		}
	}
	szFirstExt[0] = L'\0';
	return;
}

/*! タイプ別設定の拡張子リストをダイアログ用リストに変換する
	@param pszSrcExt [in]  拡張子リスト 例「.c cpp;.h」(ドットはオプション)
	@param szExt [in] リストの先頭にする拡張子 例「.h」(ドット必須)
	@return 拡張子リスト 例「*.h;*.c;*.cpp」

	@date 2014.12.06 syat CFileExtから移動
*/
std::wstring CDocTypeManager::ConvertTypesExtToDlgExt(const WCHAR *pszSrcExt, const WCHAR* szExt)
{
	//	2003.08.14 MIK NULLじゃなくてfalse
	if (pszSrcExt == nullptr) return L"";

	std::wstring destExt;
	if (szExt != NULL && szExt[0] != L'\0') {
		// ファイルパスがあり、拡張子ありの場合、トップに指定
		destExt.assign(L"*");
		destExt.append(szExt);
	}

	std::wstring strSrcTokens = pszSrcExt;
	WCHAR* context = nullptr;
	WCHAR* token = wcstok_s(strSrcTokens.data(), m_typeExtSeps, &context);
	while( token )
	{
		if (szExt == NULL || szExt[0] == L'\0' || wmemicmp(token, szExt + 1) != 0) {
			if (0 < destExt.length()) {
				destExt.append(L";");
			}
			// 拡張子指定なし、またはマッチした拡張子でない
			if (wcspbrk(token, m_typeExtWildcards) == NULL) {
				if (L'.' == *token) {
					destExt.append(L"*");
				}
				else {
					destExt.append(L"*.");
				}
			}
			destExt.append(token);
		}
		token = wcstok_s(nullptr, m_typeExtSeps, &context);
	}
	return destExt;
}
