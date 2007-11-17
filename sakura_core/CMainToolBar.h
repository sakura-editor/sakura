#pragma once

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
