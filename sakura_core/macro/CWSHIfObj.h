/*!	@file
	@brief WSH�C���^�t�F�[�X�I�u�W�F�N�g��{�N���X

	@date 2009.10.29 syat CWSH.h����؂�o��

*/
/*
	Copyright (C) 2002, �S, genta
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
#ifndef SAKURA_CWSHIFOBJ_EAA6C3E3_1442_4940_B8A3_2AAB324D8788D_H_
#define SAKURA_CWSHIFOBJ_EAA6C3E3_1442_4940_B8A3_2AAB324D8788D_H_
#include <list>
#include "OleTypes.h"
#include "macro/CIfObj.h"
#include "macro/CWSH.h" // CWSHClient::List, ListIter
class CEditView;
#include <activscp.h>
#include "CSMacroMgr.h" // MacroFuncInfo, MacroFuncInfoArray


class CWSHIfObj
: public CIfObj
{
public:
	// �^��`
	typedef std::list<CWSHIfObj*> List;
	typedef List::const_iterator ListIter;

	// �R���X�g���N�^
	CWSHIfObj(const wchar_t* name, bool isGlobal)
	: CIfObj(name, isGlobal)
	{}

	virtual void ReadyMethods( CEditView* pView, int flags );

protected:
	// ����
	//	2007.07.20 genta : flags�ǉ�
	//  2009.09.05 syat CWSHManager����ړ�
	void ReadyCommands(MacroFuncInfo *Info, int flags);
	HRESULT MacroCommand(int ID, DISPPARAMS *Arguments, VARIANT* Result, void *Data);

	// �������
	virtual bool HandleFunction(CEditView* View, EFunctionCode ID, const VARIANT *Arguments, const int ArgSize, VARIANT &Result) = 0;	//�֐�����������
	virtual void HandleCommand(CEditView* View, EFunctionCode ID, const WCHAR* Arguments[], const int ArgSize) = 0;	//�R�}���h����������
	virtual MacroFuncInfoArray GetMacroCommandInfo() const = 0;	//�R�}���h�����擾����
	virtual MacroFuncInfoArray GetMacroFuncInfo() const = 0;	//�֐������擾����

	CEditView* m_pView;
};


#endif /* SAKURA_CWSHIFOBJ_EAA6C3E3_1442_4940_B8A3_2AAB324D8788D_H_ */
/*[EOF]*/
