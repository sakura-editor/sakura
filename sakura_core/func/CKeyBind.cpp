/*!	@file
	@brief キー割り当てに関するクラス

	@author Norio Nakatani
	@date 1998/03/25 新規作成
	@date 1998/05/16 クラス内にデータを持たないように変更
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, jepro, genta
	Copyright (C) 2002, YAZAKI, aroka
	Copyright (C) 2007, ryoji
	Copyright (C) 2008, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "stdafx.h"
#include "func/CKeyBind.h"
#include "debug/Debug.h"
#include "macro/CSMacroMgr.h"// 2002/2/10 aroka
#include "func/CFuncLookup.h"
#include "mem/CMemory.h"// 2002/2/10 aroka


//実装補助
/* KEYDATA配列にデータをセット */
static void SetKeyNameArrVal(
	DLLSHAREDATA*	pShareData,
	int				nIdx,
	short			nKeyCode,
	TCHAR*			pszKeyName,
	EFunctionCode	nFuncCode_0,
	EFunctionCode	nFuncCode_1,
	EFunctionCode	nFuncCode_2,
	EFunctionCode	nFuncCode_3,
	EFunctionCode	nFuncCode_4,
	EFunctionCode	nFuncCode_5,
	EFunctionCode	nFuncCode_6,
	EFunctionCode	nFuncCode_7
);


CKeyBind::CKeyBind()
{
}


CKeyBind::~CKeyBind()
{
}




/*! Windows アクセラレータの作成
	@date 2007.02.22 ryoji デフォルト機能割り当てに関する処理を追加
*/
HACCEL CKeyBind::CreateAccerelator(
		int			nKeyNameArrNum,
		KEYDATA*	pKeyNameArr
)
{
	ACCEL*	pAccelArr;
	HACCEL	hAccel;
	int		j, k;

	// 機能が割り当てられているキーの数をカウント -> nAccelArrNum
	int nAccelArrNum = 0;
	for( int i = 0; i < nKeyNameArrNum; ++i ){
		if( 0 != pKeyNameArr[i].m_nKeyCode ){
			for( j = 0; j < 8; ++j ){
				if( 0 != GetFuncCodeAt( pKeyNameArr[i], j ) ){
					nAccelArrNum++;
				}
			}
		}
	}
//	nAccelArrNum = nKeyNameArrNum * 8;


	if( nAccelArrNum <= 0 ){
		/* 機能割り当てがゼロ */
		return NULL;
	}
	pAccelArr = new ACCEL[nAccelArrNum];
	k = 0;
	for( int i = 0; i < nKeyNameArrNum; ++i ){
		if( 0 != pKeyNameArr[i].m_nKeyCode ){
			for( j = 0; j < 8; ++j ){
				if( 0 != GetFuncCodeAt( pKeyNameArr[i], j ) ){
					pAccelArr[k].fVirt = FNOINVERT | FVIRTKEY;;
					pAccelArr[k].key = pKeyNameArr[i].m_nKeyCode;
					pAccelArr[k].cmd = pKeyNameArr[i].m_nKeyCode | (((WORD)j)<<8) ;
					if( j & _SHIFT ){
						pAccelArr[k].fVirt |= FSHIFT;
					}
					if( j & _CTRL ){
						pAccelArr[k].fVirt |= FCONTROL;
					}
					if( j & _ALT ){
						pAccelArr[k].fVirt |= FALT;
					}
					k++;
				}
			}
		}
	}
	hAccel = ::CreateAcceleratorTable( pAccelArr, nAccelArrNum );
	delete [] pAccelArr;
	return hAccel;
}






/*! アクラセレータ識別子に対応するコマンド識別子を返す．
	対応するアクラセレータ識別子がない場合または機能未割り当ての場合は0を返す．

	@date 2007.02.22 ryoji デフォルト機能割り当てに関する処理を追加
*/
EFunctionCode CKeyBind::GetFuncCode(
		WORD		nAccelCmd,
		int			nKeyNameArrNum,
		KEYDATA*	pKeyNameArr,
		BOOL		bGetDefFuncCode /* = TRUE */
)
{
	int nCmd = (int)( nAccelCmd & 0x00ff );
	int nSts = (int)( ( nAccelCmd & 0xff00 ) >> 8 );
	for( int i = 0; i < nKeyNameArrNum; ++i ){
		if( nCmd == pKeyNameArr[i].m_nKeyCode ){
			return GetFuncCodeAt( pKeyNameArr[i], nSts, bGetDefFuncCode );
		}
	}
	return F_DEFAULT;
}






/*!
	@return 機能が割り当てられているキーストロークの数
	
	@date Oct. 31, 2001 genta 動的な機能名に対応するため引数追加
	@date 2007.02.22 ryoji デフォルト機能割り当てに関する処理を追加
*/
int CKeyBind::CreateKeyBindList(
	HINSTANCE		hInstance,		//!< [in] インスタンスハンドル
	int				nKeyNameArrNum,	//!< [in]
	KEYDATA*		pKeyNameArr,	//!< [out]
	CNativeW&		cMemList,		//!<
	CFuncLookup*	pcFuncLookup,	//!< [in] 機能番号→名前の対応を取る
	BOOL			bGetDefFuncCode //!< [in] ON:デフォルト機能割り当てを使う/OFF:使わない デフォルト:TRUE
)
{
	int		i;
	int		j;
	int		nValidKeys;
	WCHAR	pszStr[256];
	WCHAR	szFuncName[256];
	WCHAR	szFuncNameJapanese[256];

	nValidKeys = 0;
//	cMemList = "";
//	cMemList.SetData( "", strlen( "" ) );
	cMemList.SetString(LTEXT(""));
	const WCHAR*	pszSHIFT = LTEXT("Shift+");
	const WCHAR*	pszCTRL  = LTEXT("Ctrl+");
	const WCHAR*	pszALT   = LTEXT("Alt+");
//	char*	pszEQUAL = " = ";
	const WCHAR*	pszTAB   = LTEXT("\t");

//	char*	pszCR = "\n";	//Feb. 17, 2001 JEPRO \n=0x0a=LFが行末コードになってしまうので
	const WCHAR*	pszCR    = LTEXT("\r\n");	//\r=0x0d=CRを追加


	cMemList.AppendString( LTEXT("キー\t機能名\t関数名\t機能番号\tキーマクロ記録可/不可") );
	cMemList.AppendString( pszCR );
	cMemList.AppendString( LTEXT("-----\t-----\t-----\t-----\t-----") );
	cMemList.AppendString( pszCR );

	for( j = 0; j < 8; ++j ){
		for( i = 0; i < nKeyNameArrNum; ++i ){
			int iFunc = GetFuncCodeAt( pKeyNameArr[i], j, bGetDefFuncCode );

			if( 0 != iFunc ){
				nValidKeys++;
				if( j & _SHIFT ){
					cMemList.AppendString( pszSHIFT );
				}
				if( j & _CTRL ){
					cMemList.AppendString( pszCTRL );
				}
				if( j & _ALT ){
					cMemList.AppendString( pszALT );
				}
				cMemList.AppendString( to_wchar(pKeyNameArr[i].m_szKeyName) );
//				cMemList.AppendString( pszEQUAL );
//				cMemList.AppendString( pszTAB );
				//	Oct. 31, 2001 genta 
				if( !pcFuncLookup->Funccode2Name(
					iFunc,
					szFuncNameJapanese, 255 )){
					auto_strcpy( szFuncNameJapanese, LTEXT("---名前が定義されていない-----") );
				}
				auto_strcpy( szFuncName, LTEXT("")/*"---unknown()--"*/ );

//				/* 機能名日本語 */
//				::LoadString(
//					hInstance,
//					pKeyNameArr[i].m_nFuncCodeArr[j],
//					 szFuncNameJapanese, 255
//				);
				cMemList.AppendString( pszTAB );
				cMemList.AppendString( szFuncNameJapanese );

				/* 機能ID→関数名，機能名日本語 */
				//@@@ 2002.2.2 YAZAKI マクロをCSMacroMgrに統一
//				CMacro::GetFuncInfoByID(
				CSMacroMgr::GetFuncInfoByID(
					hInstance,
					iFunc,
					szFuncName,
					szFuncNameJapanese
				);

				/* 関数名 */
				cMemList.AppendString( pszTAB );
				cMemList.AppendString( szFuncName );

				/* 機能番号 */
				cMemList.AppendString( pszTAB );
				auto_sprintf( pszStr, LTEXT("%d"), iFunc );
				cMemList.AppendString( pszStr );

				/* キーマクロに記録可能な機能かどうかを調べる */
				cMemList.AppendString( pszTAB );
				//@@@ 2002.2.2 YAZAKI マクロをCSMacroMgrに統一
//				if( CMacro::CanFuncIsKeyMacro( pKeyNameArr[i].m_nFuncCodeArr[j] ) ){
				if( CSMacroMgr::CanFuncIsKeyMacro( iFunc ) ){
					cMemList.AppendString( LTEXT("○") );
				}else{
					cMemList.AppendString( LTEXT("×") );
				}



				cMemList.AppendString( pszCR );
			}
		}
	}
//	delete [] pszStr;
	return nValidKeys;
}

/** 機能に対応するキー名のサーチ(補助関数)

	与えられたシフト状態に対して，指定された範囲のキーエリアから
	当該機能に対応するキーがあるかを調べ，見つかったら
	対応するキー文字列をセットする．
	
	関数から出るときには検索開始位置(nKeyNameArrBegin)に
	次に処理するindexを設定する．

	@param[in,out] nKeyNameArrBegin 調査開始INDEX (終了時には次回の開始INDEXに書き換えられる)
	@param[in] nKeyNameArrBegin 調査終了INDEX + 1
	@param[in] pKeyNameArr キー配列
	@param[in] nShiftState シフト状態
	@param[out] cMemList キー文字列設定先
	@param[in]	nFuncId 検索対象機能ID
	@param[in]	bGetDefFuncCode 標準機能を取得するかどうか
*/
bool CKeyBind::GetKeyStrSub(
		int&		nKeyNameArrBegin,
		int			nKeyNameArrEnd,
		KEYDATA*	pKeyNameArr,
		int			nShiftState,
		CNativeT&	cMemList,
		int			nFuncId,
		BOOL		bGetDefFuncCode /* = TRUE */
)
{
	const TCHAR*	pszSHIFT = _T("Shift+");
	const TCHAR*	pszCTRL  = _T("Ctrl+");
	const TCHAR*	pszALT   = _T("Alt+");

	int i;
	for( i = nKeyNameArrBegin; i < nKeyNameArrEnd; ++i ){
		if( nFuncId == GetFuncCodeAt( pKeyNameArr[i], nShiftState, bGetDefFuncCode ) ){
			if( nShiftState & _SHIFT ){
				cMemList.AppendString( pszSHIFT );
			}
			if( nShiftState & _CTRL ){
				cMemList.AppendString( pszCTRL );
			}
			if( nShiftState & _ALT ){
				cMemList.AppendString( pszALT );
			}
			cMemList.AppendString( pKeyNameArr[i].m_szKeyName );
			nKeyNameArrBegin = i + 1;
			return true;
		}
	}
	nKeyNameArrBegin = i;
	return false;
}


/** 機能に対応するキー名の取得
	@date 2007.02.22 ryoji デフォルト機能割り当てに関する処理を追加
	@date 2007.11.04 genta マウスクリックよりキー割り当ての優先度を上げる
	@date 2007.11.04 genta 共通機能のサブルーチン化
*/
int CKeyBind::GetKeyStr(
		HINSTANCE	hInstance,
		int			nKeyNameArrNum,
		KEYDATA*	pKeyNameArr,
		CNativeT&	cMemList,
		int			nFuncId,
		BOOL		bGetDefFuncCode /* = TRUE */
)
{
	int		i;
	int		j;
	const TCHAR*	pszSHIFT = _T("Shift+");
	const TCHAR*	pszCTRL  = _T("Ctrl+");
	const TCHAR*	pszALT   = _T("Alt+");
	cMemList.SetString(_T(""));

	//	先にキー部分を調査する
	for( j = 0; j < 8; ++j ){
		for( i = MOUSEFUNCTION_KEYBEGIN; i < nKeyNameArrNum; /* 1を加えてはいけない */ ){
			if( GetKeyStrSub( i, nKeyNameArrNum, pKeyNameArr, j, cMemList, nFuncId, bGetDefFuncCode )){
				return 1;
			}
		}
	}

	//	後にマウス部分を調査する
	for( j = 0; j < 8; ++j ){
		for( i = 0; i < MOUSEFUNCTION_KEYBEGIN; /* 1を加えてはいけない */ ){
			if( GetKeyStrSub( i, nKeyNameArrNum, pKeyNameArr, j, cMemList, nFuncId, bGetDefFuncCode )){
				return 1;
			}
		}
	}
	return 0;
}


/** 機能に対応するキー名の取得(複数)
	@date 2007.02.22 ryoji デフォルト機能割り当てに関する処理を追加
	@date 2007.11.04 genta 共通機能のサブルーチン化
*/
int CKeyBind::GetKeyStrList(
	HINSTANCE	hInstance,
	int			nKeyNameArrNum,
	KEYDATA*	pKeyNameArr,
	CNativeT***	pppcMemList,
	int			nFuncId,
	BOOL		bGetDefFuncCode /* = TRUE */
)
{
	int		i;
	int		j;
	const TCHAR*	pszSHIFT = _T("Shift+");
	const TCHAR*	pszCTRL  = _T("Ctrl+");
	const TCHAR*	pszALT   = _T("Alt+");
	int		nAssignedKeysNum;

	nAssignedKeysNum = 0;
	if( 0 == nFuncId ){
		return 0;
	}
	for( j = 0; j < 8; ++j ){
		for( i = 0; i < nKeyNameArrNum; ++i ){
			if( nFuncId == GetFuncCodeAt( pKeyNameArr[i], j, bGetDefFuncCode ) ){
				nAssignedKeysNum++;
			}
		}
	}
	if( 0 == nAssignedKeysNum ){
		return 0;
	}
	(*pppcMemList) = new CNativeT*[nAssignedKeysNum + 1];
	for( i = 0; i < nAssignedKeysNum; ++i ){
		(*pppcMemList)[i] = new CNativeT;
	}
	(*pppcMemList)[i] = NULL;


	nAssignedKeysNum = 0;
	for( j = 0; j < 8; ++j ){
		for( i = 0; i < nKeyNameArrNum; /* 1を加えてはいけない */ ){
			//	2007.11.04 genta 共通機能のサブルーチン化
			if( GetKeyStrSub( i, nKeyNameArrNum, pKeyNameArr, j,
					*((*pppcMemList)[nAssignedKeysNum]), nFuncId, bGetDefFuncCode )){
				nAssignedKeysNum++;
			}
		}
	}
	return nAssignedKeysNum;
}


/*! メニューラベルの作成
	@date 2007.02.22 ryoji デフォルト機能割り当てに関する処理を追加
*/
TCHAR* CKeyBind::GetMenuLabel(
		HINSTANCE	hInstance,
		int			nKeyNameArrNum,
		KEYDATA*	pKeyNameArr,
		int			nFuncId,
		TCHAR*		pszLabel,
		BOOL		bKeyStr,
		BOOL		bGetDefFuncCode /* = TRUE */
)
{
	CNativeT		cMemList;
//	int			i;


	if( 0 == _tcslen( pszLabel ) ){
		_tcscpy( pszLabel, _T("-- undefined name --") );
		::LoadString( hInstance, nFuncId, pszLabel, 255 );
	}


	/* 機能に対応するキー名を追加するか */
	if( bKeyStr ){
		/* 機能に対応するキー名の取得 */
		if( ( IDM_SELWINDOW <= nFuncId && nFuncId <= IDM_SELWINDOW + 999 )
		 || ( IDM_SELMRU <= nFuncId && nFuncId <= IDM_SELMRU + 999 )
		 || ( IDM_SELOPENFOLDER <= nFuncId && nFuncId <= IDM_SELOPENFOLDER + 999 )
		 ){
		}else{
			_tcscat( pszLabel, _T("\t") );
		}

		if( GetKeyStr( hInstance, nKeyNameArrNum, pKeyNameArr, cMemList, nFuncId, bGetDefFuncCode ) ){
			_tcscat( pszLabel, cMemList.GetStringPtr() );
		}
	}
	return pszLabel;
}


/*! キーのデフォルト機能を取得する

	@param nKeyCode [in] キーコード
	@param nState [in] Shift,Ctrl,Altキー状態

	@return 機能番号

	@date 2007.02.22 ryoji 新規作成
*/
EFunctionCode CKeyBind::GetDefFuncCode( int nKeyCode, int nState )
{
	DLLSHAREDATA* pShareData = CShareData::getInstance()->GetShareData();
	if( pShareData == NULL )
		return F_DEFAULT;

	EFunctionCode nDefFuncCode = F_DEFAULT;
	if( nKeyCode == VK_F4 ){
		if( nState == _CTRL ){
			nDefFuncCode = F_FILECLOSE;	// 閉じて(無題)
			if( pShareData->m_Common.m_sTabBar.m_bDispTabWnd && !pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin ){
				nDefFuncCode = F_WINCLOSE;	// 閉じる
			}
		}
		else if( nState == _ALT ){
			nDefFuncCode = F_WINCLOSE;	// 閉じる
			if( pShareData->m_Common.m_sTabBar.m_bDispTabWnd && !pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin ){
				if( !pShareData->m_Common.m_sTabBar.m_bTab_CloseOneWin ){
					nDefFuncCode = F_GROUPCLOSE;	// グループを閉じる	// 2007.06.20 ryoji
				}
			}
		}
	}
	return nDefFuncCode;
}


/*! 特定のキー情報から機能コードを取得する

	@param KeyData [in] キー情報
	@param nState [in] Shift,Ctrl,Altキー状態
	@param bGetDefFuncCode [in] デフォルト機能を取得するかどうか

	@return 機能番号

	@date 2007.03.07 ryoji インライン関数から通常の関数に変更（BCCの最適化バグ対策）
*/
EFunctionCode CKeyBind::GetFuncCodeAt( KEYDATA& KeyData, int nState, BOOL bGetDefFuncCode )
{
	if( 0 != KeyData.m_nFuncCodeArr[nState] )
		return KeyData.m_nFuncCodeArr[nState];
	if( bGetDefFuncCode )
		return GetDefFuncCode( KeyData.m_nKeyCode, nState );
	return F_DEFAULT;
};















//	Sep. 14, 2000 JEPRO
//	Shift+F1 に「コマンド一覧」, Alt+F1 に「ヘルプ目次」, Shift+Alt+F1 に「キーワード検索」を追加	//Nov. 25, 2000 JEPRO 殺していたのを修正・復活
//Dec. 25, 2000 JEPRO Shift+Ctrl+F1 に「バージョン情報」を追加
// 2001.12.03 hor F2にブックマーク関連を割当
//Sept. 21, 2000 JEPRO	Ctrl+F3 に「検索マークのクリア」を追加
//Aug. 12, 2002 ai	Ctrl+Shift+F3 に「検索開始位置へ戻る」を追加
//Oct. 7, 2000 JEPRO	Alt+F4 に「ウィンドウを閉じる」, Shift+Alt+F4 に「すべてのウィンドウを閉じる」を追加
//	Ctrl+F4に割り当てられていた「縦横に分割」を「閉じて(無題)」に変更し Shift+Ctrl+F4 に「閉じて開く」を追加
//Jan. 14, 2001 Ctrl+Alt+F4 に「テキストエディタの全終了」を追加
//Jun. 2001「サクラエディタの全終了」に改称
//2006.10.21 ryoji Alt+F4 には何も割り当てない（デフォルトのシステムコマンド「閉じる」が実行されるように）
//2007.02.13 ryoji Shift+Ctrl+F4をF_WIN_CLOSEALLからF_EXITALLEDITORSに変更
//2007.02.22 ryoji Ctrl+F4 への割り当てを削除（デフォルトのコマンドを実行）
//	Sep. 20, 2000 JEPRO Ctrl+F5 に「外部コマンド実行」を追加  なおマクロ名はCMMAND からCOMMAND に変更済み
//Oct. 28, 2000 F5 は「再描画」に変更	//Jan. 14, 2001 Alt+F5 に「uudecodeして保存」, Ctrl+ Alt+F5 に「TAB→空白」を追加
//	May 28, 2001 genta	S-C-A-F5にSPACE-to-TABを追加
//Jan. 14, 2001 JEPRO	Ctrl+F6 に「小文字」, Alt+F6 に「Base64デコードして保存」を追加
// 2007.11.15 nasukoji	トリプルクリック・クアドラプルクリック対応
//Jan. 14, 2001 JEPRO	Ctrl+F7 に「大文字」, Alt+F7 に「UTF-7→SJISコード変換」, Shift+Alt+F7 に「SJIS→UTF-7コード変換」, Ctrl+Alt+F7 に「UTF-7で開き直す」を追加
//Nov. 9, 2000 JEPRO	Shift+F8 に「CRLF改行でコピー」を追加
//Jan. 14, 2001 JEPRO	Ctrl+F8 に「全角→半角」, Alt+F8 に「UTF-8→SJISコード変換」, Shift+Alt+F8 に「SJIS→UTF-8コード変換」, Ctrl+Alt+F8 に「UTF-8で開き直す」を追加
//Jan. 14, 2001 JEPRO	Ctrl+F9 に「半角＋全ひら→全角・カタカナ」, Alt+F9 に「Unicode→SJISコード変換」, Ctrl+Alt+F9 に「Unicodeで開き直す」を追加
//Oct. 28, 2000 JEPRO F10 に「SQL*Plusで実行」を追加(F5からの移動)
//Jan. 14, 2001 JEPRO	Ctrl+F10 に「半角＋全カタ→全角・ひらがな」, Alt+F10 に「EUC→SJISコード変換」, Shift+Alt+F10 に「SJIS→EUCコード変換」, Ctrl+Alt+F10 に「EUCで開き直す」を追加
//Jan. 14, 2001 JEPRO	Shift+F11 に「SQL*Plusをアクティブ表示」, Ctrl+F11 に「半角カタカナ→全角カタカナ」, Alt+F11 に「E-Mail(JIS→SJIS)コード変換」, Shift+Alt+F11 に「SJIS→JISコード変換」, Ctrl+Alt+F11 に「JISで開き直す」を追加
//Jan. 14, 2001 JEPRO	Ctrl+F12 に「半角カタカナ→全角ひらがな」, Alt+F12 に「自動判別→SJISコード変換」, Ctrl+Alt+F11 に「SJISで開き直す」を追加
//Sept. 1, 2000 JEPRO	Alt+Enter に「ファイルのプロパティ」を追加	//Oct. 15, 2000 JEPRO Ctrl+Enter に「ファイル内容比較」を追加
//Oct. 7, 2000 JEPRO 長いので名称を簡略形に変更(BackSpace→BkSp)
//Oct. 7, 2000 JEPRO 名称をVC++に合わせ簡略形に変更(Insert→Ins)
//Oct. 7, 2000 JEPRO 名称をVC++に合わせ簡略形に変更(Delete→Del)
//Jun. 26, 2001 JEPRO	Shift+Del に「切り取り」を追加
//Oct. 7, 2000 JEPRO	Shift+Ctrl+Alt+↑に「縦方向に最大化」を追加
//Jun. 27, 2001 JEPRO
//	Ctrl+↑に割り当てられていた「カーソル上移動(２行ごと)」を「テキストを１行下へスクロール」に変更
//2001.02.10 by MIK Shift+Ctrl+Alt+→に「横方向に最大化」を追加
//Sept. 14, 2000 JEPRO
//	Ctrl+↓に割り当てられていた「右クリックメニュー」を「カーソル下移動(２行ごと)」に変更
//	それに付随してさらに「右クリックメニュー」をCtrl＋Alt＋↓に変更
//Jun. 27, 2001 JEPRO
//	Ctrl+↓に割り当てられていた「カーソル下移動(２行ごと)」を「テキストを１行上へスクロール」に変更
//Oct. 15, 2000 JEPRO Ctrl+PgUp, Shift+Ctrl+PgDn にそれぞれ「１ページダウン」, 「(選択)１ページダウン」を追加
//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に交換(RollUp→PgDn) //Oct. 10, 2000 JEPRO 名称変更
//2001.12.03 hor 1Page/HalfPage 入替え
//Oct. 15, 2000 JEPRO Ctrl+PgUp, Shift+Ctrl+PgDn にそれぞれ「１ページアップ」, 「(選択)１ページアップ」を追加
//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に交換(RollDown→PgUp) //Oct. 10, 2000 JEPRO 名称変更
//2001.12.03 hor 1Page/HalfPage 入替え
//Oct. 7, 2000 JEPRO 名称をVC++に合わせ簡略形に変更(SpaceBar→Space)
//Oct. 7, 2000 JEPRO	Ctrl+0 を「タイプ別設定一覧」→「未定義」に変更
//Jan. 13, 2001 JEPRO	Alt+0 に「カスタムメニュー10」, Shift+Alt+0 に「カスタムメニュー20」を追加
//Oct. 7, 2000 JEPRO	Ctrl+1 を「タイプ別設定」→「ツールバーの表示」に変更
//Jan. 13, 2001 JEPRO	Alt+1 に「カスタムメニュー1」, Shift+Alt+1 に「カスタムメニュー11」を追加
//Jan. 19, 2001 JEPRO	Shift+Ctrl+1 に「カスタムメニュー21」を追加
//Oct. 7, 2000 JEPRO	Ctrl+2 を「共通設定」→「ファンクションキーの表示」に変更
//Jan. 13, 2001 JEPRO	Alt+2 を「アウトプット」→「カスタムメニュー2」に変更し「アウトプット」は Alt+O に移動, Shift+Alt+2 に「カスタムメニュー12」を追加
//Jan. 19, 2001 JEPRO	Shift+Ctrl+2 に「カスタムメニュー22」を追加
//Oct. 7, 2000 JEPRO	Ctrl+3 を「フォント設定」→「ステータスバーの表示」に変更
//Jan. 13, 2001 JEPRO	Alt+3 に「カスタムメニュー3」, Shift+Alt+3 に「カスタムメニュー13」を追加
//Jan. 19, 2001 JEPRO	Shift+Ctrl+3 に「カスタムメニュー23」を追加
//Oct. 7, 2000 JEPRO	Ctrl+4 を「ツールバーの表示」→「タイプ別設定一覧」に変更
//Jan. 13, 2001 JEPRO	Alt+4 に「カスタムメニュー4」, Shift+Alt+4 に「カスタムメニュー14」を追加
//Jan. 19, 2001 JEPRO	Shift+Ctrl+4 に「カスタムメニュー24」を追加
//Oct. 7, 2000 JEPRO	Ctrl+5 を「ファンクションキーの表示」→「タイプ別設定」に変更
//Jan. 13, 2001 JEPRO	Alt+5 に「カスタムメニュー5」, Shift+Alt+5 に「カスタムメニュー15」を追加
//Oct. 7, 2000 JEPRO	Ctrl+6 を「ステータスバーの表示」→「共通設定」に変更
//Jan. 13, 2001 JEPRO	Alt+6 に「カスタムメニュー6」, Shift+Alt+6 に「カスタムメニュー16」を追加
//Oct. 7, 2000 JEPRO	Ctrl+7 に「フォント設定」を追加
//Jan. 13, 2001 JEPRO	Alt+7 に「カスタムメニュー7」, Shift+Alt+7 に「カスタムメニュー17」を追加
//Jan. 13, 2001 JEPRO	Alt+8 に「カスタムメニュー8」, Shift+Alt+8 に「カスタムメニュー18」を追加
//Jan. 13, 2001 JEPRO	Alt+9 に「カスタムメニュー9」, Shift+Alt+9 に「カスタムメニュー19」を追加
//2001.12.06 hor Alt+A を「SORT_ASC」に割当
//Jan. 13, 2001 JEPRO	Ctrl+B に「ブラウズ」を追加
//Jan. 16, 2001 JEPRO	SHift+Ctrl+C に「.hと同名の.c(なければ.cpp)を開く」を追加
//Feb. 07, 2001 JEPRO	SHift+Ctrl+C を「.hと同名の.c(なければ.cpp)を開く」→「同名のC/C++ヘッダ(ソース)を開く」に変更
//Jan. 16, 2001 JEPRO	Ctrl+D に「単語切り取り」, Shift+Ctrl+D に「単語削除」を追加
//2001.12.06 hor Alt+D を「SORT_DESC」に割当
//Oct. 7, 2000 JEPRO	Ctrl+Alt+E に「重ねて表示」を追加
//Jan. 16, 2001	JEPRO	Ctrl+E に「行切り取り(折り返し単位)」, Shift+Ctrl+E に「行削除(折り返し単位)」を追加
//Oct. 07, 2000 JEPRO	Ctrl+Alt+H に「上下に並べて表示」を追加
//Jan. 16, 2001 JEPRO	Ctrl+H を「カーソル前を削除」→「カーソル行をウィンドウ中央へ」に変更し	Shift+Ctrl+H に「.cまたは.cppと同名の.hを開く」を追加
//Feb. 07, 2001 JEPRO	SHift+Ctrl+H を「.cまたは.cppと同名の.hを開く」→「同名のC/C++ヘッダ(ソース)を開く」に変更
//Jan. 21, 2001	JEPRO	Ctrl+I に「行の二重化」を追加
//Jan. 16, 2001	JEPRO	Ctrl+K に「行末まで切り取り(改行単位)」, Shift+Ctrl+E に「行末まで削除(改行単位)」を追加
//Jan. 14, 2001 JEPRO	Ctrl+Alt+L に「小文字」, Shift+Ctrl+Alt+L に「大文字」を追加
//Jan. 16, 2001 Ctrl+L を「カーソル行をウィンドウ中央へ」→「キーマクロの読み込み」に変更し「カーソル行をウィンドウ中央へ」は Ctrl+H に移動
//2001.12.03 hor Alt+L を「LTRIM」に割当
//Jan. 16, 2001 JEPRO	Ctrl+M に「キーマクロの保存」を追加
//2001.12.06 hor Alt+M を「MERGE」に割当
//Oct. 20, 2000 JEPRO	Alt+N に「移動履歴: 次へ」を追加
//Jan. 13, 2001 JEPRO	Alt+O に「アウトプット」を追加
//Oct. 7, 2000 JEPRO	Ctrl+P に「印刷」, Shift+Ctrl+P に「印刷プレビュー」, Ctrl+Alt+P に「ページ設定」を追加
//Oct. 20, 2000 JEPRO	Alt+P に「移動履歴: 前へ」を追加
//Jan. 24, 2001	JEPRO	Ctrl+Q に「キー割り当て一覧をコピー」を追加
//2001.12.03 hor Alt+R を「RTRIM」に割当
//Oct. 7, 2000 JEPRO	Shift+Ctrl+S に「名前を付けて保存」を追加
//Oct. 7, 2000 JEPRO	Ctrl+Alt+T に「左右に並べて表示」を追加
//Jan. 21, 2001	JEPRO	Ctrl+T に「タグジャンプ」, Shift+Ctrl+T に「タグジャンプバック」を追加
//Oct. 7, 2000 JEPRO	Ctrl+Alt+U に「現在のウィンドウ幅で折り返し」を追加
//Jan. 16, 2001	JEPRO	Ctrl+U に「行頭まで切り取り(改行単位)」, Shift+Ctrl+U に「行頭まで削除(改行単位)」を追加
//Jan. 13, 2001 JEPRO	Alt+X を「カスタムメニュー1」→「未定義」に変更し「カスタムメニュー1」は Alt+1 に移動
//Oct. 7, 2000 JEPRO	Shift+Ctrl+- に「上下に分割」を追加
// 2002.02.08 hor Ctrl+-にファイル名をコピーを追加
//Oct. 7, 2000 JEPRO	Shift+Ctrl+\ に「左右に分割」を追加
//Sept. 20, 2000 JEPRO	Ctrl+@ に「ファイル内容比較」を追加  //Oct. 15, 2000 JEPRO「選択範囲内全行コピー」に変更
//	Aug. 16, 2000 genta
//	反対向きの括弧にも括弧検索を追加
//Oct. 7, 2000 JEPRO	Shift+Ctrl+; に「縦横に分割」を追加	//Jan. 16, 2001	Alt+; に「日付挿入」を追加
//Sept. 14, 2000 JEPRO	Ctrl+: に「選択範囲内全行行番号付きコピー」を追加	//Jan. 16, 2001	Alt+: に「時刻挿入」を追加
//Sept. 14, 2000 JEPRO	Ctrl+. に「選択範囲内全行引用符付きコピー」を追加
//	Nov. 15, 2000 genta PC/ATキーボードに合わせてキーコードを変更
//	PC98救済のため，従来のキーコードに対応する項目を追加．
//Oct. 7, 2000 JEPRO	長くて表示しきれない所がでてきてしまうのでアプリケーションキー→アプリキーに短縮
//2008.05.03 kobake 可読性が著しく低下していたので、書式を整理。
// 2008.05.30 nasukoji	Ctrl+Alt+S に「指定桁で折り返す」を追加
// 2008.05.30 nasukoji	Ctrl+Alt+W に「右端で折り返す」を追加
// 2008.05.30 nasukoji	Ctrl+Alt+X に「折り返さない」を追加

/*!	@brief 共有メモリ初期化/キー割り当て

	デフォルトキー割り当て関連の初期化処理

	@date 2005.01.30 genta CShareData::Init()から分離
	@date 2007.11.04 genta キー設定数がDLLSHAREの領域を超えたら起動できないように
*/
bool CShareData::InitKeyAssign(DLLSHAREDATA* pShareData)
{
	/********************/
	/* 共通設定の規定値 */
	/********************/
	struct KEYDATAINIT {
		short			nKeyCode;		//!< Key Code (0 for non-keybord button)
		TCHAR*			pszKeyName;		//!< Key Name (for display)
		EFunctionCode	nFuncCode_0;	//!<                      Key
		EFunctionCode	nFuncCode_1;	//!< Shift +              Key
		EFunctionCode	nFuncCode_2;	//!<         Ctrl +       Key
		EFunctionCode	nFuncCode_3;	//!< Shift + Ctrl +       Key
		EFunctionCode	nFuncCode_4;	//!<                Alt + Key
		EFunctionCode	nFuncCode_5;	//!< Shift +        Alt + Key
		EFunctionCode	nFuncCode_6;	//!<         Ctrl + Alt + Key
		EFunctionCode	nFuncCode_7;	//!< Shift + Ctrl + Alt + Key
	};
#define _SQL_RUN	F_PLSQL_COMPILE_ON_SQLPLUS
#define _COPYWITHLINENUM	F_COPYLINESWITHLINENUMBER
	static KEYDATAINIT	KeyDataInit[] = {
	//Sept. 1, 2000 Jepro note: key binding
	//Feb. 17, 2001 jepro note 2: 順番は2進で下位3ビット[Alt][Ctrl][Shift]の組合せの順(それに2を加えた値)
	//		0,		1,		 2(000), 3(001),4(010),	5(011),		6(100),	7(101),		8(110),		9(111)

		/* マウスボタン */
		//keycode,	keyname,			なし,			Shitf+,				Ctrl+,					Shift+Ctrl+,		Alt+,					Shit+Alt+,			Ctrl+Alt+,				Shift+Ctrl+Alt+
		{ 0,_T("ダブルクリック"),		F_SELECTWORD,	F_SELECTWORD,		F_SELECTWORD,			F_SELECTWORD,		F_SELECTWORD,			F_SELECTWORD,		F_SELECTWORD,			F_SELECTWORD }, //Feb. 19, 2001 JEPRO Altと右クリックの組合せは効かないので右クリックメニューのキー割り当てをはずした
		{ 0,_T("右クリック"),			F_MENU_RBUTTON,	F_MENU_RBUTTON,		F_MENU_RBUTTON,			F_MENU_RBUTTON,		F_0,					F_0,				F_0,					F_0 },
		{ 0,_T("中クリック"),			F_0,			F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 }, // novice 2004/10/11 マウス中ボタン対応
		{ 0,_T("左サイドクリック"),		F_0,			F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 }, // novice 2004/10/10 マウスサイドボタン対応
		{ 0,_T("右サイドクリック"),		F_0,			F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 },
		{ 0,_T("トリプルクリック"),		F_SELECTLINE,	F_SELECTLINE,		F_SELECTLINE,			F_SELECTLINE,		F_SELECTLINE,			F_SELECTLINE,		F_SELECTLINE,			F_SELECTLINE },
		{ 0,_T("クアドラプルクリック"),	F_SELECTALL,	F_SELECTALL,		F_SELECTALL,			F_SELECTALL,		F_SELECTALL,			F_SELECTALL,		F_SELECTALL,			F_SELECTALL },

		/* ファンクションキー */
		//keycode,	keyname,			なし,			Shitf+,				Ctrl+,					Shift+Ctrl+,		Alt+,					Shit+Alt+,			Ctrl+Alt+,				Shift+Ctrl+Alt+
		{ VK_F1,	_T("F1" ),			F_EXTHTMLHELP,	F_MENU_ALLFUNC,		F_EXTHELP1,				F_ABOUT,			F_HELP_CONTENTS,		F_HELP_SEARCH,		F_0,					F_0 },
		{ VK_F2,	_T("F2" ),			F_BOOKMARK_NEXT,F_BOOKMARK_PREV,	F_BOOKMARK_SET,			F_BOOKMARK_RESET,	F_BOOKMARK_VIEW,		F_0,				F_0,					F_0 },
		{ VK_F3,	_T("F3" ),			F_SEARCH_NEXT,	F_SEARCH_PREV,		F_SEARCH_CLEARMARK,		F_JUMP_SRCHSTARTPOS,F_0,					F_0,				F_0,					F_0 },
		{ VK_F4,	_T("F4" ),			F_SPLIT_V,		F_SPLIT_H,			F_0,					F_FILECLOSE_OPEN,	F_0,					F_EXITALLEDITORS,	F_EXITALL,				F_0 },
		{ VK_F5,	_T("F5" ),			F_REDRAW,		F_0,				F_EXECMD_DIALOG,		F_0,				F_UUDECODE,				F_0,				F_TABTOSPACE,			F_SPACETOTAB },
		{ VK_F6,	_T("F6" ),			F_BEGIN_SEL,	F_BEGIN_BOX,		F_TOLOWER,				F_0,				F_BASE64DECODE,			F_0,				F_0,					F_0 },
		{ VK_F7,	_T("F7" ),			F_CUT,			F_0,				F_TOUPPER,				F_0,				F_CODECNV_UTF72SJIS,	F_CODECNV_SJIS2UTF7,F_FILE_REOPEN_UTF7,		F_0 },
		{ VK_F8,	_T("F8" ),			F_COPY,			F_COPY_CRLF,		F_TOHANKAKU,			F_0,				F_CODECNV_UTF82SJIS,	F_CODECNV_SJIS2UTF8,F_FILE_REOPEN_UTF8,		F_0 },
		{ VK_F9,	_T("F9" ),			F_PASTE,		F_PASTEBOX,			F_TOZENKAKUKATA,		F_0,				F_CODECNV_UNICODE2SJIS,	F_0,				F_FILE_REOPEN_UNICODE,	F_0 },
		{ VK_F10,	_T("F10"),			_SQL_RUN,		F_DUPLICATELINE,	F_TOZENKAKUHIRA,		F_0,				F_CODECNV_EUC2SJIS,		F_CODECNV_SJIS2EUC,	F_FILE_REOPEN_EUC,		F_0 },
		{ VK_F11,	_T("F11"),			F_OUTLINE,		F_ACTIVATE_SQLPLUS,	F_HANKATATOZENKATA,		F_0,				F_CODECNV_EMAIL,		F_CODECNV_SJIS2JIS,	F_FILE_REOPEN_JIS,		F_0 },
		{ VK_F12,	_T("F12"),			F_TAGJUMP,		F_TAGJUMPBACK,		F_HANKATATOZENHIRA,		F_0,				F_CODECNV_AUTO2SJIS,	F_0,				F_FILE_REOPEN_SJIS,		F_0 },
		{ VK_F13,	_T("F13"),			F_0,			F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 },
		{ VK_F14,	_T("F14"),			F_0,			F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 },
		{ VK_F15,	_T("F15"),			F_0,			F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 },
		{ VK_F16,	_T("F16"),			F_0,			F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 },
		{ VK_F17,	_T("F17"),			F_0,			F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 },
		{ VK_F18,	_T("F18"),			F_0,			F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 },
		{ VK_F19,	_T("F19"),			F_0,			F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 },
		{ VK_F20,	_T("F20"),			F_0,			F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 },
		{ VK_F21,	_T("F21"),			F_0,			F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 },
		{ VK_F22,	_T("F22"),			F_0,			F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 },
		{ VK_F23,	_T("F23"),			F_0,			F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 },
		{ VK_F24,	_T("F24"),			F_0,			F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 },

		/* 特殊キー */
		//keycode,	keyname,			なし,			Shitf+,				Ctrl+,					Shift+Ctrl+,		Alt+,					Shit+Alt+,			Ctrl+Alt+,				Shift+Ctrl+Alt+
		{ VK_TAB,	_T("Tab"),			F_INDENT_TAB,	F_UNINDENT_TAB,		F_NEXTWINDOW,			F_PREVWINDOW,		F_0,					F_0,				F_0,					F_0 },
		{ VK_RETURN,_T("Enter"),		F_0,			F_0,				F_COMPARE,				F_0,				F_PROPERTY_FILE,		F_0,				F_0,					F_0 },
		{ VK_ESCAPE,_T("Esc"),			F_CANCEL_MODE,	F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 },
		{ VK_BACK,	_T("BkSp"),			F_DELETE_BACK,	F_0,				F_WordDeleteToStart,	F_0,				F_0,					F_0,				F_0,					F_0 },
		{ VK_INSERT,_T("Ins"),			F_CHGMOD_INS,	F_PASTE,			F_COPY,					F_0,				F_0,					F_0,				F_0,					F_0 },
		{ VK_DELETE,_T("Del"),			F_DELETE,		F_CUT,				F_WordDeleteToEnd,		F_0,				F_0,					F_0,				F_0,					F_0 },
		{ VK_HOME,	_T("Home"),			F_GOLINETOP,	F_GOLINETOP_SEL,	F_GOFILETOP,			F_GOFILETOP_SEL,	F_0,					F_0,				F_0,					F_0 },
		{ VK_END,	_T("End(Help)"),	F_GOLINEEND,	F_GOLINEEND_SEL,	F_GOFILEEND,			F_GOFILEEND_SEL,	F_0,					F_0,				F_0,					F_0 },
		{ VK_LEFT,	_T("←"),			F_LEFT,			F_LEFT_SEL,			F_WORDLEFT,				F_WORDLEFT_SEL,		F_BEGIN_BOX,			F_0,				F_0,					F_0 },
		{ VK_UP,	_T("↑"),			F_UP,			F_UP_SEL,			F_WndScrollDown,		F_UP2_SEL,			F_BEGIN_BOX,			F_0,				F_0,					F_MAXIMIZE_V },
		{ VK_RIGHT,	_T("→"),			F_RIGHT,		F_RIGHT_SEL,		F_WORDRIGHT,			F_WORDRIGHT_SEL,	F_BEGIN_BOX,			F_0,				F_0,					F_MAXIMIZE_H },
		{ VK_DOWN,	_T("↓"),			F_DOWN,			F_DOWN_SEL,			F_WndScrollUp,			F_DOWN2_SEL,		F_BEGIN_BOX,			F_0,				F_MENU_RBUTTON,			F_MINIMIZE_ALL },
		{ VK_NEXT,	_T("PgDn(RollUp)"),	F_1PageDown,	F_1PageDown_Sel,	F_HalfPageDown,			F_HalfPageDown_Sel,	F_0,					F_0,				F_0,					F_0 },
		{ VK_PRIOR,	_T("PgUp(RollDn)"),	F_1PageUp,		F_1PageUp_Sel,		F_HalfPageUp,			F_HalfPageUp_Sel,	F_0,					F_0,				F_0,					F_0 },
		{ VK_SPACE,	_T("Space"),		F_INDENT_SPACE,	F_UNINDENT_SPACE,	F_HOKAN,				F_0,				F_0,					F_0,				F_0,					F_0 },

		/* 数字 */
		//keycode,	keyname,			なし,			Shitf+,				Ctrl+,					Shift+Ctrl+,		Alt+,					Shit+Alt+,			Ctrl+Alt+,				Shift+Ctrl+Alt+
		{ '0',		_T("0"),			F_0,			F_0,				F_0,					F_0,				F_CUSTMENU_10,			F_CUSTMENU_20,		F_0,					F_0 },
		{ '1',		_T("1"),			F_0,			F_0,				F_SHOWTOOLBAR,			F_CUSTMENU_21,		F_CUSTMENU_1,			F_CUSTMENU_11,		F_0,					F_0 },
		{ '2',		_T("2"),			F_0,			F_0,				F_SHOWFUNCKEY,			F_CUSTMENU_22,		F_CUSTMENU_2,			F_CUSTMENU_12,		F_0,					F_0 },
		{ '3',		_T("3"),			F_0,			F_0,				F_SHOWSTATUSBAR,		F_CUSTMENU_23,		F_CUSTMENU_3,			F_CUSTMENU_13,		F_0,					F_0 },
		{ '4',		_T("4"),			F_0,			F_0,				F_TYPE_LIST,			F_CUSTMENU_24,		F_CUSTMENU_4,			F_CUSTMENU_14,		F_0,					F_0 },
		{ '5',		_T("5"),			F_0,			F_0,				F_OPTION_TYPE,			F_0,				F_CUSTMENU_5,			F_CUSTMENU_15,		F_0,					F_0 },
		{ '6',		_T("6"),			F_0,			F_0,				F_OPTION,				F_0,				F_CUSTMENU_6,			F_CUSTMENU_16,		F_0,					F_0 },
		{ '7',		_T("7"),			F_0,			F_0,				F_FONT,					F_0,				F_CUSTMENU_7,			F_CUSTMENU_17,		F_0,					F_0 },
		{ '8',		_T("8"),			F_0,			F_0,				F_0,					F_0,				F_CUSTMENU_8,			F_CUSTMENU_18,		F_0,					F_0 },
		{ '9',		_T("9"),			F_0,			F_0,				F_0,					F_0,				F_CUSTMENU_9,			F_CUSTMENU_19,		F_0,					F_0 },

		/* アルファベット */
		//keycode,	keyname,			なし,			Shitf+,				Ctrl+,					Shift+Ctrl+,		Alt+,					Shit+Alt+,			Ctrl+Alt+,				Shift+Ctrl+Alt+
		{ 'A',		_T("A"),			F_0,			F_0,				F_SELECTALL,			F_0,				F_SORT_ASC,				F_0,				F_0,					F_0 },
		{ 'B',		_T("B"),			F_0,			F_0,				F_BROWSE,				F_0,				F_0,					F_0,				F_0,					F_0 },
		{ 'C',		_T("C"),			F_0,			F_0,				F_COPY,					F_OPEN_HfromtoC,	F_0,					F_0,				F_0,					F_0 },
		{ 'D',		_T("D"),			F_0,			F_0,				F_WordCut,				F_WordDelete,		F_SORT_DESC,			F_0,				F_0,					F_0 },
		{ 'E',		_T("E"),			F_0,			F_0,				F_CUT_LINE,				F_DELETE_LINE,		F_0,					F_0,				F_CASCADE,				F_0 },
		{ 'F',		_T("F"),			F_0,			F_0,				F_SEARCH_DIALOG,		F_0,				F_0,					F_0,				F_0,					F_0 },
		{ 'G',		_T("G"),			F_0,			F_0,				F_GREP_DIALOG,			F_0,				F_0,					F_0,				F_0,					F_0 },
		{ 'H',		_T("H"),			F_0,			F_0,				F_CURLINECENTER,		F_OPEN_HfromtoC,	F_0,					F_0,				F_TILE_V,				F_0 },
		{ 'I',		_T("I"),			F_0,			F_0,				F_DUPLICATELINE,		F_0,				F_0,					F_0,				F_0,					F_0 },
		{ 'J',		_T("J"),			F_0,			F_0,				F_JUMP_DIALOG,			F_0,				F_0,					F_0,				F_0,					F_0 },
		{ 'K',		_T("K"),			F_0,			F_0,				F_LineCutToEnd,			F_LineDeleteToEnd,	F_0,					F_0,				F_0,					F_0 },
		{ 'L',		_T("L"),			F_0,			F_0,				F_LOADKEYMACRO,			F_EXECKEYMACRO,		F_LTRIM,				F_0,				F_TOLOWER,				F_TOUPPER },
		{ 'M',		_T("M"),			F_0,			F_0,				F_SAVEKEYMACRO,			F_RECKEYMACRO,		F_MERGE,				F_0,				F_0,					F_0 },
		{ 'N',		_T("N"),			F_0,			F_0,				F_FILENEW,				F_0,				F_JUMPHIST_NEXT,		F_0,				F_0,					F_0 },
		{ 'O',		_T("O"),			F_0,			F_0,				F_FILEOPEN,				F_0,				F_0,					F_0,				F_0,					F_0 },
		{ 'P',		_T("P"),			F_0,			F_0,				F_PRINT,				F_PRINT_PREVIEW,	F_JUMPHIST_PREV,		F_0,				F_PRINT_PAGESETUP,		F_0 },
		{ 'Q',		_T("Q"),			F_0,			F_0,				F_CREATEKEYBINDLIST,	F_0,				F_0,					F_0,				F_0,					F_0 },
		{ 'R',		_T("R"),			F_0,			F_0,				F_REPLACE_DIALOG,		F_0,				F_RTRIM,				F_0,				F_0,					F_0 },
		{ 'S',		_T("S"),			F_0,			F_0,				F_FILESAVE,				F_FILESAVEAS_DIALOG,F_0,					F_0,				F_TMPWRAPSETTING,		F_0 },
		{ 'T',		_T("T"),			F_0,			F_0,				F_TAGJUMP,				F_TAGJUMPBACK,		F_0,					F_0,				F_TILE_H,				F_0 },
		{ 'U',		_T("U"),			F_0,			F_0,				F_LineCutToStart,		F_LineDeleteToStart,F_0,					F_0,				F_WRAPWINDOWWIDTH,		F_0 },
		{ 'V',		_T("V"),			F_0,			F_0,				F_PASTE,				F_0,				F_0,					F_0,				F_0,					F_0 },
		{ 'W',		_T("W"),			F_0,			F_0,				F_SELECTWORD,			F_0,				F_0,					F_0,				F_TMPWRAPWINDOW,		F_0 },
		{ 'X',		_T("X"),			F_0,			F_0,				F_CUT,					F_0,				F_0,					F_0,				F_TMPWRAPNOWRAP,		F_0 },
		{ 'Y',		_T("Y"),			F_0,			F_0,				F_REDO,					F_0,				F_0,					F_0,				F_0,					F_0 },
		{ 'Z',		_T("Z"),			F_0,			F_0,				F_UNDO,					F_0,				F_0,					F_0,				F_0,					F_0 },

		/* 記号 */
		//keycode,	keyname,			なし,			Shitf+,				Ctrl+,					Shift+Ctrl+,		Alt+,					Shit+Alt+,			Ctrl+Alt+,				Shift+Ctrl+Alt+
		{ 0x00bd,	_T("-"),			F_0,			F_0,				F_COPYFNAME,			F_SPLIT_V,			F_0,					F_0,				F_0,					F_0 },
		{ 0x00de,	_T("^(英語')"),		F_0,			F_0,				F_COPYTAG,				F_0,				F_0,					F_0,				F_0,					F_0 },
		{ 0x00dc,	_T("\\"),			F_0,			F_0,				F_COPYPATH,				F_SPLIT_H,			F_0,					F_0,				F_0,					F_0 },
		{ 0x00c0,	_T("@(英語`)"),		F_0,			F_0,				F_COPYLINES,			F_0,				F_0,					F_0,				F_0,					F_0 },
		{ 0x00db,	_T("["),			F_0,			F_0,				F_BRACKETPAIR,			F_0,				F_0,					F_0,				F_0,					F_0 },
		{ 0x00bb,	_T(";"),			F_0,			F_0,				F_0,					F_SPLIT_VH,			F_INS_DATE,				F_0,				F_0,					F_0 },
		{ 0x00ba,	_T(":"),			F_0,			F_0,				_COPYWITHLINENUM,		F_0,				F_INS_TIME,				F_0,				F_0,					F_0 },
		{ 0x00dd,	_T("]"),			F_0,			F_0,				F_BRACKETPAIR,			F_0,				F_0,					F_0,				F_0,					F_0 },
		{ 0x00bc,	_T(","),			F_0,			F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 },
		{ 0x00be,	_T("."),			F_0,			F_0,				F_COPYLINESASPASSAGE,	F_0,				F_0,					F_0,				F_0,					F_0 },
		{ 0x00bf,	_T("/"),			F_0,			F_0,				F_HOKAN,				F_0,				F_0,					F_0,				F_0,					F_0 },
		{ 0x00e2,	_T("_"),			F_0,			F_0,				F_UNDO,					F_0,				F_0,					F_0,				F_0,					F_0 },
		{ 0x00df,	_T("_(PC-98)"),		F_0,			F_0,				F_UNDO,					F_0,				F_0,					F_0,				F_0,					F_0 },
		{ VK_APPS,	_T("アプリキー"),	F_MENU_RBUTTON,	F_MENU_RBUTTON,		F_MENU_RBUTTON,			F_MENU_RBUTTON,		F_MENU_RBUTTON,			F_MENU_RBUTTON,		F_MENU_RBUTTON,			F_MENU_RBUTTON }
	};
	const int	nKeyDataInitNum = sizeof( KeyDataInit ) / sizeof( KeyDataInit[0] );
	//	From Here 2007.11.04 genta バッファオーバーラン防止
	if( nKeyDataInitNum > sizeof( pShareData->m_Common.m_sKeyBind.m_pKeyNameArr ) / sizeof( pShareData->m_Common.m_sKeyBind.m_pKeyNameArr[0])) {
		::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP, _T("作者に教えて欲しいエラー"),
			_T("キー設定数に対してDLLSHARE::m_nKeyNameArr[]のサイズが不足しています") );
		return false;
	}
	//	To Here 2007.11.04 genta バッファオーバーラン防止
	for( int i = 0; i < nKeyDataInitNum; ++i ){
		SetKeyNameArrVal(
			pShareData,
			i,
			KeyDataInit[i].nKeyCode,
			KeyDataInit[i].pszKeyName,
			KeyDataInit[i].nFuncCode_0,
			KeyDataInit[i].nFuncCode_1,
			KeyDataInit[i].nFuncCode_2,
			KeyDataInit[i].nFuncCode_3,
			KeyDataInit[i].nFuncCode_4,
			KeyDataInit[i].nFuncCode_5,
			KeyDataInit[i].nFuncCode_6,
			KeyDataInit[i].nFuncCode_7
		 );
	}
	pShareData->m_Common.m_sKeyBind.m_nKeyNameArrNum = nKeyDataInitNum;
	return true;
}





// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         実装補助                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/* KEYDATA配列にデータをセット */
static void SetKeyNameArrVal(
	DLLSHAREDATA*	pShareData,
	int				nIdx,
	short			nKeyCode,
	TCHAR*			pszKeyName,
	EFunctionCode	nFuncCode_0,
	EFunctionCode	nFuncCode_1,
	EFunctionCode	nFuncCode_2,
	EFunctionCode	nFuncCode_3,
	EFunctionCode	nFuncCode_4,
	EFunctionCode	nFuncCode_5,
	EFunctionCode	nFuncCode_6,
	EFunctionCode	nFuncCode_7
)
{
	pShareData->m_Common.m_sKeyBind.m_pKeyNameArr[nIdx].m_nKeyCode = nKeyCode;
	_tcscpy( pShareData->m_Common.m_sKeyBind.m_pKeyNameArr[nIdx].m_szKeyName, pszKeyName );
	pShareData->m_Common.m_sKeyBind.m_pKeyNameArr[nIdx].m_nFuncCodeArr[0] = nFuncCode_0;
	pShareData->m_Common.m_sKeyBind.m_pKeyNameArr[nIdx].m_nFuncCodeArr[1] = nFuncCode_1;
	pShareData->m_Common.m_sKeyBind.m_pKeyNameArr[nIdx].m_nFuncCodeArr[2] = nFuncCode_2;
	pShareData->m_Common.m_sKeyBind.m_pKeyNameArr[nIdx].m_nFuncCodeArr[3] = nFuncCode_3;
	pShareData->m_Common.m_sKeyBind.m_pKeyNameArr[nIdx].m_nFuncCodeArr[4] = nFuncCode_4;
	pShareData->m_Common.m_sKeyBind.m_pKeyNameArr[nIdx].m_nFuncCodeArr[5] = nFuncCode_5;
	pShareData->m_Common.m_sKeyBind.m_pKeyNameArr[nIdx].m_nFuncCodeArr[6] = nFuncCode_6;
	pShareData->m_Common.m_sKeyBind.m_pKeyNameArr[nIdx].m_nFuncCodeArr[7] = nFuncCode_7;
}

