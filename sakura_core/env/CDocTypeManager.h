/*
	2008.05.18 kobake CShareData から分離
*/

#pragma once

#include "DLLSHAREDATA.h"

//! ドキュメントタイプ管理
class CDocTypeManager{
public:
	CDocTypeManager()
	{
		m_pShareData = &GetDllShareData();
	}
	CTypeConfig GetDocumentTypeOfPath( const TCHAR* pszFilePath );	/* ファイルパスを渡して、ドキュメントタイプ（数値）を取得する */
	CTypeConfig GetDocumentTypeOfExt( const TCHAR* pszExt );		/* 拡張子を渡して、ドキュメントタイプ（数値）を取得する */

	STypeConfig& GetTypeSetting(CTypeConfig cDocumentType);
private:
	DLLSHAREDATA* m_pShareData;
};
