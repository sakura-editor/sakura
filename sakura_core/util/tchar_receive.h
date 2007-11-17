#pragma once

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
*/
template <class RECEIVE_CHAR_TYPE>
class TcharReceiver{
public:
	TcharReceiver(RECEIVE_CHAR_TYPE* pReceiver, size_t nReceiverCount)	//!< �󂯎��o�b�t�@���w��B
	: m_pReceiver(pReceiver), m_nReceiverCount(nReceiverCount) { }
	operator TCHAR* (){ return GetBufferPointer(); }
	~TcharReceiver(){ Apply(); }
protected:
	TCHAR* GetBufferPointer();	//!< �ꎞ�o�b�t�@��񋟁B�o�b�t�@�����͒Z���̂Œ��ӁB
	void Apply();				//!< �ꎞ�o�b�t�@����A���ۂ̎󂯎��o�b�t�@�փf�[�^���R�s�[�B
private:
	RECEIVE_CHAR_TYPE*	m_pReceiver;
	size_t				m_nReceiverCount;
};
