/*
	Copyright (C) 2007, kobake

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
#ifndef SAKURA_TCHAR_RECEIVE_173C52CE_CAC9_4ED5_8399_EFEF8CC7DBD2_H_
#define SAKURA_TCHAR_RECEIVE_173C52CE_CAC9_4ED5_8399_EFEF8CC7DBD2_H_

/*!
	���ۂ̃f�[�^��Ɋւ�炸�ATCHAR[]�^�̎󂯎��o�b�t�@��񋟂���N���X�B
	�g����������Ȃ̂Œ��ӁB

	��:
	{
		wchar_t buf[256];
		GetWindowText(hwnd,TcharReceiver(buf));
	}

	���̃R�[�h�́AANSI�r���h�AUNICODE�r���h�A�Ƃ��ɒʂ�܂��B
	ANSI�r���h���́Achar��wchar_t�ϊ����������邽�߁A�������ׂ�������܂� (���̗�̏ꍇ)�B
	UNICODE�r���h���́A���ׂ� TcharReceiver ���g��Ȃ��ꍇ�ƂقƂ�Ǖς��܂��� (���̗�̏ꍇ)�B

	���쌴���̓\�[�X���Q�Ƃ̂��ƁB
	operator TCHAR* �� GetWindowText �ɓn���|�C���^��񋟂��A
	~TcharReceiver �ɂ����āA�K�v�ł���� (�r���h��Ǝ󂯎��^���قȂ��)�A
	TCHAR��wchar_t�ϊ����������܂��B

	2007.10.27 kobake �쐬
	2009.02.21 ryoji		�W��������ȊO�������ꍇ�iUNICODE�r���h��ACHAR�AANSI�r���h��WCHAR�j��
							512�����̃T�C�Y�����t���ÓI�o�b�t�@���g�p���Ă����̂��A
							�T�C�Y�����̖������I�o�b�t�@���g���悤�ɕύX�B�i���ׂ͂ǂ݂̂��ϊ��̂ق��ɂ�������j
*/
template <class RECEIVE_CHAR_TYPE>
class TcharReceiver{
public:
	TcharReceiver(RECEIVE_CHAR_TYPE* pReceiver, size_t nReceiverCount)	//!< �󂯎��o�b�t�@���w��B
	: m_pReceiver(pReceiver), m_nReceiverCount(nReceiverCount), m_pBuff(NULL) { }
	operator TCHAR* (){ return GetBufferPointer(); }
	~TcharReceiver(){ Apply(); }
protected:
	TCHAR* GetBufferPointer();	//!< �ꎞ�o�b�t�@��񋟁B�o�b�t�@�����͒Z���̂Œ��ӁB
	void Apply();				//!< �ꎞ�o�b�t�@����A���ۂ̎󂯎��o�b�t�@�փf�[�^���R�s�[�B
private:
	RECEIVE_CHAR_TYPE*	m_pReceiver;
	size_t				m_nReceiverCount;
	TCHAR*				m_pBuff;
};

#endif /* SAKURA_TCHAR_RECEIVE_173C52CE_CAC9_4ED5_8399_EFEF8CC7DBD2_H_ */
/*[EOF]*/
