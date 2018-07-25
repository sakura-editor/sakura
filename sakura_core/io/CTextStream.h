//2007.09.24 kobake 作成
//設定ファイル等のテキスト入出力を行うためのクラス群。
//.ini や .mac の入出力を扱うときに使うと良いです。
//※「編集テキスト」を扱うためではなく、あくまでも、.iniや.macのような「設定ファイル」を扱う目的のクラス群です。
//
//今のところはShiftJISで入出力を行うが、
//将来はUTF-8等にすることにより、UNICODEデータの欠落が起こらないようにしたい。
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
#ifndef SAKURA_CTEXTSTREAM_0D3CC877_CA34_4CC8_9596_B120F4F902939_H_
#define SAKURA_CTEXTSTREAM_0D3CC877_CA34_4CC8_9596_B120F4F902939_H_

#include <string>

#include "CStream.h"
class CCodeBase;

//テキスト入力ストリーム (UTF-8, SJIS)
class CTextInputStream : public CStream{
public:
	//コンストラクタ・デストラクタ
	CTextInputStream(const TCHAR* tszPath);
	CTextInputStream();
	virtual ~CTextInputStream();

	//操作
	std::wstring ReadLineW(); //!< 1行読込。改行は削る

private:
	bool m_bIsUtf8; //!< UTF-8ならtrue
};

//テキスト出力ストリーム
// 2008.01.26 kobake 出力文字コードを任意で指定できるように変更
class CTextOutputStream : public COutputStream{
public:
	//コンストラクタ・デストラクタ
	CTextOutputStream(const TCHAR* tszPath, ECodeType eCodeType = CODE_UTF8, bool bExceptionMode = false, bool bBom = true);
	virtual ~CTextOutputStream();

	//文字列書込。改行を入れたい場合は、文字列内に'\n'を含めること。(クラス側で適切な改行コードに変換して出力します)
	void WriteString(const wchar_t* szData, int nLen = -1);
	void WriteF(const wchar_t* format, ...);

	//数値書込。(クラス側で適当に整形して出力します)
	void WriteInt(int n);

private:
	CCodeBase* m_pcCodeBase;
};



//テキスト入力ストリーム。相対パスの場合はINIファイルのパスからの相対パスとして開く。
class CTextInputStream_AbsIni : public CTextInputStream{
public:
	CTextInputStream_AbsIni(const TCHAR* tszPath, bool bOrExedir = true);
};

#endif /* SAKURA_CTEXTSTREAM_0D3CC877_CA34_4CC8_9596_B120F4F902939_H_ */
/*[EOF]*/
