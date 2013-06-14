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

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/


#include "StdAfx.h"
#include <io.h>
#include "CMigemo.h"
#include "CShareData.h"
#include "etc_uty.h"
#include "charcode.h"

int __cdecl pcre_char2int_sjis(const unsigned char*, unsigned int*);
int __cdecl pcre_char2int_utf8(const unsigned char*, unsigned int*);
int __cdecl pcre_int2char_utf8(unsigned int, unsigned char*);

//-----------------------------------------
//	DLL 初期化関数
//-----------------------------------------
int CMigemo::InitDll(void)
{
	//	staticにしてはいけないらしい
	
	const ImportTable table[] = {
		&m_migemo_open              ,"migemo_open"              ,
		&m_migemo_close             ,"migemo_close"             ,
		&m_migemo_query             ,"migemo_query"             ,
		&m_migemo_release           ,"migemo_release"           ,
		&m_migemo_set_operator      ,"migemo_set_operator"      ,
		&m_migemo_get_operator      ,"migemo_get_operator"      ,
		&m_migemo_setproc_char2int  ,"migemo_setproc_char2int"  ,
		&m_migemo_setproc_int2char  ,"migemo_setproc_int2char"  ,
		&m_migemo_load              ,"migemo_load"              ,
		&m_migemo_is_enable         ,"migemo_is_enable"         ,     
		NULL, 0
	};
	
	if( ! RegisterEntries(table) ){
			return 1;
	}

	m_migemo_open_s             = (Proc_migemo_open_s)            m_migemo_open;
	m_migemo_close_s            = (Proc_migemo_close_s)           m_migemo_close;
	m_migemo_query_s            = (Proc_migemo_query_s)           m_migemo_query;
	m_migemo_release_s          = (Proc_migemo_release_s)         m_migemo_release;
	m_migemo_set_operator_s     = (Proc_migemo_set_operator_s)    m_migemo_set_operator;
	m_migemo_get_operator_s     = (Proc_migemo_get_operator_s)    m_migemo_get_operator;
	m_migemo_setproc_char2int_s = (Proc_migemo_setproc_char2int_s)m_migemo_setproc_char2int;
	m_migemo_setproc_int2char_s = (Proc_migemo_setproc_int2char_s)m_migemo_setproc_int2char;
	m_migemo_load_s             = (Proc_migemo_load_s)            m_migemo_load;
	m_migemo_is_enable_s        = (Proc_migemo_is_enable_s)       m_migemo_is_enable;

	// ver 1.3 以降は stdcall
	DWORD dwVersionMS, dwVersionLS;
	GetAppVersionInfo( GetInstance(), VS_VERSION_INFO, &dwVersionMS, &dwVersionLS );
	
	DWORD dwver103 = (1 << 16) | 3;
	if( dwver103 <= dwVersionMS ){
		m_bStdcall = true;
	}else{
		m_bStdcall = false;
	}

	if( ! migemo_open(NULL) )
		return 1;
	
	return 0;
}

int CMigemo::DeInitDll(void)
{
	migemo_close();
	return 0;
}

const char* CMigemo::GetDllName(const char *)
{
	char* szDll;
	static char szDllName[_MAX_PATH];
	szDll = CShareData::getInstance()->GetShareData()->m_Common.m_sHelper.m_szMigemoDll;

	if(szDll[0] == _T('\0')){
		GetInidir( szDllName, _T("migemo.dll") );
		return fexist(szDllName) ? szDllName: _T("migemo.dll");
	}else{
		if(_IS_REL_PATH(szDll)){
			GetInidirOrExedir(szDllName , szDll);	// 2007.05.21 ryoji 相対パスは設定ファイルからのパスを優先
			szDll = szDllName;
		}
		return szDll;
	}
	//return "migemo.dll";
}

long CMigemo::migemo_open(char* dict)
{	

	if (!IsAvailable())
		return 0;
	if( m_bStdcall ){
		m_migemo = (*m_migemo_open_s)(NULL);
	}else{
		m_migemo = (*m_migemo_open)(NULL);
	}

	if (m_migemo == NULL)
		return 0;
	
	//if (!migemo_load(MIGEMO_DICTID_MIGEMO, path2))
	//	return 0;
	
	return 1;
}
void CMigemo::migemo_close()
{
	if (!IsAvailable() || (m_migemo == NULL))
		return;

	if( m_bStdcall ){
		(*m_migemo_close_s)(m_migemo);
	}else{
		(*m_migemo_close)(m_migemo);
	}
}
unsigned char* CMigemo::migemo_query(unsigned char* query)
{
	if (!IsAvailable() || (m_migemo == NULL))
		return NULL;
	
	if( m_bStdcall ){
		return (*m_migemo_query_s)(m_migemo, query);
	}else{
		return (*m_migemo_query)(m_migemo, query);
	}
}

std::string CMigemo::migemo_query_a(const unsigned char* query)
{
	unsigned char* ret = migemo_query((unsigned char*)query);
	std::string retVal = (const char*)ret;
	migemo_release(ret);
	return retVal;
}

void CMigemo::migemo_release( unsigned char* str)
{
	if (!IsAvailable() || (m_migemo == NULL))
		return;

	if( m_bStdcall ){
		(*m_migemo_release_s)(m_migemo, str);
	}else{
		(*m_migemo_release)(m_migemo, str);
	}

}
int CMigemo::migemo_set_operator(int index, unsigned char* op)
{
	if (!IsAvailable() || (m_migemo == NULL))
		return 0;
	
	if( m_bStdcall ){
		return (*m_migemo_set_operator_s)(m_migemo, index, op);
	}else{
		return (*m_migemo_set_operator)(m_migemo, index, op);
	}
}
const unsigned char* CMigemo::migemo_get_operator(int index)
{
	if (!IsAvailable() || (m_migemo == NULL))
		return NULL;
	
	if( m_bStdcall ){
		return (*m_migemo_get_operator_s)(m_migemo,index);
	}else{
		return (*m_migemo_get_operator)(m_migemo,index);
	}
}
void CMigemo::migemo_setproc_char2int(MIGEMO_PROC_CHAR2INT proc)
{
	if (!IsAvailable() || (m_migemo == NULL))
		return ;
	
	if( m_bStdcall ){
		(*m_migemo_setproc_char2int_s)(m_migemo,proc);
	}else{
		(*m_migemo_setproc_char2int)(m_migemo,proc);
	}
}
void CMigemo::migemo_setproc_int2char(MIGEMO_PROC_INT2CHAR proc)
{
	if (!IsAvailable() || (m_migemo == NULL))
		return;

	if( m_bStdcall ){
		(*m_migemo_setproc_int2char_s)(m_migemo,proc);
	}else{
		(*m_migemo_setproc_int2char)(m_migemo,proc);
	}
}

int CMigemo::migemo_load_a(int dict_id, const char* dict_file)
{
	if (!IsAvailable() || (m_migemo == NULL))
		return 0;
	if( m_bStdcall ){
		return (*m_migemo_load_s)(m_migemo, dict_id, dict_file);
	}else{
		return (*m_migemo_load)(m_migemo, dict_id, dict_file);
	}
}

int CMigemo::migemo_is_enable()
{
	if (!IsAvailable() || (m_migemo == NULL))
		return 0;
	
	if( m_bStdcall ){
		return (*m_migemo_is_enable_s)(m_migemo);
	}else{
		return (*m_migemo_is_enable)(m_migemo);
	}
}

int CMigemo::migemo_load_all()
{
	if( !migemo_is_enable()){
		
		TCHAR* szDict = CShareData::getInstance()->GetShareData()->m_Common.m_sHelper.m_szMigemoDict;
		TCHAR path[MAX_PATH];
		//char path2[MAX_PATH];
		TCHAR *ppath;
		
		if (szDict[0] == _T('\0')){
			GetInidirOrExedir(path,_T("dict"));	// 2007.05.20 ryoji 相対パスは設定ファイルからのパスを優先
		}
		else{
			if (_IS_REL_PATH(szDict)){
				GetInidirOrExedir(path,szDict);	// 2007.05.19 ryoji 相対パスは設定ファイルからのパスを優先
			}else{
				_tcscpy(path,szDict);
			}
		}
		ppath = &path[_tcslen(path)];
		*(ppath++) = _T('\\');
		// ver1.3 utf8対応
		_tcscpy(ppath,_T("cp932\\migemo-dict"));
		if(fexist(path)){
			_tcscpy(ppath,_T("cp932\\"));
			ppath = &path[_tcslen(path)];
		}
		_tcscpy(ppath,_T("migemo-dict"));

		migemo_load_t(MIGEMO_DICTID_MIGEMO,path);
		_tcscpy(ppath,_T("han2zen.dat"));
		migemo_load_t(MIGEMO_DICTID_HAN2ZEN,path);
		_tcscpy(ppath,_T("hira2kata.dat"));
		migemo_load_t(MIGEMO_DICTID_HIRA2KATA,path);
		_tcscpy(ppath,_T("roma2hira.dat"));
		migemo_load_t(MIGEMO_DICTID_ROMA2HIRA,path);
		_tcscpy(ppath,_T("zen2han.dat"));
		migemo_load_t(MIGEMO_DICTID_ZEN2HAN,path);

		// 2011.12.11 Moca 辞書登録後でないとmigemo内臓のものに変更されてしまう
		{
			migemo_setproc_char2int(pcre_char2int_sjis);
			migemo_setproc_int2char(pcre_int2char);	// 2009.04.30 miau
		}
	}
	return 1;
}


CMigemo::~CMigemo()
{
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

/*[EOF]*/
