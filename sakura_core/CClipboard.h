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
#ifndef SAKURA_CCLIPBOARD_25819BE6_8087_45F7_840E_161DE12E736AR_H_
#define SAKURA_CCLIPBOARD_25819BE6_8087_45F7_840E_161DE12E736AR_H_

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
	static int GetDataType();      //!< �N���b�v�{�[�h�f�[�^�`��(CF_UNICODETEXT��)�̎擾
};

#endif /* SAKURA_CCLIPBOARD_25819BE6_8087_45F7_840E_161DE12E736AR_H_ */
/*[EOF]*/
