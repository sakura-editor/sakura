#include "stdafx.h"
#include "tchar_receive.h"
using namespace std;

//一時バッファ
static TCHAR static_buf[512];

//TcharReceiver実装
#ifdef _UNICODE
	//UNICODEビルドで、WCHARを受け取る
	template <> TCHAR* TcharReceiver<WCHAR>::GetBufferPointer(){ return m_pReceiver; }
	template <> void   TcharReceiver<WCHAR>::Apply(){} //何もしない

	//UNICODEビルドで、ACHARを受け取る
	template <> TCHAR* TcharReceiver<ACHAR>::GetBufferPointer(){ return static_buf; }
	template <> void   TcharReceiver<ACHAR>::Apply(){ _tcstombs(m_pReceiver, static_buf, m_nReceiverCount); }

#else
	//ANSIビルドで、WCHARを受け取る
	template <> TCHAR* TcharReceiver<WCHAR>::GetBufferPointer(){ return static_buf; }
	template <> void   TcharReceiver<WCHAR>::Apply(){ _tcstowcs(m_pReceiver, static_buf, m_nReceiverCount); } //何もしない

	//ANSIビルドで、ACHARを受け取る
	template <> TCHAR* TcharReceiver<ACHAR>::GetBufferPointer(){ return m_pReceiver; }
	template <> void   TcharReceiver<ACHAR>::Apply(){} //何もしない

#endif

//インスタンス化
template class TcharReceiver<WCHAR>;
template class TcharReceiver<ACHAR>;


