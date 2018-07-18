/*
	ドキュメント種別の管理

	2008.01～03 kobake 作成
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
#ifndef SAKURA_CDOCTYPE_BB51F346_E9F1_42DD_8B28_2F5BAFCE7CE09_H_
#define SAKURA_CDOCTYPE_BB51F346_E9F1_42DD_8B28_2F5BAFCE7CE09_H_

#include "types/CType.h" // CTypeConfig
#include "env/CDocTypeManager.h"

class CDocType{
public:
	//生成と破棄
	CDocType(CEditDoc* pcDoc);
	
	//ロック機能	//	Nov. 29, 2000 genta 設定の一時変更時に拡張子による強制的な設定変更を無効にする
	void LockDocumentType(){ m_nSettingTypeLocked = true; }
	void UnlockDocumentType(){ m_nSettingTypeLocked = false; }
	bool GetDocumentLockState(){ return m_nSettingTypeLocked; }
	
	// 文書種別の設定と取得		// Nov. 23, 2000 genta
	void SetDocumentType(CTypeConfig type, bool force, bool bTypeOnly = false);	//!< 文書種別の設定
	void SetDocumentTypeIdx( int id = -1, bool force = false);
	CTypeConfig GetDocumentType() const					//!< 文書種別の取得
	{
		return m_nSettingType;
	}
	const STypeConfig& GetDocumentAttribute() const						//!< 文書種別の詳細情報
	{
		return m_typeConfig;
	}
	STypeConfig& GetDocumentAttributeWrite()						//!< 文書種別の詳細情報
	{
		return m_typeConfig;
	}

	// 拡張機能
	void SetDocumentIcon();	//アイコンの設定	//Sep. 10, 2002 genta

private:
	CEditDoc*				m_pcDocRef;
	CTypeConfig				m_nSettingType;
	STypeConfig				m_typeConfig;
	bool					m_nSettingTypeLocked;		//!< 文書種別の一時設定状態
};

#endif /* SAKURA_CDOCTYPE_BB51F346_E9F1_42DD_8B28_2F5BAFCE7CE09_H_ */
/*[EOF]*/
