#pragma once

/*!
	実際のデータ種に関わらず、TCHAR[]型の受け取りバッファを提供するクラス。
	使い方が特殊なので注意。

	例:
	{
		wchar_t buf[256];
		GetWindowText(hwnd,TcharReceiver(buf));
	}

	このコードは、ANSIビルド、UNICODEビルド、ともに通ります。
	ANSIビルド時は、char→wchar_t変換が発生するため、少し負荷がかかります (この例の場合)。
	UNICODEビルド時は、負荷は TcharReceiver を使わない場合とほとんど変わりません (この例の場合)。

	動作原理はソースを参照のこと。
	operator TCHAR* が GetWindowText に渡すポインタを提供し、
	~TcharReceiver において、必要であれば (ビルド種と受け取り型が異なれば)、
	TCHAR→wchar_t変換が発生します。

	2007.10.27 kobake 作成
*/
template <class RECEIVE_CHAR_TYPE>
class TcharReceiver{
public:
	TcharReceiver(RECEIVE_CHAR_TYPE* pReceiver, size_t nReceiverCount)	//!< 受け取りバッファを指定。
	: m_pReceiver(pReceiver), m_nReceiverCount(nReceiverCount) { }
	operator TCHAR* (){ return GetBufferPointer(); }
	~TcharReceiver(){ Apply(); }
protected:
	TCHAR* GetBufferPointer();	//!< 一時バッファを提供。バッファ寿命は短いので注意。
	void Apply();				//!< 一時バッファから、実際の受け取りバッファへデータをコピー。
private:
	RECEIVE_CHAR_TYPE*	m_pReceiver;
	size_t				m_nReceiverCount;
};
