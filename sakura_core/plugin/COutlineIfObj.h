/*!	@file
	@brief Outline�I�u�W�F�N�g

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
#ifndef SAKURA_COUTLINEIFOBJ_5C2DD72B_8550_40AB_9C49_2B1C5D17C91FW_H_
#define SAKURA_COUTLINEIFOBJ_5C2DD72B_8550_40AB_9C49_2B1C5D17C91FW_H_

#include "macro/CWSHIfObj.h"

class COutlineIfObj : public CWSHIfObj {
	// �^��`
	enum FuncId {
		F_OL_COMMAND_FIRST = 0,					//���R�}���h�͈ȉ��ɒǉ�����
		F_OL_ADDFUNCINFO,						//�A�E�g���C����͂ɒǉ�����
		F_OL_ADDFUNCINFO2,						//�A�E�g���C����͂ɒǉ�����i�[���w��j
		F_OL_SETTITLE,							//�A�E�g���C���_�C�A���O�^�C�g�����w��
		F_OL_SETLISTTYPE,						//�A�E�g���C�����X�g��ʂ��w��
		F_OL_FUNCTION_FIRST = F_FUNCTION_FIRST	//���֐��͈ȉ��ɒǉ�����
	};
	typedef std::string string;
	typedef std::wstring wstring;

	// �R���X�g���N�^
public:
	COutlineIfObj( CFuncInfoArr& cFuncInfoArr )
		: CWSHIfObj( L"Outline" )
		, m_cFuncInfoArr( cFuncInfoArr )
		, m_nListType( OUTLINE_PLUGIN )
	{
	}

	// �f�X�g���N�^
public:
	~COutlineIfObj(){}

	// ����
public:
	//�R�}���h�����擾����
	MacroFuncInfoArray GetMacroCommandInfo() const{ return m_MacroFuncInfoArr; }
	//�֐������擾����
	MacroFuncInfoArray GetMacroFuncInfo() const{ return m_MacroFuncInfoNotCommandArr; };
	//�֐�����������
	bool HandleFunction(CEditView* View, EFunctionCode ID, const VARIANT *Arguments, const int ArgSize, VARIANT &Result)
	{
		return false;
	}
	//�R�}���h����������
	void HandleCommand(CEditView* View, EFunctionCode ID, const WCHAR* Arguments[], const int ArgSize)
	{
		switch ( LOWORD(ID) ) 
		{
		case F_OL_ADDFUNCINFO:			//�A�E�g���C����͂ɒǉ�����
		case F_OL_ADDFUNCINFO2:			//�A�E�g���C����͂ɒǉ�����i�[���w��j
			{
				if( Arguments[0] == NULL )break;
				if( Arguments[1] == NULL )break;
				if( Arguments[2] == NULL )break;
				if( Arguments[3] == NULL )break;
				CLogicPoint ptLogic( _wtoi(Arguments[1])-1, _wtoi(Arguments[0])-1 );
				if( ptLogic.x < 0 )ptLogic.x = 0;
				if( ptLogic.y < 0 )ptLogic.y = 0;
				CLayoutPoint ptLayout;
				View->GetDocument()->m_cLayoutMgr.LogicToLayout( ptLogic, &ptLayout );
				int nParam = _wtoi(Arguments[3]);
				if( LOWORD(ID) == F_OL_ADDFUNCINFO ){
					m_cFuncInfoArr.AppendData( ptLogic.GetY()+1, ptLogic.GetX()+1, ptLayout.GetY()+1, ptLayout.GetX()+1, Arguments[2], nParam );
				}else{
					int nDepth = nParam & FUNCINFO_INFOMASK;
					nParam -= nDepth;
					m_cFuncInfoArr.AppendData( ptLogic.GetY()+1, ptLogic.GetX()+1, ptLayout.GetY()+1, ptLayout.GetX()+1, Arguments[2], nParam, nDepth );
				}
			}
			break;
		case F_OL_SETTITLE:				//�A�E�g���C���_�C�A���O�^�C�g�����w��
			if( Arguments[0] == NULL )break;
			m_sOutlineTitle = to_tchar( Arguments[0] );
			break;
		case F_OL_SETLISTTYPE:			//�A�E�g���C�����X�g��ʂ��w��
			if( Arguments[0] == NULL )break;
			m_nListType = (EOutlineType)_wtol(Arguments[0]);
			break;
		}
	}

	// �����o�ϐ�
public:
	tstring m_sOutlineTitle;
	EOutlineType m_nListType;
private:
	CFuncInfoArr& m_cFuncInfoArr;
	static MacroFuncInfo m_MacroFuncInfoArr[];
	static MacroFuncInfo m_MacroFuncInfoNotCommandArr[];
};

//�R�}���h���
MacroFuncInfo COutlineIfObj::m_MacroFuncInfoArr[] = 
{
	//ID									�֐���							����										�߂�l�̌^	m_pszData
	{EFunctionCode(F_OL_ADDFUNCINFO),		LTEXT("AddFuncInfo"),			{VT_I4, VT_I4, VT_BSTR, VT_I4},				VT_EMPTY,	NULL }, //�A�E�g���C����͂ɒǉ�����
	{EFunctionCode(F_OL_ADDFUNCINFO2),		LTEXT("AddFuncInfo2"),			{VT_I4, VT_I4, VT_BSTR, VT_I4},				VT_EMPTY,	NULL }, //�A�E�g���C����͂ɒǉ�����i�[���w��j
	{EFunctionCode(F_OL_SETTITLE),			LTEXT("SetTitle"),				{VT_BSTR, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL },	//�A�E�g���C���_�C�A���O�^�C�g�����w��
	{EFunctionCode(F_OL_SETLISTTYPE),		LTEXT("SetListType"),			{VT_I4, VT_EMPTY, VT_EMPTY, VT_EMPTY},		VT_EMPTY,	NULL }, //�A�E�g���C�����X�g��ʂ��w��
	//	�I�[
	{F_INVALID,	NULL, {VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}
};

//�֐����
MacroFuncInfo COutlineIfObj::m_MacroFuncInfoNotCommandArr[] = 
{
	//ID									�֐���							����										�߂�l�̌^	m_pszData
	//	�I�[
	{F_INVALID,	NULL, {VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}
};

#endif /* SAKURA_COUTLINEIFOBJ_5C2DD72B_8550_40AB_9C49_2B1C5D17C91FW_H_ */
/*[EOF]*/
