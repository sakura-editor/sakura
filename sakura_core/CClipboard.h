#pragma once

//!�T�N���G�f�B�^�p�N���b�v�{�[�h�N���X�B��X�͂��̒��őS�ẴN���b�v�{�[�hAPI���Ă΂������B
class CClipboard{
public:
	//�R���X�g���N�^�E�f�X�g���N�^
	CClipboard(HWND hwnd); //!< �R���X�g���N�^���ŃN���b�v�{�[�h���J�����
	virtual ~CClipboard(); //!< �f�X�g���N�^����Close���Ă΂��

	//�C���^�[�t�F�[�X
	void Empty(); //!< �N���b�v�{�[�h����ɂ���
	void Close(); //!< �N���b�v�{�[�h�����
	bool SetText(const wchar_t* pData, int nDataLen, bool bColmnSelect, bool bLineSelect);   //!< �e�L�X�g��ݒ肷��
	bool GetText(CNativeW* cmemBuf, bool* pbColmnSelect, bool* pbLineSelect); //!< �e�L�X�g���擾����

	//���Z�q
	operator bool() const{ return m_bOpenResult!=FALSE; } //!< �N���b�v�{�[�h���J�����Ȃ�true

private:
	HWND m_hwnd;
	BOOL m_bOpenResult;




	// -- -- static�C���^�[�t�F�[�X -- -- //
public:
	static bool HasValidData();    //!< �N���b�v�{�[�h���ɁA�T�N���G�f�B�^�ň�����f�[�^�������true
	static UINT GetSakuraFormat(); //!< �T�N���G�f�B�^�Ǝ��̃N���b�v�{�[�h�f�[�^�`��
};
