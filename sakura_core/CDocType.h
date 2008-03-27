#pragma once

class CDocType{
public:
	CDocType(CEditDoc* pcDoc)
	: m_pcDocRef(pcDoc)
	, m_nSettingTypeLocked( false )	//	�ݒ�l�ύX�\�t���O
	, m_nSettingType( 0 )	// Sep. 11, 2002 genta
	{
	}

	//	2002.10.13 Moca
	void SetDocumentIcon();	// Sep. 10, 2002 genta

	//	Nov. 29, 2000 From Here	genta
	//	�ݒ�̈ꎞ�ύX���Ɋg���q�ɂ�鋭���I�Ȑݒ�ύX�𖳌��ɂ���
	void LockDocumentType(void){ m_nSettingTypeLocked = true; }
	void UnlockDocumentType(void){ m_nSettingTypeLocked = false; }
	bool GetDocumentLockState(void){ return m_nSettingTypeLocked; }
	//	Nov. 29, 2000 To Here

	//	Nov. 23, 2000 From Here	genta
	//	������ʏ��̐ݒ�C�擾Interface
	void SetDocumentType(CDocumentType type, bool force);	//	������ʂ̐ݒ�
	CDocumentType GetDocumentType(void) const	//!<	������ʂ̓ǂݏo��
	{
		return m_nSettingType;
	}
	Types& GetDocumentAttribute(void) const	//!<	�ݒ肳�ꂽ�������ւ̎Q�Ƃ�Ԃ�
	{
		return GetDllShareData().GetTypeSetting(m_nSettingType);
	}
	//	Nov. 23, 2000 To Here

private:
	CEditDoc*		m_pcDocRef;
	CDocumentType	m_nSettingType;
	bool			m_nSettingTypeLocked;		//!< ������ʂ̈ꎞ�ݒ���
};
