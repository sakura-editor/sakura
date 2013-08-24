/*!	@file
	@brief WSH Manager

	@date 2009.10.29 syat CWSH.cppから切り出し
*/
/*
	Copyright (C) 2002, 鬼, genta
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
#include "CWSH.h"
#include "CWSHManager.h"
#include "CMacroFactory.h"
#include "CMacro.h"
#include "CSMacroMgr.h"
#include "CEditView.h"
#include "CEditDoc.h"
#include "os.h"
#include "OleTypes.h"

static void MacroError(BSTR Description, BSTR Source, void *Data)
{
	CEditView *View = reinterpret_cast<CEditView*>(Data);

	char MessageA[1024], SourceA[1024];
	
	MessageA[WideCharToMultiByte(CP_ACP, 0, Description, SysStringLen(Description), MessageA, 1023, NULL, NULL)] = 0;
	SourceA[WideCharToMultiByte(CP_ACP, 0, Source, SysStringLen(Source), SourceA, 1023, NULL, NULL)] = 0;
	
	MessageBox(View->m_hWnd, MessageA, SourceA, MB_ICONERROR);
}

CWSHMacroManager::CWSHMacroManager(std::wstring const AEngineName) : m_EngineName(AEngineName)
{
}

CWSHMacroManager::~CWSHMacroManager()
{
}

/** WSHマクロの実行

	@param EditView [in] 操作対象EditView
	
	@date 2007.07.20 genta : flags追加
*/
void CWSHMacroManager::ExecKeyMacro(CEditView *EditView, int flags) const
{
	CWSHClient* Engine;
	Engine = new CWSHClient(m_EngineName.c_str(), MacroError, EditView);
	if(Engine->m_Valid)
	{
/* // CSMacroMgr.hで配列のサイズが明確に宣言されて無いのでsizeofが使えない
		Engine->m_InterfaceObject->ReserveMethods(
						sizeof (CSMacroMgr::m_MacroFuncInfoCommandArr) / sizeof (CSMacroMgr::m_MacroFuncInfoCommandArr[0]) +
						sizeof (CSMacroMgr::m_MacroFuncInfoArr) / sizeof (CSMacroMgr::m_MacroFuncInfoArr[0]));
*/
		//	 2007.07.20 genta : コマンドに混ぜ込むフラグを渡す
		ReadyCommands(Engine->m_InterfaceObject, CSMacroMgr::m_MacroFuncInfoCommandArr, flags | FA_FROMMACRO );
		ReadyCommands(Engine->m_InterfaceObject, CSMacroMgr::m_MacroFuncInfoArr, 0);
		
		Engine->Execute(m_Source.c_str());
		
		//EditView->Redraw();
		EditView->ShowEditCaret();
	}
	delete Engine;
}

/*!
	WSHマクロの読み込み（ファイルから）

	@param hInstance [in] インスタンスハンドル(未使用)
	@param pszPath   [in] ファイルのパス
*/
BOOL CWSHMacroManager::LoadKeyMacro(HINSTANCE hInstance, const TCHAR* pszPath)
{
	BOOL Result = FALSE;
	
	HANDLE File = CreateFile(pszPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if(File != INVALID_HANDLE_VALUE)
	{
		unsigned long Size = GetFileSize(File, NULL); //ギガ単位のマクロはさすがに無いでしょう…
		char *Buffer = new char[Size];
		wchar_t *WideBuffer = new wchar_t[Size + 1]; //Unicode化して長くはならない

		if(ReadFile(File, Buffer, Size, &Size, NULL) != 0)
		{
			WideBuffer[MultiByteToWideChar(CP_ACP, 0, Buffer, Size, WideBuffer, Size)] = 0;
			m_Source = WideBuffer;
			Result = TRUE;
		}
		//	Nov. 10, 2003 FILE 配列の破棄なので、[括弧]を追加
		delete [] Buffer;
		delete [] WideBuffer;
		CloseHandle(File);
	}
	return Result;
}

/*!
	WSHマクロの読み込み（文字列から）

	@param hInstance [in] インスタンスハンドル(未使用)
	@param pszCode   [in] マクロコード
*/
BOOL CWSHMacroManager::LoadKeyMacroStr(HINSTANCE hInstance, const TCHAR* pszCode)
{
	unsigned long Size = strlen(pszCode); //ギガ単位のマクロはさすがに無いでしょう…
	wchar_t *WideBuffer = new wchar_t[Size + 1]; //Unicode化して長くはならない

	WideBuffer[MultiByteToWideChar(CP_ACP, 0, pszCode, Size, WideBuffer, Size)] = 0;
	m_Source = WideBuffer;

	delete [] WideBuffer;

	return TRUE;
}

CMacroManagerBase* CWSHMacroManager::Creator(const TCHAR* FileExt)
{
	TCHAR FileExtWithDot[1024], FileType[1024], EngineName[1024]; //1024を超えたら後は知りません
	
	_tcscpy( FileExtWithDot, _T(".") );
	_tcscat( FileExtWithDot, FileExt );

	if(ReadRegistry(HKEY_CLASSES_ROOT, FileExtWithDot, NULL, FileType, 1024))
	{
		lstrcat(FileType, _T("\\ScriptEngine"));
		if(ReadRegistry(HKEY_CLASSES_ROOT, FileType, NULL, EngineName, 1024))
		{
			wchar_t EngineNameW[1024];
			MultiByteToWideChar(CP_ACP, 0, EngineName, -1, EngineNameW, _countof(EngineNameW));
			return new CWSHMacroManager(EngineNameW);
		}
	}
	return NULL;
}

void CWSHMacroManager::declare()
{
	//暫定
	CMacroFactory::getInstance()->RegisterCreator(Creator);
}

/////////////////////////////////////////////
/*!
	マクロコマンドの実行

	@date 2005.06.27 zenryaku 戻り値の受け取りが無くてもエラーにせずに関数を実行する
*/
static HRESULT MacroCommand(int ID, DISPPARAMS *Arguments, VARIANT* Result, void *Data)
{
	int I;
	int ArgCount = Arguments->cArgs;
	if(ArgCount > 4) ArgCount = 4;

	CEditView *View = reinterpret_cast<CEditView*>(Data);
	//	2007.07.22 genta : コマンドは下位16ビットのみ
	if(LOWORD(ID) >= F_FUNCTION_FIRST)
	{
		VARIANT ret; // 2005.06.27 zenryaku 戻り値の受け取りが無くても関数を実行する
		VariantInit(&ret);
		
		// 2011.3.18 syat 引数の順序を正しい順にする
		VARIANTARG rgvargBak[4];
		memcpy( rgvargBak, Arguments->rgvarg, sizeof(VARIANTARG) * ArgCount );
		for(I = 0; I < ArgCount; I++){
			Arguments->rgvarg[ArgCount-I-1] = rgvargBak[I];
		}

		bool r = CMacro::HandleFunction(View, ID, Arguments->rgvarg, Arguments->cArgs, ret);
		if(Result) {::VariantCopyInd(Result, &ret);}
		VariantClear(&ret);
		return r ? S_OK : E_FAIL;
	}
	else
	{
		//	Nov. 29, 2005 FILE 引数を文字列で取得する
		char *StrArgs[4] = {NULL, NULL, NULL, NULL};	// 初期化必須
		char *S = NULL;									// 初期化必須
		Variant varCopy;										// VT_BYREFだと困るのでコピー用
		int Len;
		for(I = 0; I < ArgCount; ++I)
		{
			if(VariantChangeType(&varCopy.Data, &(Arguments->rgvarg[I]), 0, VT_BSTR) == S_OK)
			{
				Wrap(&varCopy.Data.bstrVal)->Get(&S, &Len);
			}
			else
			{
				S = new char[1];
				S[0] = 0;
			}
			StrArgs[ArgCount - I - 1] = S;
		}

		CMacro::HandleCommand(View, ID, const_cast<char const **>(StrArgs), ArgCount);

		//	Nov. 29, 2005 FILE 配列の破棄なので、[括弧]を追加
		for(int J = 0; J < ArgCount; ++J)
			delete [] StrArgs[J];

		return S_OK;
	}
}

/** WSHマクロエンジンへコマンド登録を行う

	@date 2007.07.20 genta flags追加．flagはコマンド登録段階で混ぜておく．
*/
void CWSHMacroManager::ReadyCommands(CInterfaceObject *Object, MacroFuncInfo *Info, int flags)
{
	while(Info->m_nFuncID != -1)	// Aug. 29, 2002 genta 番人の値が変更されたのでここも変更
	{
		wchar_t FuncName[256];
		MultiByteToWideChar(CP_ACP, 0, Info->m_pszFuncName, -1, FuncName, 255);

		int ArgCount = 0;
		for(int I = 0; I < 4; ++I)
			if(Info->m_varArguments[I] != VT_EMPTY) 
				++ArgCount;
		
		//	2007.07.21 genta : flagを加えた値を登録する
		Object->AddMethod(FuncName, Info->m_nFuncID | flags, Info->m_varArguments, ArgCount, Info->m_varResult, MacroCommand);
		
		++Info;
	}
}

