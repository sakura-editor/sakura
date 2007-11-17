#include "stdafx.h"
#include "tchar_receive.h"
using namespace std;

//�ꎞ�o�b�t�@
static TCHAR static_buf[512];

//TcharReceiver����
#ifdef _UNICODE
	//UNICODE�r���h�ŁAWCHAR���󂯎��
	template <> TCHAR* TcharReceiver<WCHAR>::GetBufferPointer(){ return m_pReceiver; }
	template <> void   TcharReceiver<WCHAR>::Apply(){} //�������Ȃ�

	//UNICODE�r���h�ŁAACHAR���󂯎��
	template <> TCHAR* TcharReceiver<ACHAR>::GetBufferPointer(){ return static_buf; }
	template <> void   TcharReceiver<ACHAR>::Apply(){ _tcstombs(m_pReceiver, static_buf, m_nReceiverCount); }

#else
	//ANSI�r���h�ŁAWCHAR���󂯎��
	template <> TCHAR* TcharReceiver<WCHAR>::GetBufferPointer(){ return static_buf; }
	template <> void   TcharReceiver<WCHAR>::Apply(){ _tcstowcs(m_pReceiver, static_buf, m_nReceiverCount); } //�������Ȃ�

	//ANSI�r���h�ŁAACHAR���󂯎��
	template <> TCHAR* TcharReceiver<ACHAR>::GetBufferPointer(){ return m_pReceiver; }
	template <> void   TcharReceiver<ACHAR>::Apply(){} //�������Ȃ�

#endif

//�C���X�^���X��
template class TcharReceiver<WCHAR>;
template class TcharReceiver<ACHAR>;


