/*
	�h�L�������g��ʂ̊Ǘ�

	2008.01�`03 kobake �쐬
*/
#pragma once


class CDocType{
public:
	//�����Ɣj��
	CDocType(CEditDoc* pcDoc);
	
	//���b�N�@�\	//	Nov. 29, 2000 genta �ݒ�̈ꎞ�ύX���Ɋg���q�ɂ�鋭���I�Ȑݒ�ύX�𖳌��ɂ���
	void LockDocumentType(){ m_nSettingTypeLocked = true; }
	void UnlockDocumentType(){ m_nSettingTypeLocked = false; }
	bool GetDocumentLockState(){ return m_nSettingTypeLocked; }
	
	// ������ʂ̐ݒ�Ǝ擾		// Nov. 23, 2000 genta
	void SetDocumentType(CTypeConfig type, bool force);	//!< ������ʂ̐ݒ�
	CTypeConfig GetDocumentType() const					//!< ������ʂ̎擾
	{
		return m_nSettingType;
	}
	STypeConfig& GetDocumentAttribute() const						//!< ������ʂ̏ڍ׏��
	{
		return GetDllShareData().GetTypeSetting(m_nSettingType);
	}

	// �g���@�\
	void SetDocumentIcon();	//�A�C�R���̐ݒ�	//Sep. 10, 2002 genta

private:
	CEditDoc*		m_pcDocRef;
	CTypeConfig	m_nSettingType;
	bool			m_nSettingTypeLocked;		//!< ������ʂ̈ꎞ�ݒ���
};
