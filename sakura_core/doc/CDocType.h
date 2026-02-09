/*! @file */
/*
	ドキュメント種別の管理

	2008.01～03 kobake 作成
*/
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CDOCTYPE_7009DED0_A784_49F3_B8C0_9A2559A9DAFA_H_
#define SAKURA_CDOCTYPE_7009DED0_A784_49F3_B8C0_9A2559A9DAFA_H_
#pragma once

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
#endif /* SAKURA_CDOCTYPE_7009DED0_A784_49F3_B8C0_9A2559A9DAFA_H_ */
