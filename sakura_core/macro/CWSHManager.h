/*!	@file
	@brief WSH Manager

	@author �S, genta
	@date 2002�N4��28��,5��3��,5��5��,5��6��,5��13��,5��16��
	@date 2002.08.25 genta CWSH.h��蕪��

	@par TODO
	@li ���m�̃G���W���ɑΉ��ł���悤CMacroFactory��ύX �� �v�c�_
	@li CEditView::HandleCommand���g�� �� CMacro::HandleCommand�ł��Ȃɂ�����Ă�悤�Ȃ̂ł�����Ȃ������H
	@li vector::reserve���g�� �� CSMacroMgr�Ō����錾����Ė����̂Ō�����
	@li �ĕ`��̑����ShowEditCaret �� protected�ł���[
*/
/*
	Copyright (C) 2002, �S, genta

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.

*/

#ifndef __CWSH_Manager_H__
#define __CWSH_Manager_H__

#include <windows.h>
#include <string>
#include "macro/CSMacroMgr.h"
#include "CMacroManagerBase.h"
class CEditView;
class CInterfaceObject;

typedef void (*EngineCallback)(wchar_t *Ext, char *EngineName);

class CWSHMacroManager: public CMacroManagerBase
{
public:
	CWSHMacroManager(std::wstring const AEngineName);
	virtual ~CWSHMacroManager();

	virtual void ExecKeyMacro(CEditView *EditView) const;
	virtual BOOL LoadKeyMacro(HINSTANCE Instance, const TCHAR* Path);

	static CMacroManagerBase* Creator(const TCHAR* FileExt);
	static void declare();
	static void EnumEngines(EngineCallback Proc); //�X�N���v�g�G���W����
protected:
	std::wstring m_Source;
	std::wstring m_EngineName;
	static void ReadyCommands(CInterfaceObject *Object, MacroFuncInfo *Info);
};
#endif
