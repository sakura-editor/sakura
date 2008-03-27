#pragma once

class CDocType{
public:
	CDocType(CEditDoc* pcDoc)
	: m_pcDocRef(pcDoc)
	, m_nSettingTypeLocked( false )	//	設定値変更可能フラグ
	, m_nSettingType( 0 )	// Sep. 11, 2002 genta
	{
	}

	//	2002.10.13 Moca
	void SetDocumentIcon();	// Sep. 10, 2002 genta

	//	Nov. 29, 2000 From Here	genta
	//	設定の一時変更時に拡張子による強制的な設定変更を無効にする
	void LockDocumentType(void){ m_nSettingTypeLocked = true; }
	void UnlockDocumentType(void){ m_nSettingTypeLocked = false; }
	bool GetDocumentLockState(void){ return m_nSettingTypeLocked; }
	//	Nov. 29, 2000 To Here

	//	Nov. 23, 2000 From Here	genta
	//	文書種別情報の設定，取得Interface
	void SetDocumentType(CDocumentType type, bool force);	//	文書種別の設定
	CDocumentType GetDocumentType(void) const	//!<	文書種別の読み出し
	{
		return m_nSettingType;
	}
	Types& GetDocumentAttribute(void) const	//!<	設定された文書情報への参照を返す
	{
		return GetDllShareData().GetTypeSetting(m_nSettingType);
	}
	//	Nov. 23, 2000 To Here

private:
	CEditDoc*		m_pcDocRef;
	CDocumentType	m_nSettingType;
	bool			m_nSettingTypeLocked;		//!< 文書種別の一時設定状態
};
