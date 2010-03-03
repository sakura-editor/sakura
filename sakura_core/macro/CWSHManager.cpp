/*!	@file
	@brief WSH Manager

	@date 2009.10.29 syat CWSH.cpp����؂�o��
*/
/*
	Copyright (C) 2002, �S, genta
	Copyright (C) 2003, FILE
	Copyright (C) 2004, genta
	Copyright (C) 2005, FILE, zenryaku
	Copyright (C) 2009, syat

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

#include "stdafx.h"
#include "macro/CWSHManager.h"
#include "macro/CWSH.h"
#include "macro/CEditorIfObj.h"
#include "io/CTextStream.h"
#include "util/os.h"
#include "macro/CMacroFactory.h"

static void MacroError(BSTR Description, BSTR Source, void *Data)
{
	CEditView *View = reinterpret_cast<CEditView*>(Data);
	MessageBox(View->GetHwnd(), to_tchar(Description), to_tchar(Source), MB_ICONERROR);
}

CWSHMacroManager::CWSHMacroManager(std::wstring const AEngineName) : m_EngineName(AEngineName)
{
}

CWSHMacroManager::~CWSHMacroManager()
{
}

/** WSH�}�N���̎��s

	@param EditView [in] ����Ώ�EditView
	
	@date 2007.07.20 genta : flags�ǉ�
*/
void CWSHMacroManager::ExecKeyMacro(CEditView *EditView, int flags) const
{
	CWSHClient* Engine;
	Engine = new CWSHClient(m_EngineName.c_str(), MacroError, EditView);
	if(Engine->m_Valid)
	{
		//�C���^�t�F�[�X�I�u�W�F�N�g�̓o�^
		CWSHIfObj* objEditor = new CEditorIfObj();
		objEditor->AddRef();
		Engine->AddInterfaceObject( objEditor );

		Engine->AddInterfaceObject( (CWSHIfObj::List&)m_Params );
	
		//�R�}���h����
		const CWSHIfObj::List& objects = Engine->GetInterfaceObject();
		for( CWSHIfObj::ListIter it = objects.begin(); it != objects.end(); it++ ){
			(*it)->ReadyMethods( flags );
		}
		
		Engine->Execute(m_Source.c_str());
		
		//EditView->Redraw();
		EditView->GetCaret().ShowEditCaret();
	}
	delete Engine;
}

/*!
	WSH�}�N���̓ǂݍ��݁i�t�@�C������j

	@param Instance [in] �C���X�^���X�n���h��(���g�p)
	@param Path		[in] �t�@�C���̃p�X
*/
BOOL CWSHMacroManager::LoadKeyMacro(HINSTANCE hInstance, const TCHAR* szPath)
{
	//�\�[�X�ǂݍ��� -> m_Source
	m_Source=L"";
	
	CTextInputStream in(szPath);
	if(!in)
		return FALSE;

	while(in){
		m_Source+=in.ReadLineW()+L"\r\n";
	}
	return TRUE;
}

/*!
	WSH�}�N���̓ǂݍ��݁i�����񂩂�j

	@param Instance [in] �C���X�^���X�n���h��(���g�p)
	@param szCode	[in] �}�N���R�[�h
*/
BOOL CWSHMacroManager::LoadKeyMacroStr(HINSTANCE hInstance, const TCHAR* szCode)
{
	//�\�[�X�ǂݍ��� -> m_Source
	m_Source = to_wchar( szCode );
	return TRUE;
}

CMacroManagerBase* CWSHMacroManager::Creator(const TCHAR* FileExt)
{
	TCHAR FileExtWithDot[1024], FileType[1024], EngineName[1024]; //1024�𒴂������͒m��܂���
	
	_tcscpy( FileExtWithDot, _T(".") );
	_tcscat( FileExtWithDot, FileExt );

	if(ReadRegistry(HKEY_CLASSES_ROOT, FileExtWithDot, NULL, FileType, 1024))
	{
		lstrcat(FileType, _T("\\ScriptEngine"));
		if(ReadRegistry(HKEY_CLASSES_ROOT, FileType, NULL, EngineName, 1024))
		{
			wchar_t EngineNameW[1024];
			_tcstowcs(EngineNameW, EngineName, _countof(EngineNameW));
			return new CWSHMacroManager(EngineNameW);
		}
	}
	return NULL;
}

void CWSHMacroManager::declare()
{
	//�b��
	CMacroFactory::Instance()->RegisterCreator(Creator);
}

//�C���^�t�F�[�X�I�u�W�F�N�g��ǉ�����
void CWSHMacroManager::AddParam( CWSHIfObj* param )
{
	m_Params.push_back( param );
}

//�C���^�t�F�[�X�I�u�W�F�N�g�B��ǉ�����
void CWSHMacroManager::AddParam( CWSHIfObj::List& params )
{
	m_Params.insert( m_Params.end(), params.begin(), params.end() );
}

//�C���^�t�F�[�X�I�u�W�F�N�g���폜����
void CWSHMacroManager::ClearParam()
{
	m_Params.clear();
}
