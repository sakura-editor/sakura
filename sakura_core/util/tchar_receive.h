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
	2009.02.21 ryoji		標準文字列以外を扱う場合（UNICODEビルドでACHAR、ANSIビルドでWCHAR）に
							512文字のサイズ制限付き静的バッファを使用していたのを、
							サイズ制限の無い動的バッファを使うように変更。（負荷はどのみち変換のほうにがかかる）
*/
template <class RECEIVE_CHAR_TYPE>
class TcharReceiver{
public:
	TcharReceiver(RECEIVE_CHAR_TYPE* pReceiver, size_t nReceiverCount)	//!< 受け取りバッファを指定。
	: m_pReceiver(pReceiver), m_nReceiverCount(nReceiverCount), m_pBuff(NULL) { }
	operator TCHAR* (){ return GetBufferPointer(); }
	~TcharReceiver(){ Apply(); }
protected:
	TCHAR* GetBufferPointer();	//!< 一時バッファを提供。バッファ寿命は短いので注意。
	void Apply();				//!< 一時バッファから、実際の受け取りバッファへデータをコピー。
private:
	RECEIVE_CHAR_TYPE*	m_pReceiver;
	size_t				m_nReceiverCount;
	TCHAR*				m_pBuff;
};

#endif /* SAKURA_TCHAR_RECEIVE_173C52CE_CAC9_4ED5_8399_EFEF8CC7DBD2_H_ */
/*[EOF]*/
