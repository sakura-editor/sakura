/*!	@file
	@brief PPA Library Handler

	PPA.DLLを利用するためのインターフェース

	@author YAZAKI
	@date 2002年1月26日
*/
/*
	Copyright (C) 2001, YAZAKI
	Copyright (C) 2002, YAZAKI, aroka, genta, Moca
	Copyright (C) 2003, Moca, genta
	Copyright (C) 2007, genta
	Copyright (C) 2008, syat

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
#include "CPPA.h"
#include "CEditView.h"
#include "Funccode.h"
#include "Debug.h"
#include "CMacro.h"
#include "CSMacroMgr.h"// 2002/2/10 aroka
#include "CShareData.h"
#include "etc_uty.h"
#include "CEditApp.h"
#include "CEditDoc.h"	//	2002/5/13 YAZAKI ヘッダ整理
#include "OleTypes.h"

#define NEVER_USED_PARAM(p) ((void)p)


// 2003.06.01 Moca
#define omGet (0)
#define omSet (1)

//	2007.07.26 genta
CPPA::PpaExecInfo* CPPA::m_CurInstance = NULL;
bool			CPPA::m_bIsRunning = false;

CPPA::CPPA()
{
}

CPPA::~CPPA()
{
}

//	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
void CPPA::Execute(CEditView* pcEditView, int flags )
{
	//PPAの多重起動禁止 2008.10.22 syat
	if ( CPPA::m_bIsRunning ) {
		MYMESSAGEBOX( pcEditView->m_hWnd, MB_OK, _T("PPA実行エラー"), _T("PPA実行中に新たにPPAマクロを呼び出すことはできません") );
		m_fnAbort();
		CPPA::m_bIsRunning = false;
		return;
	}
	CPPA::m_bIsRunning = true;

	PpaExecInfo info;
	info.m_pcEditView = pcEditView;
	info.m_pShareData = CShareData::getInstance()->GetShareData();
	info.m_bError = false;			//	2003.06.01 Moca
	info.m_cMemDebug.SetString("");	//	2003.06.01 Moca
	info.m_commandflags = flags | FA_FROMMACRO;	//	2007.07.22 genta
	
	//	実行前にインスタンスを待避する
	PpaExecInfo* old_instance = m_CurInstance;
	m_CurInstance = &info;
	m_fnExecute();
	
	//	マクロ実行完了後はここに戻ってくる
	m_CurInstance = old_instance;

	//PPAの多重起動禁止 2008.10.22 syat
	CPPA::m_bIsRunning = false;
}

LPCTSTR CPPA::GetDllName( LPCTSTR str )
{
	return _T("PPA.DLL");
}

/*!
	DLLの初期化

	関数のアドレスを取得してメンバに保管する．

	@retval true 成功
	@retval false アドレス取得に失敗
*/
bool CPPA::InitDll()
{
	/* PPA.DLLが持っている関数を準備 */

	//	Apr. 15, 2002 genta constを付けた
	//	アドレスの入れ場所はオブジェクトに依存するので
	//	static配列にはできない。
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
		{ &m_fnSetFinishProc, "SetFinishProc"}, // 2003.06.23 Moca
#endif

		{ NULL, 0 }
	};

	//	Apr. 15, 2002 genta
	//	CDllHandlerの共通関数化した
	if( ! RegisterEntries(table) )
		return false;

	SetIntFunc((void *)CPPA::stdIntFunc);	// 2003.02.24 Moca
	SetStrFunc((void *)CPPA::stdStrFunc);
	SetProc((void *)CPPA::stdProc);

	// 2003.06.01 Moca エラーメッセージを追加
	SetErrProc((void *)CPPA::stdError);
	SetStrObj((void *)CPPA::stdStrObj);	// UserErrorMes用
#if PPADLL_VER >= 123
	SetFinishProc((void *)CPPA::stdFinishProc);
#endif

	SetDefine( "sakura-editor" );	// 2003.06.01 Moca SAKURAエディタ用独自関数を準備
	AddStrObj( "UserErrorMes", "", FALSE, 2 ); // 2003.06.01 デバッグ用文字列変数を用意

	int i;
	
	//	Jun. 16, 2003 genta 一時作業エリア
	char buf[1024];
	// コマンドに置き換えられない関数 ＝ PPA無しでは使えない。。。
	for (i=0; CSMacroMgr::m_MacroFuncInfoArr[i].m_pszFuncName != NULL; i++) {
		//	2003.06.08 Moca メモリーリークの修正
		//	2003.06.16 genta バッファを外から与えるように
		//	関数登録用文字列を作成する
		GetDeclarations( CSMacroMgr::m_MacroFuncInfoArr[i], buf );
		SetDefProc( buf );
	}

	// コマンドに置き換えられる関数 ＝ PPA無しでも使える。
	for (i=0; CSMacroMgr::m_MacroFuncInfoCommandArr[i].m_pszFuncName != NULL; i++) {
		//	2003.06.08 Moca メモリーリークの修正
		//	2003.06.16 genta バッファを外から与えるように
		//	関数登録用文字列を作成する
		GetDeclarations( CSMacroMgr::m_MacroFuncInfoCommandArr[i], buf );
		SetDefProc( buf );
	}
	return true; 
}

/*! PPAに関数を登録するための文字列を作成する

	@param cMacroFuncInfo [in]	マクロデータ
	@param szBuffer [out]		生成した文字列を入れるバッファへのポインタ

	@note バッファサイズは 9 + 3 + メソッド名の長さ + 13 * 4 + 9 + 5 は最低必要

	@date 2003.06.01 Moca
				スタティックメンバに変更
				cMacroFuncInfo.m_pszDataを書き換えないように変更

	@date 2003.06.16 genta 無駄なnew/deleteを避けるためバッファを外から与えるように
*/
char* CPPA::GetDeclarations( const MacroFuncInfo& cMacroFuncInfo, char* szBuffer )
{
	char szType[20];			//	procedure/function用バッファ
	char szReturn[20];			//	戻り値型用バッファ
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
	
	char szArguments[4][20];	//	引数用バッファ
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
	if ( i > 0 ){	//	引数があったとき
		int j;
		char szArgument[80];
		// 2002.12.06 Moca 原因不明だが，strcatがVC6Proでうまく動かなかったため，strcpyにしてみたら動いた
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
	//	Jun. 01, 2003 Moca / Jun. 16, 2003 genta
	return szBuffer;
}



/*! ユーザー定義文字列型オブジェクト
	現在は、デバッグ用文字列を設定する為のみ
*/
void __stdcall CPPA::stdStrObj(const char* ObjName, int Index, BYTE GS_Mode, int* Err_CD, char** Value)
{
	NEVER_USED_PARAM(ObjName);
	*Err_CD = 0;
	switch(Index){
	case 2:
		switch(GS_Mode){
		case omGet:
			*Value = m_CurInstance->m_cMemDebug.GetStringPtr();
			break;
		case omSet:
			m_CurInstance->m_cMemDebug.SetString(*Value);
			break;
		}
		break;
	default:
		*Err_CD = -1;
	}
}


/*! ユーザー定義関数のエラーメッセージの作成

	stdProc, stdIntFunc, stdStrFunc がエラーコードを返した場合、PPAから呼び出される。
	異常終了/不正引数時のエラーメッセージを独自に指定する。
	@author Moca
	@param Err_CD IN  0以外各コールバック関数が設定した値
			 1以上 FuncID + 1
			 0     PPAのエラー
			-1以下 その他ユーザ定義エラー
	@param Err_Mes IN エラーメッセージ

	@date 2003.06.01 Moca
*/
void __stdcall CPPA::stdError( int Err_CD, const char* Err_Mes )
{
	if( false != m_CurInstance->m_bError ){
		return;
	}
	m_CurInstance->m_bError = true; // 関数内で関数を呼ぶ場合等、2回表示されるのを防ぐ

	char szMes[2048]; // 2048あれば足りるかと
	const char* pszErr;
	pszErr = szMes;
	if( 0 < Err_CD ){
		int i, FuncID;
		FuncID = Err_CD - 1;
		for( i = 0; CSMacroMgr::m_MacroFuncInfoArr[i].m_nFuncID != -1; i++ ){
			if( CSMacroMgr::m_MacroFuncInfoArr[i].m_nFuncID == FuncID ){
				break;
			}
		}
		if( CSMacroMgr::m_MacroFuncInfoArr[i].m_nFuncID != -1 ){
			char szFuncDec[1024];
			GetDeclarations( CSMacroMgr::m_MacroFuncInfoArr[i], szFuncDec );
			wsprintf( szMes, "関数の実行エラー\n%s", szFuncDec );
		}else{
			wsprintf( szMes, "不明な関数の実行エラー(バグです)\nFunc_ID=%d", FuncID );
		}
	}else{
		switch( Err_CD ){
		case 0:
			if( '\0' == Err_Mes[0] ){
				pszErr = "詳細不明のエラー";
			}else{
				pszErr = Err_Mes;
			}
			break;
		default:
			wsprintf( szMes, "未定義のエラー\nError_CD=%d\n%s", Err_CD, Err_Mes );
		}
	}
	//	2007.07.26 genta : ネスト実行した場合にPPAが不正なポインタを渡す可能性を考慮．
	//	実際には不正なエラーは全てPPA.DLL内部でトラップされるようだが念のため．
	if( IsBadStringPtr( pszErr, 256 )){
		pszErr = "エラー情報が不正";
	}
	if( 0 == m_CurInstance->m_cMemDebug.GetStringLength() ){
		MYMESSAGEBOX( m_CurInstance->m_pcEditView->m_hWnd, MB_OK, _T("PPA実行エラー"), _T("%s"), pszErr );
	}
	else{
		MYMESSAGEBOX( m_CurInstance->m_pcEditView->m_hWnd, MB_OK, _T("PPA実行エラー"), _T("%s\n%s"), pszErr, m_CurInstance->m_cMemDebug.GetStringPtr() );
	}
}



//----------------------------------------------------------------------
/** プロシージャ実行callback

	@date 2007.07.20 genta Indexと一緒にフラグを渡す
*/
void __stdcall CPPA::stdProc(
	const char*		FuncName,
	const int		Index,
	const char*		Argument[],
	const int		ArgSize,
	int*			Err_CD
)
{
	NEVER_USED_PARAM(FuncName);

	*Err_CD = 0;
	CMacro::HandleCommand( m_CurInstance->m_pcEditView, Index | m_CurInstance->m_commandflags, Argument, ArgSize );
}

//----------------------------------------------------------------------
/*!
	整数値を返す関数を処理する

	PPAから呼びだされる
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
			*Err_CD = -2; // 2003.06.01 Moca 値変更
		}
		::VariantClear(&Ret);
		return;
	}
	*Err_CD = Index + 1; // 2003.06.01 Moca
	::VariantClear(&Ret);
	return;
}

//----------------------------------------------------------------------
/*!
	文字列を返す関数を処理する

	PPAから呼びだされる
	@date 2003.02.24 Moca CallHandleFunction対応
*/
void __stdcall CPPA::stdStrFunc(
	const char* FuncName, const int Index,
	const char* Argument[], const int ArgSize, int* Err_CD,
	char** ResultValue)
{
	NEVER_USED_PARAM(FuncName);

	VARIANT Ret;
	::VariantInit(&Ret);
	*Err_CD = 0;
	if( false != CallHandleFunction(Index, Argument, ArgSize, &Ret) ){
		if(VT_BSTR == Ret.vt){
			int len;
			char* buf;
			Wrap(&Ret.bstrVal)->Get(&buf,&len);
			m_CurInstance->m_cMemRet.SetString(buf,len); // Mar. 9, 2003 genta
			delete[] buf;
			*ResultValue = m_CurInstance->m_cMemRet.GetStringPtr();
			::VariantClear(&Ret);
			return;
		}
	}
	::VariantClear(&Ret);
	*Err_CD = Index + 1;
	*ResultValue = const_cast<char*>("");
	return;
}

/*!
	引数型変換

	文字列で与えられた引数をVARIANT/BSTRに変換してCMacro::HandleFunction()を呼びだす
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
		Ret = CMacro::HandleFunction(m_CurInstance->m_pcEditView, Index, vtArg, ArgCnt, *Result);
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


#if PPADLL_VER >= 123

/*!
	PPAマクロの実行終了時に呼ばれる
	
	@date 2003.06.01 Moca
*/
void __stdcall CPPA::stdFinishProc()
{
	// 2007.07.26 genta : 終了処理は不要
}

#endif

/*[EOF]*/
