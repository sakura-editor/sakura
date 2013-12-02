/*
	2008.05.18 kobake CShareData から分離
*/
/*
	Copyright (C) 2008, kobake

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
#ifndef SAKURA_CDOCTYPEMANAGER_48534203_AB9B_4C1A_AF78_B76CA88CA0019_H_
#define SAKURA_CDOCTYPEMANAGER_48534203_AB9B_4C1A_AF78_B76CA88CA0019_H_

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
	CTypeConfig GetDocumentTypeOfId( int id );

	bool GetTypeConfig(CTypeConfig cDocumentType, STypeConfig& type);
	bool SetTypeConfig(CTypeConfig cDocumentType, const STypeConfig& type);
	bool GetTypeConfigMini(CTypeConfig cDocumentType, const STypeConfigMini** type);
	bool AddTypeConfig(CTypeConfig cDocumentType);
	bool DelTypeConfig(CTypeConfig cDocumentType);

private:
	DLLSHAREDATA* m_pShareData;
};

#endif /* SAKURA_CDOCTYPEMANAGER_48534203_AB9B_4C1A_AF78_B76CA88CA0019_H_ */
/*[EOF]*/
