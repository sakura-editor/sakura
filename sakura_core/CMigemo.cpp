/*!	@file
	@brief C/Migemo �C���^�[�t�F�[�X

	@author isearch
	@date 2004.09.14 �V�K�쐬
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
#include "env/CShareData.h"
#include "util/module.h"
#include "util/file.h"

//-----------------------------------------
//	DLL �������֐�
//-----------------------------------------
CMigemo* CMigemo::_instance;

CMigemo* CMigemo::getInstance(){
	return _instance;
}

CMigemo::CMigemo() {
	_instance = this;
}


bool CMigemo::InitDllImp()
{
	//	static�ɂ��Ă͂����Ȃ��炵��
	
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
		return false;
	}

	if( ! migemo_open(NULL) )
		return false;
	
	return true;
}

int CMigemo::DeInitDll(void)
{
	migemo_close();
	return 0;
}

LPCTSTR CMigemo::GetDllNameImp(int nIndex)
{
	if(nIndex==0){
		TCHAR* szDll;
		static TCHAR szDllName[_MAX_PATH];
		szDll = CShareData::getInstance()->GetShareData()->m_Common.m_sHelper.m_szMigemoDll;

		if(_tcslen(szDll)==0){
			GetInidir( szDllName, _T("migemo.dll") );
			return fexist(szDllName) ? szDllName : _T("migemo.dll");
		}
		else{
			if(_IS_REL_PATH(szDll)){
				GetInidirOrExedir(szDllName , szDll);	// 2007.05.21 ryoji ���΃p�X�͐ݒ�t�@�C������̃p�X��D��
				szDll = szDllName;
			}
			return szDll;
		}
		//return "migemo.dll";
	}
	else{
		return NULL;
	}
}

long CMigemo::migemo_open(char* dict)
{	

	if (!IsAvailable())
		return 0;

	m_migemo = (*m_migemo_open)(NULL);
	
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

int CMigemo::migemo_load_a(int dict_id, const char* dict_file)
{
	if (!IsAvailable() || (m_migemo == NULL))
		return 0;
	return (*m_migemo_load)(m_migemo, dict_id, dict_file);

}

int CMigemo::migemo_load_w(int dict_id, const wchar_t* dict_file)
{
	char szBuf[_MAX_PATH];
	wcstombs2(szBuf,dict_file,_countof(szBuf));
	return migemo_load_a(dict_id,szBuf);
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
		
		TCHAR* szDict = CShareData::getInstance()->GetShareData()->m_Common.m_sHelper.m_szMigemoDict;
		TCHAR path[MAX_PATH];
		//char path2[MAX_PATH];
		TCHAR *ppath;
		
		if (_tcslen(szDict) == 0){
			GetInidirOrExedir(path,_T("dict"));	// 2007.05.20 ryoji ���΃p�X�͐ݒ�t�@�C������̃p�X��D��
		}
		else{
			if (_IS_REL_PATH(szDict)){
				GetInidirOrExedir(path,szDict);	// 2007.05.19 ryoji ���΃p�X�͐ݒ�t�@�C������̃p�X��D��
			}else{
				_tcscpy(path,szDict);
			}
		}
		ppath = &path[_tcslen(path)];
		*(ppath++) = _T('\\');

		_tcscpy(ppath,_T("migemo-dict"));
		migemo_load_t(MIGEMO_DICTID_MIGEMO,path);
		_tcscpy(ppath,_T("han2zen.dat"));
		migemo_load_t(MIGEMO_DICTID_HAN2ZEN,path);
		_tcscpy(ppath,_T("hira2kata.dat"));
		migemo_load_t(MIGEMO_DICTID_HIRA2KATA,path);
		_tcscpy(ppath,_T("roma2hira.dat"));
		migemo_load_t(MIGEMO_DICTID_ROMA2HIRA,path);
	}
	return 1;
}


CMigemo::~CMigemo()
{
}



