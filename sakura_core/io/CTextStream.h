//2007.09.24 kobake 作成
//設定ファイル等のテキスト入出力を行うためのクラス群。
//.ini や .mac の入出力を扱うときに使うと良いです。
//※「編集テキスト」を扱うためではなく、あくまでも、.iniや.macのような「設定ファイル」を扱う目的のクラス群です。
//
//今のところはShiftJISで入出力を行うが、
//将来はUTF-8等にすることにより、UNICODEデータの欠落が起こらないようにしたい。

#pragma once

#include <string>

#include "CStream.h"

//テキスト入力ストリーム
class CTextInputStream : public CStream{
public:
	//コンストラクタ・デストラクタ
	CTextInputStream(const TCHAR* tszPath);
	CTextInputStream();
	virtual ~CTextInputStream();

	//操作
	void Open(const TCHAR* tszPath);
	std::wstring ReadLineW(); //!< 1行読込。改行は削る

private:
	bool m_bIsUtf8; //!< UTF-8ならtrue
};

//テキスト出力ストリーム
class CTextOutputStream : public CStream{
public:
	//コンストラクタ・デストラクタ
	CTextOutputStream(const TCHAR* tszPath);
	virtual ~CTextOutputStream();

	//文字列書込。改行を入れたい場合は、文字列内に'\n'を含めること。(クラス側で適切な改行コードに変換して出力します)
	void WriteString(const wchar_t* szData);
	void WriteF(const wchar_t* format, ...);

	//数値書込。(クラス側で適当に整形して出力します)
	void WriteInt(int n);
};



//テキスト入力ストリーム。相対パスの場合はINIファイルのパスからの相対パスとして開く。
class CTextInputStream_AbsIni : public CTextInputStream{
public:
	CTextInputStream_AbsIni(const TCHAR* tszPath, bool bOrExedir = true);
};
