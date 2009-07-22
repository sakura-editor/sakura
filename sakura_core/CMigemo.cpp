/*!	@file
	@brief C/Migemo インターフェース

	@author isearch
	@date 2004.09.14 新規作成
*/
/*
	Copyright (C) 2004, isearch
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/


#include "stdafx.h"
#include <io.h>
#include "CMigemo.h"
#include "Csharedata.h"
#include "etc_uty.h"
//-----------------------------------------
//	DLL 初期化関数
//-----------------------------------------
CMigemo* CMigemo::_instance;

CMigemo* CMigemo::getInstance(){
	return _instance;
}

CMigemo::CMigemo() {
	_instance = this;
}


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
	szDll = CShareData::getInstance()->GetShareData()->m_Common.m_szMigemoDll;

	if(strlen(szDll)==0){
		GetInidir( szDllName, _T("migemo.dll") );
		return ( _taccess( szDllName, 0 ) != -1 )? szDllName: _T("migemo.dll");
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

	m_migemo = (*m_migemo_open)(NULL);
	
	if (m_migemo == NULL)
		return 0;
	
	migemo_setproc_int2char(pcre_int2char);
	//if (!migemo_load(MIGEMO_DICTID_MIGEMO, path2))
	//	return 0;
	
	return 1;
}
void CMigemo::migemo_close()
{
	if (!IsAvailable() || (m_migemo == NULL))
		return;

	(*m_migemo_close)(m_migemo);

}
unsigned char* CMigemo::migemo_query(unsigned char* query)
{
	if (!IsAvailable() || (m_migemo == NULL))
		return NULL;
	
	return (*m_migemo_query)(m_migemo, query);
}
void CMigemo::migemo_release( unsigned char* string)
{
	if (!IsAvailable() || (m_migemo == NULL))
		return;

	(*m_migemo_release)(m_migemo, string);

}
int CMigemo::migemo_set_operator(int index, unsigned char* op)
{
	if (!IsAvailable() || (m_migemo == NULL))
		return 0;
	
	return (*m_migemo_set_operator)(m_migemo, index, op);

}
const unsigned char* CMigemo::migemo_get_operator(int index)
{
	if (!IsAvailable() || (m_migemo == NULL))
		return NULL;
	
	return (*m_migemo_get_operator)(m_migemo,index);

}
void CMigemo::migemo_setproc_char2int(MIGEMO_PROC_CHAR2INT proc)
{
	if (!IsAvailable() || (m_migemo == NULL))
		return ;
	
	(*m_migemo_setproc_char2int)(m_migemo,proc);
}
void CMigemo::migemo_setproc_int2char(MIGEMO_PROC_INT2CHAR proc)
{
	if (!IsAvailable() || (m_migemo == NULL))
		return;

	(*m_migemo_setproc_int2char)(m_migemo,proc);
}
int CMigemo::migemo_load(int dict_id, char* dict_file)
{
	if (!IsAvailable() || (m_migemo == NULL))
		return 0;
	return (*m_migemo_load)(m_migemo, dict_id, dict_file);

}
int CMigemo::migemo_is_enable()
{
	if (!IsAvailable() || (m_migemo == NULL))
		return 0;
	
	return (*m_migemo_is_enable)(m_migemo);
}

int CMigemo::migemo_load_all()
{
	if( !migemo_is_enable()){
		
		char* szDict = CShareData::getInstance()->GetShareData()->m_Common.m_szMigemoDict;
		char path[MAX_PATH];
		//char path2[MAX_PATH];
		char *ppath;
		
		if (strlen(szDict) == 0){
			GetInidirOrExedir(path,_T("dict"));	// 2007.05.20 ryoji 相対パスは設定ファイルからのパスを優先
			
			//::GetModuleFileName(NULL,path,MAX_PATH);
			//::GetFullPathName(path,260,path2,&ppath);			
			//strcpy(ppath, "dict\\");
		}else{
			if (_IS_REL_PATH(szDict)){
				GetInidirOrExedir(path,szDict);	// 2007.05.19 ryoji 相対パスは設定ファイルからのパスを優先
			}else{
				strcpy(path,szDict);
			}
		}
		ppath = &path[strlen(path)];
		*(ppath++) = '\\';

		strcpy(ppath,"migemo-dict");
		migemo_load(MIGEMO_DICTID_MIGEMO,path);
		strcpy(ppath,"han2zen.dat");
		migemo_load(MIGEMO_DICTID_HAN2ZEN,path);
		strcpy(ppath,"hira2kata.dat");
		migemo_load(MIGEMO_DICTID_HIRA2KATA,path);
		strcpy(ppath,"roma2hira.dat");
		migemo_load(MIGEMO_DICTID_ROMA2HIRA,path);
	}
	return 1;
}

CMigemo::~CMigemo()
{
}

// C/Migemo ソース中の rxgen.c:default_int2char を元に作成。
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
	    case '[': case ']': case '~':
	    case '|': case '(': case ')':
	    case '+': case '?': case '{': case '}':
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

/*[EOF]*/
