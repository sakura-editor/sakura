/*!	@file
	@brief PPA Library Handler

	PPA.DLL�𗘗p���邽�߂̃C���^�[�t�F�[�X

	@author YAZAKI
	@date 2002�N1��26��
*/
/*
	Copyright (C) 2001, YAZAKI
	Copyright (C) 2003, Moca

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

#include "CPPA.h"
#include "CEditView.h"
#include "Funccode.h"
#include "debug.h"
#include "CMacro.h"
#include "CSMacroMgr.h"// 2002/2/10 aroka
#include "CShareData.h"
#include "etc_uty.h"
#include "CEditApp.h"
#include "CEditDoc.h"	//	2002/5/13 YAZAKI �w�b�_����
#include "OleTypes.h"

#define NEVER_USED_PARAM(p) ((void)p)

CEditView*		CPPA::m_pcEditView = NULL;
DLLSHAREDATA*	CPPA::m_pShareData = NULL;

CPPA::CPPA()
{
}

CPPA::~CPPA()
{
	//	Apr. 15, 2002 genta cleanup�����ǉ�
	if( IsAvailable()){
		DeinitDll();
	}
}


//	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
void CPPA::Execute(CEditView* pcEditView )
{
	m_pcEditView = pcEditView;
	m_pShareData = CShareData::getInstance()->GetShareData();
	m_fnExecute();
}

char *
CPPA::GetDllName( char* str )
{
	return "PPA.DLL";
}

/*!
	DLL�̏�����

	�֐��̃A�h���X���擾���ă����o�ɕۊǂ���D

	@retval 0 ����
	@retval 1 �A�h���X�擾�Ɏ��s
*/
int CPPA::InitDll()
{
	/* PPA.DLL�������Ă���֐������� */

	//	Apr. 15, 2002 genta const��t����
	//	�A�h���X�̓���ꏊ�̓I�u�W�F�N�g�Ɉˑ�����̂�
	//	static�z��ɂ͂ł��Ȃ��B
	const ImportTable table[] = 
	{
		{ &m_fnExecute,		"Execute" },
		{ &m_fnSetDeclare,	"SetDeclare" },
		{ &m_fnSetSource,	"SetSource" },
		{ &m_fnSetDefProc,	"SetDefProc" },
		{ &m_fnSetDefine,	"SetDefine" },
		{ &m_fnSetIntFunc,	"SetIntFunc" },
		{ &m_fnSetStrFunc,	"SetStrFunc" },
		{ &m_fnSetProc,		"SetProc" },
		{ &m_fnSetErrProc,	"SetErrProc" },
		{ &m_fnAbort,		"ppaAbort" },
		{ &m_fnGetVersion,	"GetVersion" },
		{ &m_fnDeleteVar,	"DeleteVar" },
		{ &m_fnGetArgInt,	"GetArgInt" },
		{ &m_fnGetArgStr,	"GetArgStr" },
		{ &m_fnGetArgBStr,	"GetArgBStr" },
		{ &m_fnGetIntVar,	"GetIntVar" },
		{ &m_fnGetStrVar,	"GetStrVar" },
		{ &m_fnGetBStrVar,	"GetBStrVar" },
		{ &m_fnSetIntVar,	"SetIntVar" },
		{ &m_fnSetStrVar,	"SetStrVar" },
		{ &m_fnAddIntObj,	"AddIntObj" },
		{ &m_fnAddStrObj,	"AddStrObj" },
		{ &m_fnAddIntVar,	"AddIntVar" },
		{ &m_fnAddStrVar,	"AddStrVar" },
		{ &m_fnSetIntObj,	"SetIntObj" },
		{ &m_fnSetStrObj,	"SetStrObj" },

#if PPADLL_VER >= 120
		{ &m_fnAddRealVar,	"AddRealVar" },
		{ &m_fnSetRealObj,	"SetRealObj" },
		{ &m_fnAddRealObj,	"AddRealObj" },
		{ &m_fnGetRealVar,	"GetRealVar" },
		{ &m_fnSetRealVar,	"SetRealVar" },
		{ &m_fnSetRealFunc,	"SetRealFunc" },
		{ &m_fnGetArgReal,	"GetArgReal" },
#endif

#if PPADLL_VER >= 123
		{ &m_fnIsRunning, "IsRunning" },
#endif

		{ NULL, 0 }
	};

	//	Apr. 15, 2002 genta
	//	CDllHandler�̋��ʊ֐�������
	if( ! RegisterEntries(table) )
		return 1;

	SetIntFunc((void*)CPPA::stdIntFunc);	// 2003.02.24 Moca
	SetStrFunc((void*)CPPA::stdStrFunc);
	SetProc((void*)CPPA::stdProc);

	/* SAKURA�G�f�B�^�p�Ǝ��֐������� */

	int i;
	// �R�}���h�ɒu���������Ȃ��֐� �� PPA�����ł͎g���Ȃ��B�B�B
	for (i=0; CSMacroMgr::m_MacroFuncInfoNotCommandArr[i].m_pszFuncName != NULL; i++) {
		CSMacroMgr::m_MacroFuncInfoNotCommandArr[i].m_pszData = new char [ strlen(CSMacroMgr::m_MacroFuncInfoNotCommandArr[i].m_pszFuncName) + 256 ];	//	256�������v���X
		GetDeclarations( CSMacroMgr::m_MacroFuncInfoNotCommandArr[i] );	//	m_pszData���쐬����
		SetDefProc(CSMacroMgr::m_MacroFuncInfoNotCommandArr[i].m_pszData);
	}

	// �R�}���h�ɒu����������֐� �� PPA�����ł��g����B
	for (i=0; CSMacroMgr::m_MacroFuncInfoArr[i].m_pszFuncName != NULL; i++) {
		CSMacroMgr::m_MacroFuncInfoArr[i].m_pszData = new char [ strlen(CSMacroMgr::m_MacroFuncInfoArr[i].m_pszFuncName) + 256 ];	//	256�������v���X
		GetDeclarations( CSMacroMgr::m_MacroFuncInfoArr[i] );	//	m_pszData���쐬����
		SetDefProc(CSMacroMgr::m_MacroFuncInfoArr[i].m_pszData);
	}
	return 0; 
}

void CPPA::GetDeclarations( MacroFuncInfo& cMacroFuncInfo )
{
	char szBuffer[1024];	//	�Œ�1024�łǂ����H

	char szType[20];			//	procedure/function�p�o�b�t�@
	char szReturn[20];			//	�߂�l�^�p�o�b�t�@
	if (cMacroFuncInfo.m_varResult == VT_EMPTY){
		strcpy( szType, "procedure" );
		szReturn[0] = '\0';
	}
	else {
		strcpy( szType, "function" );
		if (cMacroFuncInfo.m_varResult == VT_BSTR){
			strcpy( szReturn, ": string" );
		}
		else if ( cMacroFuncInfo.m_varResult == VT_I4 ){
			strcpy( szReturn, ": Integer" );
		}
		else {
			szReturn[0] = '\0';
		}
	}
	
	char szArguments[4][20];	//	�����p�o�b�t�@
	int i;
	for (i=0; i<4; i++){
		if ( cMacroFuncInfo.m_varArguments[i] == VT_EMPTY ){
			break;
		}
		if ( cMacroFuncInfo.m_varArguments[i] == VT_BSTR ){
			strcpy( szArguments[i], "s0: string" );
			szArguments[i][1] = '0' + i;
		}
		else if ( cMacroFuncInfo.m_varArguments[i] == VT_I4 ){
			strcpy( szArguments[i], "i0: Integer" );
			szArguments[i][1] = '0' + i;
		}
		else {
			strcpy( szArguments[i], "u0: Unknown" );
		}
	}
	if ( i > 0 ){	//	�������������Ƃ�
		int j;
		char szArgument[80];
		// 2002.12.06 Moca �����s�������Cstrcat��VC6Pro�ł��܂������Ȃ��������߁Cstrcpy�ɂ��Ă݂��瓮����
		strcpy( szArgument, szArguments[0] );
		for ( j=1; j<i; j++){
			strcat( szArgument, "; " );
			strcat( szArgument, szArguments[j] );
		}
		sprintf( szBuffer, "%s S_%s(%s)%s; index %d;",
			szType,
			cMacroFuncInfo.m_pszFuncName,
			szArgument,
			szReturn,
			cMacroFuncInfo.m_nFuncID
		);
	}
	else {
		sprintf( szBuffer, "%s S_%s%s; index %d;",
			szType,
			cMacroFuncInfo.m_pszFuncName,
			szReturn,
			cMacroFuncInfo.m_nFuncID
		);
	}
	cMacroFuncInfo.m_pszData = new char [ lstrlen( szBuffer ) + 1 ];
	strcpy( cMacroFuncInfo.m_pszData, szBuffer );
}

/*!	
*/
int CPPA::DeinitDll( void )
{
	int i;
	for (i=0; CSMacroMgr::m_MacroFuncInfoArr[i].m_pszFuncName != NULL; i++){
		if (CSMacroMgr::m_MacroFuncInfoArr[i].m_pszData)
			delete CSMacroMgr::m_MacroFuncInfoArr[i].m_pszData;
	}
	for (i=0; CSMacroMgr::m_MacroFuncInfoNotCommandArr[i].m_pszFuncName != NULL; i++) {
		if (CSMacroMgr::m_MacroFuncInfoNotCommandArr[i].m_pszData)
			delete CSMacroMgr::m_MacroFuncInfoNotCommandArr[i].m_pszData;
	}
	return 0;
}


//----------------------------------------------------------------------
void __stdcall CPPA::stdProc(
	const char* FuncName, const int Index,
	const char* Argument[], const int ArgSize, int* Err_CD)
{
	NEVER_USED_PARAM(FuncName);

	*Err_CD = 0;
	CMacro::HandleCommand( m_pcEditView, Index, Argument, ArgSize );
}

//----------------------------------------------------------------------
/*!
	�����l��Ԃ��֐�����������

	PPA����Ăт������
	@author Moca
	@date 2003.02.24 Moca
*/
void __stdcall CPPA::stdIntFunc(
	const char* FuncName, const int Index,
	const char* Argument[], const int ArgSize, int* Err_CD,
	int* ResultValue)
{
	NEVER_USED_PARAM(FuncName);
	VARIANT Ret;
	::VariantInit(&Ret);

	*ResultValue = 0;
	*Err_CD = 0;
	if( false != CallHandleFunction(Index, Argument, ArgSize, &Ret) ){
		switch( Ret.vt ){
		case VT_I4:
			*ResultValue = Ret.lVal;
			break;
		case VT_INT:
			*ResultValue = Ret.intVal;
			break;
		case VT_UINT:
			*ResultValue = Ret.uintVal;
			break;
		default:
			*Err_CD = -1;
		}
		::VariantClear(&Ret);
		return;
	}
	::VariantClear(&Ret);
	*Err_CD = -1;
	return;
}

//----------------------------------------------------------------------
/*!
	�������Ԃ��֐�����������

	PPA����Ăт������
	@date 2003.02.24 Moca CallHandleFunction�Ή�
*/
void __stdcall CPPA::stdStrFunc(
	const char* FuncName, const int Index,
	const char* Argument[], const int ArgSize, int* Err_CD,
	char** ResultValue)
{
	NEVER_USED_PARAM(FuncName);
	static CMemory cMem; // ANSI������łȂ���΂Ȃ�Ȃ�
						// ����̊Ǘ�����|�C���^��PPA�ɓn���̂�static�ł���K�v������B
	VARIANT Ret;
	::VariantInit(&Ret);
	*Err_CD = 0;
	if( false != CallHandleFunction(Index, Argument, ArgSize, &Ret) ){
		if(VT_BSTR == Ret.vt){
			int len;
			char* buf;
			Wrap(&Ret.bstrVal)->Get(&buf,&len);
			cMem.SetData(buf,len); // Mar. 9, 2003 genta
			delete[] buf;
			*ResultValue = cMem.GetPtr();
			::VariantClear(&Ret);
			return;
		}
	}
	::VariantClear(&Ret);
	*Err_CD = -1;
	*ResultValue = "";
	return;
}

/*!
	�����^�ϊ�

	������ŗ^����ꂽ������VARIANT/BSTR�ɕϊ�����CMacro::HandleFunction()���Ăт���
	@author Moca
*/
bool CPPA::CallHandleFunction(
	const int Index, const char* Arg[],
	int ArgSize, VARIANT* Result )
{
	int i, ArgCnt;
	VARIANT vtArg[4];
	const MacroFuncInfo* mfi;
	bool Ret;

	mfi = CSMacroMgr::GetFuncInfoByID(Index);
	for( i=0; i<4; i++ ){
		::VariantInit( &vtArg[i] );
	}
	for(i=0, ArgCnt=0; i<4 && i<ArgSize; i++ ){
		if(VT_EMPTY == mfi->m_varArguments[i]){
			break;
		}

		switch( mfi->m_varArguments[i] ){
		case VT_I4:
		{
			vtArg[i].vt = VT_I4;
			vtArg[i].lVal = atoi( Arg[i] );
			break;
		}
		case VT_BSTR:
		{
			SysString S(Arg[i],lstrlen(Arg[i]));
			Wrap(&vtArg[i])->Receive(S);
			break;
		}
		default:
			for( i=0; i<4; i++ ){
				::VariantClear( &vtArg[i] );
			}
			return false;
		}
		ArgCnt++;
	}

	if(Index >= F_FUNCTION_FIRST)
	{
		Ret = CMacro::HandleFunction(m_pcEditView, Index, vtArg, ArgCnt, *Result);
		for( i=0; i<4; i++ ){
			::VariantClear( &vtArg[i] );
		}
		return Ret;
	}else{
		for( i=0; i<4; i++ ){
			::VariantClear( &vtArg[i] );
		}
		return false;
	}
}
