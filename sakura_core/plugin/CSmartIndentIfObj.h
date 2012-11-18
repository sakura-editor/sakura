/*!	@file
	@brief SmartIndent�I�u�W�F�N�g

*/
/*
	Copyright (C) 2009, syat
	Copyright (C) 2010, syat

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
#ifndef SAKURA_CSMARTINDENTIFOBJ_5708577D_5A1D_4374_91FD_5E180C9F4E20_H_
#define SAKURA_CSMARTINDENTIFOBJ_5708577D_5A1D_4374_91FD_5E180C9F4E20_H_

#include "macro/CWSHIfObj.h"

// �X�}�[�g�C���f���g�pWSH�I�u�W�F�N�g
class CSmartIndentIfObj : public CWSHIfObj
{
	// �^��`
	enum FuncId {
		F_SI_COMMAND_FIRST = 0,					//���R�}���h�͈ȉ��ɒǉ�����
		F_SI_FUNCTION_FIRST = F_FUNCTION_FIRST,	//���֐��͈ȉ��ɒǉ�����
		F_SI_GETCHAR							//���������L�[���擾����
	};
	typedef std::string string;
	typedef std::wstring wstring;

	// �R���X�g���N�^
public:
	CSmartIndentIfObj( wchar_t ch )
		: CWSHIfObj( L"Indent", false )
		, m_wcChar( ch )
	{
	}

	// �f�X�g���N�^
public:
	~CSmartIndentIfObj(){}

	// ����
public:
	//�R�}���h�����擾����
	MacroFuncInfoArray GetMacroCommandInfo() const{
		static MacroFuncInfo macroFuncInfoArr[] = {
			//	�I�[
			{F_INVALID,	NULL, {VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}
		};
		return macroFuncInfoArr;
	}
	//�֐������擾����
	MacroFuncInfoArray GetMacroFuncInfo() const{
		static MacroFuncInfo macroFuncInfoNotCommandArr[] = {
			//ID									�֐���							����										�߂�l�̌^	m_pszData
			{EFunctionCode(F_SI_GETCHAR),			LTEXT("GetChar"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_BSTR,	NULL }, //�I�v�V�����t�@�C���ɒl������
			//	�I�[
			{F_INVALID,	NULL, {VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}
		};
		return macroFuncInfoNotCommandArr;
	}
	//�֐�����������
	bool HandleFunction(CEditView* View, EFunctionCode ID, const VARIANT *Arguments, const int ArgSize, VARIANT &Result)
	{
		switch ( LOWORD(ID) ) 
		{
		case F_SI_GETCHAR:						//���������L�[���擾����
			{
				wstring sValue;
				sValue += m_wcChar;
				SysString S(sValue.c_str(), sValue.size());
				Wrap(&Result)->Receive(S);
			}
			return true;
		}
		return false;
	}
	//�R�}���h����������
	void HandleCommand(CEditView* View, EFunctionCode ID, const WCHAR* Arguments[], const int ArgSize)
	{
		return;
	}

	// �����o�ϐ�
public:
	wchar_t m_wcChar;
};

#endif /* SAKURA_CSMARTINDENTIFOBJ_5708577D_5A1D_4374_91FD_5E180C9F4E20_H_ */
