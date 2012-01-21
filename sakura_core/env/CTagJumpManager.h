/*
	2008.05.18 kobake CShareData ���番��
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
#ifndef SAKURA_CTAGJUMPMANAGER_A826CC13_50FF_44A9_813D_CC5B918410A7_H_
#define SAKURA_CTAGJUMPMANAGER_A826CC13_50FF_44A9_813D_CC5B918410A7_H_

// �v��s��`
// #define DLLSHAREDATA.h


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

#endif /* SAKURA_CTAGJUMPMANAGER_A826CC13_50FF_44A9_813D_CC5B918410A7_H_ */
/*[EOF]*/
