/*!	@file
	@brief C/Migemo インターフェース

	@author isearch
	@date 2004.09.14 新規作成
*/
/*
	Copyright (C) 2004, isearch
	Copyright (C) 2007, ryoji
	Copyright (C) 2009, miau
	Copyright (C) 2012, Moca
	Copyright (C) 2018-2025, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "extmodule/CMigemo.h"

#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"
#include "charset/CUtf8.h"
#include "charset/codechecker.h"
#include "util/module.h"
#include "util/file.h"
#include "util/tchar_convert.h"
#include "mem/CNativeA.h"

/*! @brief PCRE メタキャラクタのエスケープ処理を行う。
 （CMigemo::migemo_setproc_int2char の引数として使用）
 @param[in] in 入力文字コード(unsigned int)
 @param[out] out 出力バイト列(unsigned char*)
 @return 出力された文字列のバイト数。
  0を返せばデフォルトのプロシージャが実行される。
*/
static int __cdecl pcre_int2char(unsigned int in, unsigned char* out);	// 2009.04.30 miau

int __cdecl pcre_char2int_sjis(const unsigned char*, unsigned int*);
int __cdecl pcre_char2int_utf8(const unsigned char*, unsigned int*);
int __cdecl pcre_int2char_utf8(unsigned int, unsigned char*);

//-----------------------------------------
//	DLL 初期化関数
//-----------------------------------------
bool CMigemo::InitDllImp()
{
	//	staticにしてはいけないらしい
	
	const ImportTable table[] = {
		{ &m_migemo_open              ,"migemo_open"              },
		{ &m_migemo_close             ,"migemo_close"             },
		{ &m_migemo_query             ,"migemo_query"             },
		{ &m_migemo_release           ,"migemo_release"           },
		{ &m_migemo_setproc_char2int  ,"migemo_setproc_char2int"  },
		{ &m_migemo_setproc_int2char  ,"migemo_setproc_int2char"  },
		{ &m_migemo_load              ,"migemo_load"              },
		{ &m_migemo_is_enable         ,"migemo_is_enable"         },
		{ nullptr, nullptr                                        }
	};
	
	if( ! RegisterEntries(table) ){
		return false;
	}

	m_bUtf8 = false;

	assert(IsAvailable());

	// 引数なしで migemo_open を呼び出す
	m_migemo = (*m_migemo_open)(nullptr);

	return m_migemo != nullptr;
}

bool CMigemo::DeinitDllImp(void)
{
	if (IsAvailable() && m_migemo) {
		(*m_migemo_close)(m_migemo);
	}

	return true;
}

LPCWSTR CMigemo::GetDllNameImp(int nIndex)
{
	UNREFERENCED_PARAMETER(nIndex); // ←CDllImplの再設計を推奨

	const auto& szMigemoDll = GetDllShareData().m_Common.m_sHelper.m_szMigemoDll;

	if (std::filesystem::path dllPath{ szMigemoDll }; !dllPath.empty()) {
		// 相対パスはiniファイル基準に変換
		if(dllPath.is_relative()) {
			dllPath = GetIniFileName().parent_path() / dllPath;
		}

		// 指定されたパスが存在する場合はそれを使う
		if (fexist(dllPath)) {
			return szMigemoDll;
		}
	}

	// デフォルトのDLL名を返す
	return L"migemo.dll";
}

std::string_view CMigemo::_migemo_query(const std::string& query) noexcept
{
	assert(IsAvailable());
	assert(m_migemo);
	return (LPCSTR)(*m_migemo_query)(m_migemo, LPBYTE(query.c_str()));
}

std::wstring CMigemo::migemo_query_w(std::wstring_view query) noexcept
{
	try {
		const UINT codePage = m_bUtf8 ? CP_UTF8 : CP_SJIS;
		const auto found = _migemo_query(cxx::to_string(query, codePage));
		const auto ret = cxx::to_wstring(found, codePage);
		_migemo_release(found);
		return ret;

	} catch (const std::invalid_argument&) {
		return std::wstring(query);
	}
}

void CMigemo::_migemo_release(std::string_view found) noexcept
{
	assert(IsAvailable());
	assert(m_migemo);
	(*m_migemo_release)(m_migemo, LPBYTE(found.data()));
}

int CMigemo::_migemo_load(int dict_id, const std::filesystem::path& dict_file) noexcept
{
	assert(IsAvailable());
	assert(m_migemo);
	return (*m_migemo_load)(m_migemo, dict_id, dict_file.string().c_str());
}

int CMigemo::migemo_is_enable() noexcept
{
	if (!IsAvailable() || (m_migemo == nullptr))
		return 0;
	
	return (*m_migemo_is_enable)(m_migemo);
}

int CMigemo::migemo_load_all() noexcept
{
	// 利用できない場合0を返しておく
	if (!IsAvailable() || !m_migemo) {
		return 0;
	}

	if (!migemo_is_enable()) {
		std::filesystem::path dictPath = GetDllShareData().m_Common.m_sHelper.m_szMigemoDict;

		std::filesystem::path path;
		if (dictPath.empty()) {
			path = GetIniFileName().parent_path() / L"dict";
		} else if (dictPath.is_relative()){
			path = GetIniFileName().parent_path() / dictPath;
		} else {
			path = dictPath;
		}

		// ver1.3 utf8対応
		if (fexist(path / L"utf-8" / L"migemo-dict")) {
			path /= L"utf-8";
			m_bUtf8 = true;
		} else if (fexist(path / L"cp932" / L"migemo-dict" )) {
			path /= L"cp932";
			m_bUtf8 = false;
		} else {
			return 0;
		}

		_migemo_load(MIGEMO_DICTID_MIGEMO,		path / L"migemo-dict");
		_migemo_load(MIGEMO_DICTID_HAN2ZEN,		path / L"han2zen.dat");
		_migemo_load(MIGEMO_DICTID_HIRA2KATA,	path / L"hira2kata.dat");
		_migemo_load(MIGEMO_DICTID_ROMA2HIRA,	path / L"roma2hira.dat");
		_migemo_load(MIGEMO_DICTID_ZEN2HAN,		path / L"zen2han.dat");

		// 2011.12.11 Moca 辞書登録後でないとmigemo内臓のものに変更されてしまう
		if( m_bUtf8 ){
			(*m_migemo_setproc_char2int)(m_migemo, pcre_char2int_utf8);
			(*m_migemo_setproc_int2char)(m_migemo, pcre_int2char_utf8);
		}else{
			(*m_migemo_setproc_char2int)(m_migemo, pcre_char2int_sjis);
			(*m_migemo_setproc_int2char)(m_migemo, pcre_int2char);	// 2009.04.30 miau
		}
	}

	return 1;
}

CMigemo::~CMigemo()
{
	DeinitDll();
}

int __cdecl pcre_char2int_sjis(const unsigned char* in, unsigned int* out)
{
	if( _IS_SJIS_1(in[0]) && _IS_SJIS_2(in[1]) ){
		if( out ){
			*out = (in[0] << 8) | in[1];
		}
		return 2;
	}
	if (out){
		*out = *in;
	}
    return 1;
}

// C/Migemo ソース中の rxgen.c:default_int2char を元に作成。	// 2009.04.30 miau
static int __cdecl pcre_int2char(unsigned int in, unsigned char* out)
{
    /* outは最低でも16バイトはある、という仮定を置く */
    if (in >= 0x100)
    {
	if (out)
	{
	    out[0] = (unsigned char)((in >> 8) & 0xFF);
	    out[1] = (unsigned char)(in & 0xFF);
	}
	return 2;
    }
    else
    {
	int len = 0;
	switch (in)
	{
	    case '\\':
	    case '.': case '*': case '^': case '$': case '/':
	    case '[': case ']': 
	    case '|': case '(': case ')':
	    case '+': case '?': case '{': case '}':
	    case ':': case '-': case '&':
		if (out)
		    out[len] = '\\';
		++len;
	    default:
		if (out)
		    out[len] = (unsigned char)(in & 0xFF);
		++len;
		break;
	}
	return len;
    }
}

int __cdecl pcre_char2int_utf8(const unsigned char* in, unsigned int* out)
{
	if( 0x80 & in[0] ){
		if( (0xe0 & in[0]) == 0xc0 && (0xc0 & in[1]) == 0x80 ){
			if( out ){
				*out = ((0x1f & in[0]) << 6)
					| (0x3f & in[1]);
			}
			return 2;
		}else if( (0xf0 & in[0]) == 0xe0 && (0xc0 & in[1]) == 0x80
			&& (0xc0 & in[2]) == 0x80 ){
			if( out ){
				*out = ((0x1f & in[0]) << 12)
					| ((0x3f & in[1]) << 6)
					| (0x3f & in[2]);
			}
			return 3;
		}else if( (0xf8 & in[0]) == 0xf0  && (0xc0 & in[1]) == 0x80
			&& (0xc0 & in[2]) == 0x80 && (0xc0 & in[3]) == 0x80){
			if( out ){
				*out = ((0x1f & in[0]) << 18)
					| ((0x3f & in[1]) << 12)
					| ((0x3f & in[2]) << 6)
					| (0x3f & in[3]);
			}
			return 4;
		}
	}
	if( out ){
		*out = *in;
	}
    return 1;
}

int __cdecl pcre_int2char_utf8(unsigned int in, unsigned char* out)
{
	int len = 0;
	if( in < 0x80 ){
		switch (in){
			case '\\':
			case '.': case '*': case '^': case '$': case '/':
			case '[': case ']': 
			case '|': case '(': case ')':
			case '+': case '?': case '{': case '}':
			case ':': case '-': case '&':
				if (out)
					out[len] = '\\';
				++len;
			default:
				if (out)
					out[len] = (unsigned char)(in & 0xFF);
				++len;
				break;
		}
	}else if( in < 0x800 ){
		if( out ){
			out[0] = static_cast<unsigned char>((in & 0x07c0) >> 6) | 0xc0;
			out[1] = static_cast<unsigned char>(in & 0x003f) | 0x80;
		}
		len = 2;
	}else if( in < 0x10000 ){
		if( out ){
			out[0] = static_cast<unsigned char>((in & 0xf000) >> 12) | 0xe0;
			out[1] = static_cast<unsigned char>((in & 0x0fc0) >> 6)  | 0x80;
			out[2] = static_cast<unsigned char>( in & 0x003f) | 0x80;
		}
		len = 3;
	}else{
		if( out ){
			out[0] = static_cast<unsigned char>((in & 0x001c0000) >> 18) | 0xf0;
			out[1] = static_cast<unsigned char>((in & 0x0003f000) >> 12) | 0x80;
			out[2] = static_cast<unsigned char>((in & 0x00000fc0) >> 6)  | 0x80;
			out[3] = static_cast<unsigned char>( in & 0x0000003f) | 0x80;
		}
		len = 4;
	}
	return len;
}
