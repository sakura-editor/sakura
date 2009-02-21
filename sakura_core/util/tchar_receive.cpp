#include "stdafx.h"
#include "tchar_receive.h"
using namespace std;

//TcharReceiver����
#ifdef _UNICODE
	//UNICODE�r���h�ŁAWCHAR���󂯎��
	template <> TCHAR* TcharReceiver<WCHAR>::GetBufferPointer(){ return m_pReceiver; }
	template <> void   TcharReceiver<WCHAR>::Apply(){} //�������Ȃ�

	//UNICODE�r���h�ŁAACHAR���󂯎��
	template <> TCHAR* TcharReceiver<ACHAR>::GetBufferPointer(){ return (m_pBuff = new TCHAR[m_nReceiverCount]); }
	template <> void   TcharReceiver<ACHAR>::Apply(){ _tcstombs(m_pReceiver, m_pBuff, m_nReceiverCount); delete []m_pBuff; }

#else
	//ANSI�r���h�ŁAWCHAR���󂯎��
	template <> TCHAR* TcharReceiver<WCHAR>::GetBufferPointer(){ return (m_pBuff = new TCHAR[m_nReceiverCount]); }
	template <> void   TcharReceiver<WCHAR>::Apply(){ _tcstowcs(m_pReceiver, m_pBuff, m_nReceiverCount); delete []m_pBuff; }

	//ANSI�r���h�ŁAACHAR���󂯎��
	template <> TCHAR* TcharReceiver<ACHAR>::GetBufferPointer(){ return m_pReceiver; }
	template <> void   TcharReceiver<ACHAR>::Apply(){} //�������Ȃ�

#endif

//�C���X�^���X��
template class TcharReceiver<WCHAR>;
template class TcharReceiver<ACHAR>;


