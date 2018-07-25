#include "StdAfx.h"
#include "tchar_receive.h"
using namespace std;

//TcharReceiver実装
#ifdef _UNICODE
	//UNICODEビルドで、WCHARを受け取る
	template <> TCHAR* TcharReceiver<WCHAR>::GetBufferPointer(){ return m_pReceiver; }
	template <> void   TcharReceiver<WCHAR>::Apply(){} //何もしない

	//UNICODEビルドで、ACHARを受け取る
	template <> TCHAR* TcharReceiver<ACHAR>::GetBufferPointer(){ return (m_pBuff = new TCHAR[m_nReceiverCount]); }
	template <> void   TcharReceiver<ACHAR>::Apply(){ _tcstombs(m_pReceiver, m_pBuff, m_nReceiverCount); delete []m_pBuff; }

#else
	//ANSIビルドで、WCHARを受け取る
	template <> TCHAR* TcharReceiver<WCHAR>::GetBufferPointer(){ return (m_pBuff = new TCHAR[m_nReceiverCount]); }
	template <> void   TcharReceiver<WCHAR>::Apply(){ _tcstowcs(m_pReceiver, m_pBuff, m_nReceiverCount); delete []m_pBuff; }

	//ANSIビルドで、ACHARを受け取る
	template <> TCHAR* TcharReceiver<ACHAR>::GetBufferPointer(){ return m_pReceiver; }
	template <> void   TcharReceiver<ACHAR>::Apply(){} //何もしない

#endif

//インスタンス化
template class TcharReceiver<WCHAR>;
template class TcharReceiver<ACHAR>;


