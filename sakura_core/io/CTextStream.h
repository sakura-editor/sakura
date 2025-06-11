/*! @file */
//2007.09.24 kobake 作成
//設定ファイル等のテキスト入出力を行うためのクラス群。
//.ini や .mac の入出力を扱うときに使うと良いです。
//※「編集テキスト」を扱うためではなく、あくまでも、.iniや.macのような「設定ファイル」を扱う目的のクラス群です。
//
//今のところはShiftJISで入出力を行うが、
//将来はUTF-8等にすることにより、UNICODEデータの欠落が起こらないようにしたい。
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CTEXTSTREAM_CF4FEC73_4575_4B80_98F7_CFCBC0B433CD_H_
#define SAKURA_CTEXTSTREAM_CF4FEC73_4575_4B80_98F7_CFCBC0B433CD_H_
#pragma once

#include <string>

#include "CStream.h"
#include "charset/charset.h"

class CCodeBase;

//テキスト入力ストリーム (UTF-8, SJIS)
class CTextInputStream : public CStream{
public:
	//コンストラクタ・デストラクタ
	CTextInputStream(const WCHAR* pszPath);
	CTextInputStream();
	virtual ~CTextInputStream();

	//操作
	std::wstring ReadLineW(); //!< 1行読込。改行は削る

private:
	bool m_bIsUtf8; //!< UTF-8ならtrue
};

//テキスト出力ストリーム
// 2008.01.26 kobake 出力文字コードを任意で指定できるように変更
class CTextOutputStream final : public COutputStream{
	using Me = CTextOutputStream;

public:
	//コンストラクタ・デストラクタ
	CTextOutputStream(const WCHAR* pszPath, ECodeType eCodeType = CODE_UTF8, bool bExceptionMode = false, bool bBom = true);
	CTextOutputStream(const Me&) = delete;
	Me& operator = (const Me&) = delete;
	CTextOutputStream(Me&&) noexcept = delete;
	Me& operator = (Me&&) noexcept = delete;
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
class CTextInputStream_AbsIni final : public CTextInputStream{
public:
	CTextInputStream_AbsIni(const WCHAR* pszPath, bool bOrExedir = true);
};
#endif /* SAKURA_CTEXTSTREAM_CF4FEC73_4575_4B80_98F7_CFCBC0B433CD_H_ */
