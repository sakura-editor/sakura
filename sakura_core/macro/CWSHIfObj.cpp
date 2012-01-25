/*!	@file
	@brief WSH�C���^�t�F�[�X�I�u�W�F�N�g��{�N���X

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

#include "StdAfx.h"
#include "macro/CWSHIfObj.h"
#include "macro/CSMacroMgr.h" // MacroFuncInfo
#include "Funccode_enum.h" // EFunctionCode::FA_FROMMACRO


//�R�}���h�E�֐�����������
void CWSHIfObj::ReadyMethods( CEditView* pView, int flags )
{
	this->m_pView = pView;
	//	 2007.07.20 genta : �R�}���h�ɍ������ރt���O��n��
	ReadyCommands(GetMacroCommandInfo(), flags | FA_FROMMACRO );
	ReadyCommands(GetMacroFuncInfo(), 0);
	/* CWSHIfObj���p�������T�u�N���X����ReadyMethods���Ăяo�����ꍇ�A
	 * �T�u�N���X��GetMacroCommandInfo,GetMacroFuncInfo���Ăяo�����B */
}

/** WSH�}�N���G���W���փR�}���h�o�^���s��

	@date 2007.07.20 genta flags�ǉ��Dflag�̓R�}���h�o�^�i�K�ō����Ă����D
*/
void CWSHIfObj::ReadyCommands(MacroFuncInfo *Info, int flags)
{
	while(Info->m_nFuncID != -1)	// Aug. 29, 2002 genta �Ԑl�̒l���ύX���ꂽ�̂ł������ύX
	{
		wchar_t FuncName[256];
		wcscpy(FuncName, Info->m_pszFuncName);

		int ArgCount = 0;
		for(int I = 0; I < 4; ++I)
			if(Info->m_varArguments[I] != VT_EMPTY) 
				++ArgCount;
		
		//	2007.07.21 genta : flag���������l��o�^����
		this->AddMethod(
			FuncName,
			(Info->m_nFuncID | flags),
			Info->m_varArguments,
			ArgCount,
			Info->m_varResult,
			reinterpret_cast<CIfObjMethod>(&CWSHIfObj::MacroCommand)
			/* CWSHIfObj���p�������T�u�N���X����ReadyCommands���Ăяo�����ꍇ�A
			 * �T�u�N���X��MacroCommand���Ăяo�����B */
		);
		
		++Info;
	}
}

/*!
	�}�N���R�}���h�̎��s

	@date 2005.06.27 zenryaku �߂�l�̎󂯎�肪�����Ă��G���[�ɂ����Ɋ֐������s����
*/
HRESULT CWSHIfObj::MacroCommand(int IntID, DISPPARAMS *Arguments, VARIANT* Result, void *Data)
{
	int I;
	int ArgCount = Arguments->cArgs;
	if(ArgCount > 4) ArgCount = 4;

	const EFunctionCode ID = static_cast<EFunctionCode>(IntID);
	//	2007.07.22 genta : �R�}���h�͉���16�r�b�g�̂�
	if(LOWORD(ID) >= F_FUNCTION_FIRST)
	{
		VARIANT ret; // 2005.06.27 zenryaku �߂�l�̎󂯎�肪�����Ă��֐������s����
		VariantInit(&ret);

		// 2011.3.18 syat �����̏����𐳂������ɂ���
		VARIANTARG rgvargBak[4];
		memcpy( rgvargBak, Arguments->rgvarg, sizeof(VARIANTARG) * ArgCount );
		for(I = 0; I < ArgCount; I++){
			Arguments->rgvarg[ArgCount-I-1] = rgvargBak[I];
		}

		// 2009.9.5 syat HandleFunction�̓T�u�N���X�ŃI�[�o�[���C�h����
		bool r = HandleFunction(m_pView, ID, Arguments->rgvarg, Arguments->cArgs, ret);
		if(Result) {::VariantCopyInd(Result, &ret);}
		VariantClear(&ret);
		return r ? S_OK : E_FAIL;
	}
	else
	{
		//	Nov. 29, 2005 FILE �����𕶎���Ŏ擾����
		WCHAR *StrArgs[4] = {NULL, NULL, NULL, NULL};	// �������K�{
		WCHAR *S = NULL;								// �������K�{
		Variant varCopy;							// VT_BYREF���ƍ���̂ŃR�s�[�p
		int Len;
		for(I = 0; I < ArgCount; ++I)
		{
			if(VariantChangeType(&varCopy.Data, &(Arguments->rgvarg[I]), 0, VT_BSTR) == S_OK)
			{
				Wrap(&varCopy.Data.bstrVal)->GetW(&S, &Len);
			}
			else
			{
				S = new WCHAR[1];
				S[0] = 0;
			}
			StrArgs[ArgCount - I - 1] = S;			// DISPPARAMS�͈����̏������t�]���Ă��邽�ߐ��������ɒ���
		}

		// 2009.10.29 syat HandleCommand�̓T�u�N���X�ŃI�[�o�[���C�h����
		HandleCommand(m_pView, ID, const_cast<WCHAR const **>(StrArgs), ArgCount);

		//	Nov. 29, 2005 FILE �z��̔j���Ȃ̂ŁA[����]��ǉ�
		for(int J = 0; J < ArgCount; ++J)
			delete [] StrArgs[J];

		return S_OK;
	}
}
