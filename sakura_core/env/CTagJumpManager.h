/*
	2008.05.18 kobake CShareData ���番��
*/

#pragma once

// 2004/06/21 novice �^�O�W�����v�@�\�ǉ�
//! �^�O�W�����v���
struct TagJump {
	HWND		hwndReferer;				//!< �Q�ƌ��E�B���h�E
	CLogicPoint	point;						//!< ���C��, �J����
};


//���L���������\����
//2004/06/21 �^�O�W�����v�@�\�ǉ�
//2005.04.03 MIK �L�[���[�h�w��^�O�W�����v
struct SShare_TagJump{
	//�^
	typedef StaticVector<
		StaticString<WCHAR, _MAX_PATH>,
		MAX_TAGJUMP_KEYWORD
	>					ATagJumpKeywords;

	//�f�[�^
	int					m_TagJumpNum;					//!< �^�O�W�����v���̗L���f�[�^��
	int					m_TagJumpTop;					//!< �X�^�b�N�̈�ԏ�̈ʒu
	TagJump				m_TagJump[MAX_TAGJUMPNUM];		//!< �^�O�W�����v���
	ATagJumpKeywords	m_aTagJumpKeywords;
	BOOL				m_bTagJumpICase;				//!< �啶���������𓯈ꎋ
	BOOL				m_bTagJumpAnyWhere;				//!< ������̓r���Ƀ}�b�`
};


class CTagJumpManager{
public:
	CTagJumpManager()
	{
		m_pShareData = &GetDllShareData();
	}
	//�^�O�W�����v�֘A	// 2004/06/21 novice �^�O�W�����v�@�\�ǉ�
	void PushTagJump(const TagJump *);		//!< �^�O�W�����v���̕ۑ�
	bool PopTagJump(TagJump *);				//!< �^�O�W�����v���̎Q��
private:
	DLLSHAREDATA* m_pShareData;
};
