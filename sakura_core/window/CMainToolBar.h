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
#ifndef SAKURA_CMAINTOOLBAR_F8D148A4_02B1_42E7_8B00_B51B3DB49E749_H_
#define SAKURA_CMAINTOOLBAR_F8D148A4_02B1_42E7_8B00_B51B3DB49E749_H_

class CEditWnd;

class CMainToolBar{
public:
	CMainToolBar(CEditWnd* pOwner);

	//�쐬�E�j��
	void CreateToolBar( void );		//!< �c�[���o�[�쐬
	void DestroyToolBar( void );	//!< �c�[���o�[�j��

	//���b�Z�[�W
	bool EatMessage(MSG* msg);		//!< ���b�Z�[�W�����B�Ȃ񂩏��������Ȃ� true ��Ԃ��B
	void ProcSearchBox( MSG* );		//!< �����R���{�{�b�N�X�̃��b�Z�[�W����

	//�C�x���g
	void OnToolbarTimer( void );	//!< �^�C�}�[�̏��� 20060128 aroka
	void UpdateToolbar( void );		//!< �c�[���o�[�̕\�����X�V����		// 2008.09.23 nasukoji

	//�`��
	LPARAM ToolBarOwnerDraw( LPNMCUSTOMDRAW pnmh );

	//���L�f�[�^�Ƃ̓���
	void AcceptSharedSearchKey();

	//�擾
	HWND GetToolbarHwnd() const{ return m_hwndToolBar; }
	HWND GetRebarHwnd() const{ return m_hwndReBar; }
	HWND GetSearchHwnd() const{ return m_hwndSearchBox; }
	int GetSearchKey(wchar_t* pBuf, int nBufCount); //!< �����L�[���擾�B�߂�l�͌����L�[�̕������B

	//����
	void SetFocusSearchBox( void ) const;		/* �c�[���o�[�����{�b�N�X�փt�H�[�J�X���ړ� */	// 2006.06.04 yukihane

private:
	CEditWnd*	m_pOwner;
    HWND		m_hwndToolBar;

	//�q�E�B���h�E
    HWND		m_hwndReBar;		//!< Rebar �E�B���h�E	//@@@ 2006.06.17 ryoji
	HWND		m_hwndSearchBox;	//!< �����R���{�{�b�N�X

	//�t�H���g
	HFONT		m_fontSearchBox;	//!< �����R���{�{�b�N�X�̃t�H���g
};

#endif /* SAKURA_CMAINTOOLBAR_F8D148A4_02B1_42E7_8B00_B51B3DB49E749_H_ */
/*[EOF]*/
