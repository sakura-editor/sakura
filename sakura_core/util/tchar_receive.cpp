/*! @file */
#include "StdAfx.h"
#include "tchar_receive.h"
using namespace std;

//TcharReceiver実装
//UNICODEビルドで、WCHARを受け取る
template <> WCHAR* TcharReceiver<WCHAR>::GetBufferPointer(){ return m_pReceiver; }
template <> void   TcharReceiver<WCHAR>::Apply(){} //何もしない

//UNICODEビルドで、ACHARを受け取る
template <> WCHAR* TcharReceiver<ACHAR>::GetBufferPointer(){ return (m_pBuff = new WCHAR[m_nReceiverCount]); }
template <> void   TcharReceiver<ACHAR>::Apply(){ _tcstombs(m_pReceiver, m_pBuff, m_nReceiverCount); delete []m_pBuff; }

//インスタンス化
template class TcharReceiver<WCHAR>;
template class TcharReceiver<ACHAR>;

