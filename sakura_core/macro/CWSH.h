/*!	@file
	@brief WSH Handler

	@author �S
	@date 2002�N4��28��,5��3��,5��5��,5��6��,5��13��,5��16��
	@date 2002.08.25 genta �����N�G���[����̂���CWSHManager.h�ɃG�f�B�^��
		�}�N���C���^�[�t�F�[�X���𕪗��D
	@date 2009.10.29 syat �C���^�t�F�[�X�I�u�W�F�N�g������CWSHIfObj.h�ɕ���
*/
/*
	Copyright (C) 2002, �S, genta
	Copyright (C) 2009, syat

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.

*/

#ifndef __WSH_H__
#define __WSH_H__

#include <vector>
//#include <windows.h>
//#include <winreg.h>
//#include <objbase.h>
//#include <initguid.h>
#include "activscp.h"
//��Microsoft Platform SDK ���
#include "macro/CIfObj.h"

/* 2009.10.29 syat �C���^�t�F�[�X�I�u�W�F�N�g������CWSHIfObj.h�ɕ���
template<class Base>
class ImplementsIUnknown: public Base

class CInterfaceObject: public ImplementsIUnknown<IDispatch>
 */
typedef void (*ScriptErrorHandler)(BSTR Description, BSTR Source, void *Data);

class CWSHClient : IWSHClient
{
public:
	// �^��`
	typedef std::vector<CIfObj*> List;      // ���L���Ă���C���^�t�F�[�X�I�u�W�F�N�g�̃��X�g
	typedef List::const_iterator ListIter;	// ���̃C�e���[�^

	// �R���X�g���N�^�E�f�X�g���N�^
	CWSHClient(wchar_t const *AEngine, ScriptErrorHandler AErrorHandler, void *AData);
	~CWSHClient();

	// �t�B�[���h�E�A�N�Z�T
	ScriptErrorHandler m_OnError;
	void *m_Data;
	bool m_Valid; ///< true�̏ꍇ�X�N���v�g�G���W�����g�p�\�Bfalse�ɂȂ�ꍇ�� ScriptErrorHandler�ɃG���[���e���ʒm����Ă���B
	virtual /*override*/ void* GetData() const { return this->m_Data; }
	const List& GetInterfaceObjects() {	return this->m_IfObjArr; }

	// ����
	void AddInterfaceObject( CIfObj* obj );
	void Execute(wchar_t const *AScript);
	void Error(BSTR Description, BSTR Source); ///< ScriptErrorHandler���Ăяo���B
	void Error(wchar_t* Description);          ///< ScriptErrorHandler���Ăяo���B

private:
	IActiveScript *m_Engine;
	List m_IfObjArr;
};


#endif
