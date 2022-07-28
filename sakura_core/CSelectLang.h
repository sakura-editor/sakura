﻿/*!	@file
	@brief 各国語メッセージリソース対応

	@author nasukoji
	@date 2011.04.10	新規作成
*/
/*
	Copyright (C) 2011, nasukoji
	Copyright (C) 2018-2022, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef SAKURA_CSELECTLANG_657416B2_2B3D_455C_AC28_8B86244F5F83_H_
#define SAKURA_CSELECTLANG_657416B2_2B3D_455C_AC28_8B86244F5F83_H_
#pragma once

#include <windows.h>
#include <vector>

#define MAX_SELLANG_NAME_STR	128		// メッセージリソースの言語名の最大文字列長（サイズは適当）

class CSelectLang
{

	using Me = CSelectLang;

public:
	// メッセージリソース用構造体
	struct SSelLangInfo {
		WCHAR szDllName[MAX_PATH];		// メッセージリソースDLLのファイル名
		WCHAR szLangName[MAX_SELLANG_NAME_STR];		// 言語名
		HINSTANCE hInstance;			// 読み込んだリソースのインスタンスハンドル
		WORD wLangId;					// 言語ID
		BOOL bValid;					// メッセージリソースDLLとして有効
	};

protected:
	//static LPWSTR m_szDefaultLang;					// メッセージリソースDLL未読み込み時のデフォルト言語
	static SSelLangInfo* m_psLangInfo;				// メッセージリソースの情報
public:
	typedef std::vector<SSelLangInfo*> PSSelLangInfoList;
	static PSSelLangInfoList m_psLangInfoList;

public:
	/*
	||  Constructors
	*/
	CSelectLang() noexcept = default;
	CSelectLang(const Me&) = delete;
	Me& operator = (const Me&) = delete;
	CSelectLang(Me&&) noexcept = delete;
	Me& operator = (Me&&) noexcept = delete;
	~CSelectLang();

	/*
	||  Attributes & Operations
	*/
	static HINSTANCE getLangRsrcInstance( void );			// メッセージリソースDLLのインスタンスハンドルを返す
	static LPCWSTR getDefaultLangString( void );			// メッセージリソースDLL未読み込み時のデフォルト言語（"(Japanese)" or "(English(United States))"）
	static WORD getDefaultLangId(void);

	static HINSTANCE InitializeLanguageEnvironment(void);		// 言語環境を初期化する
	static HINSTANCE LoadLangRsrcLibrary( SSelLangInfo& lang );	// メッセージ用リソースDLLをロードする
	static void ChangeLang( WCHAR* pszDllName );	// 言語を変更する

protected:
	/*
	||  実装ヘルパ関数
	*/
	static HINSTANCE ChangeLang( UINT nSelIndex );	// 言語を変更する

private:
};

/*!
	@brief 文字列リソース読み込みクラス

	@date 2011.06.01 nasukoji	新規作成
*/

#define LOADSTR_ADD_SIZE		256			// 文字列リソース用バッファの初期または追加サイズ（TCHAR単位）

class CLoadString
{
protected:
	// 文字列リソース読み込み用バッファクラス
	class CLoadStrBuffer
	{
	public:
		CLoadStrBuffer()
		{
			m_pszString   = m_szString;				// 変数内に準備したバッファを接続
			m_nBufferSize = _countof(m_szString);	// 配列個数
			m_nLength     = 0;
			m_szString[0] = L'\0';
		}

		/*virtual*/ ~CLoadStrBuffer()
		{
			// バッファを取得していた場合は解放する。
			if( m_pszString && m_pszString != m_szString ){
				delete[] m_pszString;
			}
		}

		/*virtual*/ LPCWSTR GetStringPtr() const { return m_pszString; }	// 読み込んだ文字列のポインタを返す
		/*virtual*/ int GetBufferSize() const { return m_nBufferSize; }		// 読み込みバッファのサイズ（TCHAR単位）を返す
		/*virtual*/ int GetStringLength() const { return m_nLength; }		// 読み込んだ文字数（TCHAR単位）を返す

		/*virtual*/ int LoadString( UINT uid );								// 文字列リソースを読み込む（読み込み実行部）

	protected:
		LPWSTR m_pszString;						// 文字列読み込みバッファのポインタ
		int m_nBufferSize;						// 取得配列個数（TCHAR単位）
		int m_nLength;							// 取得文字数（TCHAR単位）
		WCHAR m_szString[LOADSTR_ADD_SIZE];		// 文字列読み込みバッファ（バッファ拡張後は使用されない）

	private:
		CLoadStrBuffer( const CLoadStrBuffer& );					// コピー禁止とする
		CLoadStrBuffer operator = ( const CLoadStrBuffer& );		// 代入禁止とする
	};

	static CLoadStrBuffer m_acLoadStrBufferTemp[4];		// 文字列読み込みバッファの配列（CLoadString::LoadStringSt() が使用する）
	static int m_nDataTempArrayIndex;					// 最後に使用したバッファのインデックス（CLoadString::LoadStringSt() が使用する）
	CLoadStrBuffer m_cLoadStrBuffer;					// 文字列読み込みバッファ（CLoadString::LoadString() が使用する）

public:
	/*
	||  Constructors
	*/
	CLoadString() = default;
	CLoadString( UINT uid ){ LoadString( uid ); }		// 文字列読み込み付きコンストラクタ
	/*virtual*/ ~CLoadString() = default;

	/*
	||  Attributes & Operations
	*/
	/*virtual*/ LPCWSTR GetStringPtr() const { return m_cLoadStrBuffer.GetStringPtr(); }	// 読み込んだ文字列のポインタを返す
//	/*virtual*/ int GetBufferSize() const { return m_cLoadStrBuffer.GetBufferSize(); }		// 読み込みバッファのサイズ（TCHAR単位）を返す
	/*virtual*/ int GetStringLength() const { return m_cLoadStrBuffer.GetStringLength(); }	// 読み込んだ文字数（TCHAR単位）を返す

	static LPCWSTR LoadStringSt( UINT uid );			// 静的バッファに文字列リソースを読み込む（各国語メッセージリソース対応）
	/*virtual*/ LPCWSTR LoadString( UINT uid );			// 文字列リソースを読み込む（各国語メッセージリソース対応）

protected:

private:
	CLoadString( const CLoadString& );					// コピー禁止とする
	CLoadString operator = ( const CLoadString& );		// 代入禁止とする
};

// 文字列ロード簡易化マクロ
#define LS( id ) ( CLoadString::LoadStringSt( id ) )
#endif /* SAKURA_CSELECTLANG_657416B2_2B3D_455C_AC28_8B86244F5F83_H_ */
