/*!	@file
	@brief Plugin�I�u�W�F�N�g

*/
/*
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
#ifndef SAKURA_CPLUGINIFOBJ_2205DDE8_330B_49AC_AC5E_4C02F07DDCD5D_H_
#define SAKURA_CPLUGINIFOBJ_2205DDE8_330B_49AC_AC5E_4C02F07DDCD5D_H_

#include "macro/CWSHIfObj.h"
#include "_os/OleTypes.h"
#include "util/ole_convert.h"

// cpp�ֈړ��\��
#include "window/CEditWnd.h"
#include "view/CEditView.h"

class CPluginIfObj : public CWSHIfObj {
	// �^��`
	enum FuncId {
		F_PL_COMMAND_FIRST = 0,					//���R�}���h�͈ȉ��ɒǉ�����
		F_PL_SETOPTION,							//�I�v�V�����t�@�C���ɒl������
		F_PL_ADDCOMMAND,						//�R�}���h��ǉ�����
		F_PL_FUNCTION_FIRST = F_FUNCTION_FIRST,	//���֐��͈ȉ��ɒǉ�����
		F_PL_GETPLUGINDIR,						//�v���O�C���t�H���_�p�X���擾����
		F_PL_GETDEF,							//�ݒ�t�@�C������l��ǂ�
		F_PL_GETOPTION,							//�I�v�V�����t�@�C������l��ǂ�
		F_PL_GETCOMMANDNO,						//���s���v���O�̔ԍ����擾����
	};
	typedef std::string string;
	typedef std::wstring wstring;

	// �R���X�g���N�^
public:
	CPluginIfObj( CPlugin& cPlugin )
		: CWSHIfObj( L"Plugin", false )
		, m_cPlugin( cPlugin )
	{
	}

	// �f�X�g���N�^
public:
	~CPluginIfObj(){}

	// ����
public:
	void SetPlugIndex(int nIndex) { m_nPlugIndex = nIndex; }
	// ����
public:
	//�R�}���h�����擾����
	MacroFuncInfoArray GetMacroCommandInfo() const
	{
		return m_MacroFuncInfoArr;
	}
	//�֐������擾����
	MacroFuncInfoArray GetMacroFuncInfo() const
	{
		return m_MacroFuncInfoNotCommandArr;
	}
	//�֐�����������
	bool HandleFunction(CEditView* View, EFunctionCode ID, const VARIANT *Arguments, const int ArgSize, VARIANT &Result)
	{
		Variant varCopy;	// VT_BYREF���ƍ���̂ŃR�s�[�p

		switch(LOWORD(ID))
		{
		case F_PL_GETPLUGINDIR:			//�v���O�C���t�H���_�p�X���擾����
			{
				SysString S(m_cPlugin.m_sBaseDir.c_str(), m_cPlugin.m_sBaseDir.size());
				Wrap(&Result)->Receive(S);
			}
			return true;
		case F_PL_GETDEF:				//�ݒ�t�@�C������l��ǂ�
		case F_PL_GETOPTION:			//�I�v�V�����t�@�C������l��ǂ�
			{
				CDataProfile cProfile;
				wstring sSection;
				wstring sKey;
				wstring sValue;
				if( variant_to_wstr( Arguments[0], sSection ) != true) return false;
				if( variant_to_wstr( Arguments[1], sKey ) != true) return false;

				cProfile.SetReadingMode();
				if( LOWORD(ID) == F_PL_GETDEF ){
					cProfile.ReadProfile( m_cPlugin.GetPluginDefPath().c_str() );
				}else{
					cProfile.ReadProfile( m_cPlugin.GetOptionPath().c_str() );
				}
				cProfile.IOProfileData( sSection.c_str(), sKey.c_str(), sValue );

				SysString S(sValue.c_str(), sValue.size());
				Wrap(&Result)->Receive(S);
			}
			return true;
		case F_PL_GETCOMMANDNO:			//���s���v���O�̔ԍ����擾����
			{
				Wrap(&Result)->Receive(m_nPlugIndex);
			}
			return true;
		}
		return false;
	}
	//�R�}���h����������
	void HandleCommand(CEditView* View, EFunctionCode ID, const WCHAR* Arguments[], const int ArgSize)
	{
		switch ( LOWORD(ID) ) 
		{
		case F_PL_SETOPTION:			//�I�v�V�����t�@�C���ɒl������
			{
				CDataProfile cProfile;

				cProfile.ReadProfile( m_cPlugin.GetOptionPath().c_str() );
				cProfile.SetWritingMode();
				wstring tmp(Arguments[2]);
				cProfile.IOProfileData( Arguments[0], Arguments[1], tmp );
				cProfile.WriteProfile( m_cPlugin.GetOptionPath().c_str(), (m_cPlugin.m_sName + L" �v���O�C���ݒ�t�@�C��").c_str() );
			}
			break;
		case F_PL_ADDCOMMAND:			//�R�}���h��ǉ�����
			{
				int id = m_cPlugin.AddCommand( Arguments[0], Arguments[1], Arguments[2], true );
				View->m_pcEditWnd->RegisterPluginCommand( id );
			}
			break;
		}
	}

	// �����o�ϐ�
public:
private:
	CPlugin& m_cPlugin;
	static MacroFuncInfo m_MacroFuncInfoArr[];
	static MacroFuncInfo m_MacroFuncInfoNotCommandArr[];
	int m_nPlugIndex;	//���s���v���O�̔ԍ�
};

//�R�}���h���
MacroFuncInfo CPluginIfObj::m_MacroFuncInfoArr[] = 
{
	//ID									�֐���							����										�߂�l�̌^	m_pszData
	{EFunctionCode(F_PL_SETOPTION),			LTEXT("SetOption"),				{VT_BSTR, VT_BSTR, VT_VARIANT, VT_EMPTY},	VT_EMPTY,	NULL }, //�I�v�V�����t�@�C���ɒl������
	{EFunctionCode(F_PL_ADDCOMMAND),		LTEXT("AddCommand"),			{VT_BSTR, VT_BSTR, VT_EMPTY, VT_EMPTY},		VT_EMPTY,	NULL }, //�R�}���h��ǉ�����
	//	�I�[
	{F_INVALID,	NULL, {VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}
};

//�֐����
MacroFuncInfo CPluginIfObj::m_MacroFuncInfoNotCommandArr[] = 
{
	//ID									�֐���							����										�߂�l�̌^	m_pszData
	{EFunctionCode(F_PL_GETPLUGINDIR),		LTEXT("GetPluginDir"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_BSTR,	NULL }, //�v���O�C���t�H���_�p�X���擾����
	{EFunctionCode(F_PL_GETDEF),			LTEXT("GetDef"),				{VT_BSTR, VT_BSTR, VT_EMPTY, VT_EMPTY},		VT_BSTR,	NULL }, //�ݒ�t�@�C������l��ǂ�
	{EFunctionCode(F_PL_GETOPTION),			LTEXT("GetOption"),				{VT_BSTR, VT_BSTR, VT_EMPTY, VT_EMPTY},		VT_BSTR,	NULL }, //�I�v�V�����t�@�C������l��ǂ�
	{EFunctionCode(F_PL_GETCOMMANDNO),		LTEXT("GetCommandNo"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, //�I�v�V�����t�@�C������l��ǂ�
	//	�I�[
	{F_INVALID,	NULL, {VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}
};

#endif /* SAKURA_CPLUGINIFOBJ_2205DDE8_330B_49AC_AC5E_4C02F07DDCD5D_H_ */
/*[EOF]*/
