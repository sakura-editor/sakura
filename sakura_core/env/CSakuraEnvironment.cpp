/*
	Copyright (C) 2008, kobake, ryoji
	Copyright (C) 2012, Uchi

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
#include "CSakuraEnvironment.h"
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"
#include "env/CFormatManager.h"
#include "env/CFileNameManager.h"
#include "_main/CAppMode.h"
#include "_main/CCommandLine.h"
#include "doc/CEditDoc.h"
#include "window/CEditWnd.h"
#include "print/CPrintPreview.h"
#include "macro/CSMacroMgr.h"
#include "CEditApp.h"
#include "CGrepAgent.h"
#include "recent/CMRUFile.h"
#include "recent/CMRUFolder.h"
#include "util/string_ex2.h"
#include "util/module.h" //GetAppVersionInfo
#include "util/shell.h"
#include "util/window.h"

typedef std::wstring wstring;

CEditWnd* CSakuraEnvironment::GetMainWindow()
{
	return CEditWnd::getInstance();
}

enum EExpParamName
{
	EExpParamName_none = -1,
	EExpParamName_begin = 0,
	EExpParamName_profile = 0,
	EExpParamName_end
};

struct SExpParamName
{
	const wchar_t* m_szName;
	int m_nLen;
};
static SExpParamName SExpParamNameTable[] = {
	{L"profile", 7},
	{NULL, 0}
};
wchar_t* ExParam_LongName( wchar_t* q, wchar_t* q_max, EExpParamName eLongParam );

/*!	$xの展開

	特殊文字は以下の通り
	@li $  $自身
	@li A  アプリ名
	@li F  開いているファイルのフルパス。名前がなければ(無題)。
	@li f  開いているファイルの名前（ファイル名+拡張子のみ）
	@li g  開いているファイルの名前（拡張子除く）
	@li /  開いているファイルの名前（フルパス。パスの区切りが/）
	@li N  開いているファイルの名前(簡易表示)
	@li n  無題の通し番号
	@li E  開いているファイルのあるフォルダの名前(簡易表示)
	@li e  開いているファイルのあるフォルダの名前
	@li B  タイプ別設定の名前
	@li b  開いているファイルの拡張子
	@li Q  印刷ページ設定の名前
	@li C  現在選択中のテキスト
	@li x  現在の物理桁位置(先頭からのバイト数1開始)
	@li y  現在の物理行位置(1開始)
	@li d  現在の日付(共通設定の日付書式)
	@li t  現在の時刻(共通設定の時刻書式)
	@li p  現在のページ
	@li P  総ページ
	@li D  ファイルのタイムスタンプ(共通設定の日付書式)
	@li T  ファイルのタイムスタンプ(共通設定の時刻書式)
	@li V  エディタのバージョン文字列
	@li h  Grep検索キーの先頭32byte
	@li S  サクラエディタのフルパス
	@li I  iniファイルのフルパス
	@li M  現在実行しているマクロファイルパス
	@li <profile> プロファイル名

	@date 2003.04.03 genta wcsncpy_ex導入によるfor文の削減
	@date 2005.09.15 FILE 特殊文字S, M追加
	@date 2007.09.21 kobake 特殊文字A(アプリ名)を追加
	@date 2008.05.05 novice GetModuleHandle(NULL)→NULLに変更
	@date 2012.10.11 Moca 特殊文字n追加
*/
void CSakuraEnvironment::ExpandParameter(const wchar_t* pszSource, wchar_t* pszBuffer, int nBufferLen)
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0); //###

	// Apr. 03, 2003 genta 固定文字列をまとめる
	const wstring	PRINT_PREVIEW_ONLY		= LSW( STR_PREVIEW_ONLY );	//L"(印刷プレビューでのみ使用できます)";
	const int		PRINT_PREVIEW_ONLY_LEN	= PRINT_PREVIEW_ONLY.length();
	const wstring	NO_TITLE				= LSW( STR_NO_TITLE1 );	//L"(無題)";
	const int		NO_TITLE_LEN			= NO_TITLE.length();
	const wstring	NOT_SAVED				= LSW( STR_NOT_SAVED );	//L"(保存されていません)";
	const int		NOT_SAVED_LEN			= NOT_SAVED.length();

	const wchar_t *p, *r;	//	p：目的のバッファ。r：作業用のポインタ。
	wchar_t *q, *q_max;

	for( p = pszSource, q = pszBuffer, q_max = pszBuffer + nBufferLen; *p != '\0' && q < q_max;){
		if( *p != '$' ){
			*q++ = *p++;
			continue;
		}
		switch( *(++p) ){
		case L'$':	//	 $$ -> $
			*q++ = *p++;
			break;
		case L'A':	//アプリ名
			q = wcs_pushW( q, q_max - q, GSTR_APPNAME_W, wcslen(GSTR_APPNAME_W) );
			++p;
			break;
		case L'F':	//	開いているファイルの名前（フルパス）
			if ( !pcDoc->m_cDocFile.GetFilePathClass().IsValidPath() ){
				q = wcs_pushW( q, q_max - q, NO_TITLE.c_str(), NO_TITLE_LEN );
				++p;
			} 
			else {
				r = to_wchar(pcDoc->m_cDocFile.GetFilePath());
				q = wcs_pushW( q, q_max - q, r, wcslen( r ));
				++p;
			}
			break;
		case L'f':	//	開いているファイルの名前（ファイル名+拡張子のみ）
			// Oct. 28, 2001 genta
			//	ファイル名のみを渡すバージョン
			//	ポインタを末尾に
			if ( ! pcDoc->m_cDocFile.GetFilePathClass().IsValidPath() ){
				q = wcs_pushW( q, q_max - q, NO_TITLE.c_str(), NO_TITLE_LEN );
				++p;
			} 
			else {
				// 2002.10.13 Moca ファイル名(パスなし)を取得。日本語対応
				//	万一\\が末尾にあってもその後ろには\0があるのでアクセス違反にはならない。
				q = wcs_pushT( q, q_max - q, pcDoc->m_cDocFile.GetFileName());
				++p;
			}
			break;
		case L'g':	//	開いているファイルの名前（拡張子を除くファイル名のみ）
			//	From Here Sep. 16, 2002 genta
			if ( ! pcDoc->m_cDocFile.GetFilePathClass().IsValidPath() ){
				q = wcs_pushW( q, q_max - q, NO_TITLE.c_str(), NO_TITLE_LEN );
				++p;
			} 
			else {
				//	ポインタを末尾に
				const wchar_t *dot_position, *end_of_path;
				r = to_wchar(pcDoc->m_cDocFile.GetFileName()); // 2002.10.13 Moca ファイル名(パスなし)を取得。日本語対応
				end_of_path = dot_position =
					r + wcslen( r );
				//	後ろから.を探す
				for( --dot_position ; dot_position > r && *dot_position != '.'
					; --dot_position )
					;
				//	rと同じ場所まで行ってしまった⇔.が無かった
				if( dot_position == r )
					dot_position = end_of_path;

				q = wcs_pushW( q, q_max - q, r, dot_position - r );
				++p;
			}
			break;
			//	To Here Sep. 16, 2002 genta
		case L'/':	//	開いているファイルの名前（フルパス。パスの区切りが/）
			// Oct. 28, 2001 genta
			if ( !pcDoc->m_cDocFile.GetFilePathClass().IsValidPath() ){
				q = wcs_pushW( q, q_max - q, NO_TITLE.c_str(), NO_TITLE_LEN );
				++p;
			} 
			else {
				//	パスの区切りとして'/'を使うバージョン
				for( r = to_wchar(pcDoc->m_cDocFile.GetFilePath()); *r != L'\0' && q < q_max; ++r, ++q ){
					if( *r == L'\\' )
						*q = L'/';
					else
						*q = *r;
				}
				++p;
			}
			break;
		//	From Here 2003/06/21 Moca
		case L'N':	//	開いているファイルの名前(簡易表示)
			if( !pcDoc->m_cDocFile.GetFilePathClass().IsValidPath() ){
				q = wcs_pushW( q, q_max - q, NO_TITLE.c_str(), NO_TITLE_LEN );
				++p;
			}
			else {
				TCHAR szText[1024];
				NONCLIENTMETRICS met;
				met.cbSize = CCSIZEOF_STRUCT(NONCLIENTMETRICS, lfMessageFont);
				::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, met.cbSize, &met, 0);
				CDCFont dcFont(met.lfCaptionFont, GetMainWindow()->GetHwnd());
				CFileNameManager::getInstance()->GetTransformFileNameFast( pcDoc->m_cDocFile.GetFilePath(), szText, 1023, dcFont.GetHDC(), true );
				q = wcs_pushT( q, q_max - q, szText);
				++p;
			}
			break;
		//	To Here 2003/06/21 Moca
		case L'n':
			if( !pcDoc->m_cDocFile.GetFilePathClass().IsValidPath() ){
				if( CEditApp::getInstance()->m_pcGrepAgent->m_bGrepMode ){
				}else if( CAppMode::getInstance()->IsDebugMode() ){
				}else{
					WCHAR szText[10];
					const EditNode* node = CAppNodeManager::getInstance()->GetEditNode( GetMainWindow()->GetHwnd() );
					if( 0 < node->m_nId ){
						swprintf( szText, L"%d", node->m_nId );
						q = wcs_pushW( q, q_max - q, szText );
					}
				}
			}
			++p;
			break;
		case L'E':	// 開いているファイルのあるフォルダの名前(簡易表示)	2012/12/2 Uchi
			if( !pcDoc->m_cDocFile.GetFilePathClass().IsValidPath() ){
				q = wcs_pushW( q, q_max - q, NO_TITLE.c_str(), NO_TITLE_LEN );
			}
			else {
				WCHAR	buff[_MAX_PATH];		// \の処理をする為WCHAR
				WCHAR*	pEnd;
				WCHAR*	p;

				wcscpy_s( buff, _MAX_PATH, to_wchar(pcDoc->m_cDocFile.GetFilePath()) );
				pEnd = NULL;
				for ( p = buff; *p != '\0'; p++) {
					if (*p == L'\\') {
						pEnd = p;
					}
				}
				if (pEnd != NULL) {
					// 最後の\の後で終端
					*(pEnd+1) = '\0';
				}

				// 簡易表示に変換
				TCHAR szText[1024];
				NONCLIENTMETRICS met;
				met.cbSize = CCSIZEOF_STRUCT(NONCLIENTMETRICS, lfMessageFont);
				::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, met.cbSize, &met, 0);
				CDCFont dcFont(met.lfCaptionFont, GetMainWindow()->GetHwnd());
				CFileNameManager::getInstance()->GetTransformFileNameFast( to_tchar(buff), szText, _countof(szText)-1, dcFont.GetHDC(), true );
				q = wcs_pushT( q, q_max - q, szText);
			}
			++p;
			break;
		case L'e':	// 開いているファイルのあるフォルダの名前		2012/12/2 Uchi
			if( !pcDoc->m_cDocFile.GetFilePathClass().IsValidPath() ){
				q = wcs_pushW( q, q_max - q, NO_TITLE.c_str(), NO_TITLE_LEN );
			}
			else {
				const WCHAR*	pStr;
				const WCHAR*	pEnd;
				const WCHAR*	p;

				pStr = to_wchar(pcDoc->m_cDocFile.GetFilePath());
				pEnd = pStr - auto_strlen(pStr) - 1;
				for ( p = pStr; *p != '\0'; p++) {
					if (*p == L'\\') {
						pEnd = p;
					}
				}
				q = wcs_pushW( q, q_max - q, pStr, pEnd - pStr + 1 );
			}
			++p;
			break;
		//	From Here Jan. 15, 2002 hor
		case L'B':	// タイプ別設定の名前			2013/03/28 Uchi
			{
				const STypeConfig&	sTypeCongig = pcDoc->m_cDocType.GetDocumentAttribute();
				if (sTypeCongig.m_nIdx > 0) {	// 基本は表示しない
					q = wcs_pushT( q, q_max - q, sTypeCongig.m_szTypeName);
				}
				++p;
			}
			break;
		case L'b':	// 開いているファイルの拡張子	2013/03/28 Uchi
			if ( pcDoc->m_cDocFile.GetFilePathClass().IsValidPath() ){
				//	ポインタを末尾に
				const wchar_t	*dot_position, *end_of_path;
				r = to_wchar(pcDoc->m_cDocFile.GetFileName());
				end_of_path = dot_position = r + wcslen( r );
				//	後ろから.を探す
				while (--dot_position >= r && *dot_position != L'.')
					;
				//	.を発見(拡張子有り)
				if (*dot_position == L'.') {
					q = wcs_pushW( q, q_max - q, dot_position +1, end_of_path - dot_position -1 );
				}
			}
			++p;
			break;
		case L'Q':	// 印刷ページ設定の名前			2013/03/28 Uchi
			{
				PRINTSETTING*	ps = &GetDllShareData().m_PrintSettingArr[
					 pcDoc->m_cDocType.GetDocumentAttribute().m_nCurrentPrintSetting];
				q = wcs_pushT( q, q_max - q, ps->m_szPrintSettingName);
				++p;
			}
			break;
		case L'C':	//	現在選択中のテキスト
			{
				CNativeW cmemCurText;
				GetMainWindow()->GetActiveView().GetCurrentTextForSearch( cmemCurText );

				q = wcs_pushW( q, q_max - q, cmemCurText.GetStringPtr(), cmemCurText.GetStringLength());
				++p;
			}
		//	To Here Jan. 15, 2002 hor
			break;
		//	From Here 2002/12/04 Moca
		case L'x':	//	現在の物理桁位置(先頭からのバイト数1開始)
			{
				wchar_t szText[11];
				_itow( GetMainWindow()->GetActiveView().GetCaret().GetCaretLogicPos().x + 1, szText, 10 );
				q = wcs_pushW( q, q_max - q, szText);
				++p;
			}
			break;
		case L'y':	//	現在の物理行位置(1開始)
			{
				wchar_t szText[11];
				_itow( GetMainWindow()->GetActiveView().GetCaret().GetCaretLogicPos().y + 1, szText, 10 );
				q = wcs_pushW( q, q_max - q, szText);
				++p;
			}
			break;
		//	To Here 2002/12/04 Moca
		case L'd':	//	共通設定の日付書式
			{
				TCHAR szText[1024];
				SYSTEMTIME systime;
				::GetLocalTime( &systime );
				CFormatManager().MyGetDateFormat( systime, szText, _countof( szText ) - 1 );
				q = wcs_pushT( q, q_max - q, szText);
				++p;
			}
			break;
		case L't':	//	共通設定の時刻書式
			{
				TCHAR szText[1024];
				SYSTEMTIME systime;
				::GetLocalTime( &systime );
				CFormatManager().MyGetTimeFormat( systime, szText, _countof( szText ) - 1 );
				q = wcs_pushT( q, q_max - q, szText);
				++p;
			}
			break;
		case L'p':	//	現在のページ
			{
				CEditWnd*	pcEditWnd = GetMainWindow();	//	Sep. 10, 2002 genta
				if (pcEditWnd->m_pPrintPreview){
					wchar_t szText[1024];
					_itow(pcEditWnd->m_pPrintPreview->GetCurPageNum() + 1, szText, 10);
					q = wcs_pushW( q, q_max - q, szText, wcslen(szText));
					++p;
				}
				else {
					q = wcs_pushW( q, q_max - q, PRINT_PREVIEW_ONLY.c_str(), PRINT_PREVIEW_ONLY_LEN );
					++p;
				}
			}
			break;
		case L'P':	//	総ページ
			{
				CEditWnd*	pcEditWnd = GetMainWindow();	//	Sep. 10, 2002 genta
				if (pcEditWnd->m_pPrintPreview){
					wchar_t szText[1024];
					_itow(pcEditWnd->m_pPrintPreview->GetAllPageNum(), szText, 10);
					q = wcs_pushW( q, q_max - q, szText);
					++p;
				}
				else {
					q = wcs_pushW( q, q_max - q, PRINT_PREVIEW_ONLY.c_str(), PRINT_PREVIEW_ONLY_LEN );
					++p;
				}
			}
			break;
		case L'D':	//	タイムスタンプ
			if (!pcDoc->m_cDocFile.IsFileTimeZero()){
				TCHAR szText[1024];
				CFormatManager().MyGetDateFormat(
					pcDoc->m_cDocFile.GetFileSysTime(),
					szText,
					_countof( szText ) - 1
				);
				q = wcs_pushT( q, q_max - q, szText);
				++p;
			}
			else {
				q = wcs_pushW( q, q_max - q, NOT_SAVED.c_str(), NOT_SAVED_LEN );
				++p;
			}
			break;
		case L'T':	//	タイムスタンプ
			if (!pcDoc->m_cDocFile.IsFileTimeZero()){
				TCHAR szText[1024];
				CFormatManager().MyGetTimeFormat(
					pcDoc->m_cDocFile.GetFileSysTime(),
					szText,
					_countof( szText ) - 1
				);
				q = wcs_pushT( q, q_max - q, szText);
				++p;
			}
			else {
				q = wcs_pushW( q, q_max - q, NOT_SAVED.c_str(), NOT_SAVED_LEN );
				++p;
			}
			break;
		case L'V':	// Apr. 4, 2003 genta
			// Version number
			{
				wchar_t buf[28]; // 6(符号含むWORDの最大長) * 4 + 4(固定部分)
				//	2004.05.13 Moca バージョン番号は、プロセスごとに取得する
				DWORD dwVersionMS, dwVersionLS;
				GetAppVersionInfo( NULL, VS_VERSION_INFO, &dwVersionMS, &dwVersionLS );
				int len = auto_sprintf( buf, L"%d.%d.%d.%d",
					HIWORD( dwVersionMS ),
					LOWORD( dwVersionMS ),
					HIWORD( dwVersionLS ),
					LOWORD( dwVersionLS )
				);
				q = wcs_pushW( q, q_max - q, buf, len );
				++p;
			}
			break;
		case L'h':	//	Apr. 4, 2003 genta
			//	Grep Key文字列 MAX 32文字
			//	中身はSetParentCaption()より移植
			{
				CNativeW	cmemDes;
				// m_szGrepKey → cmemDes
				LimitStringLengthW( CAppMode::getInstance()->m_szGrepKey, wcslen( CAppMode::getInstance()->m_szGrepKey ), (q_max - q > 32 ? 32 : q_max - q - 3), cmemDes );
				if( (int)wcslen( CAppMode::getInstance()->m_szGrepKey ) > cmemDes.GetStringLength() ){
					cmemDes.AppendString(L"...");
				}
				q = wcs_pushW( q, q_max - q, cmemDes.GetStringPtr(), cmemDes.GetStringLength());
				++p;
			}
			break;
		case L'S':	//	Sep. 15, 2005 FILE
			//	サクラエディタのフルパス
			{
				SFilePath	szPath;

				::GetModuleFileName( NULL, szPath, _countof2(szPath) );
				q = wcs_pushT( q, q_max - q, szPath );
				++p;
			}
			break;
		case 'I':	//	May. 19, 2007 ryoji
			//	iniファイルのフルパス
			{
				TCHAR	szPath[_MAX_PATH + 1];
				std::tstring strProfileName = to_tchar(CCommandLine::getInstance()->GetProfileName());
				CFileNameManager::getInstance()->GetIniFileName( szPath, strProfileName.c_str() );
				q = wcs_pushT( q, q_max - q, szPath );
				++p;
			}
			break;
		case 'M':	//	Sep. 15, 2005 FILE
			//	現在実行しているマクロファイルパスの取得
			{
				// 実行中マクロのインデックス番号 (INVALID_MACRO_IDX:無効 / STAND_KEYMACRO:標準マクロ)
				CSMacroMgr* pcSMacroMgr = CEditApp::getInstance()->m_pcSMacroMgr;
				switch( pcSMacroMgr->GetCurrentIdx() ){
				case INVALID_MACRO_IDX:
					break;
				case TEMP_KEYMACRO:
					q = wcs_pushT( q, q_max - q, pcSMacroMgr->GetFile(TEMP_KEYMACRO) );
					break;
				case STAND_KEYMACRO:
					{
						TCHAR* pszMacroFilePath = GetDllShareData().m_Common.m_sMacro.m_szKeyMacroFileName;
						q = wcs_pushT( q, q_max - q, pszMacroFilePath );
					}
					break;
				default:
					{
						TCHAR szMacroFilePath[_MAX_PATH * 2];
						int n = CShareData::getInstance()->GetMacroFilename( pcSMacroMgr->GetCurrentIdx(), szMacroFilePath, _countof(szMacroFilePath) );
						if ( 0 < n ){
							q = wcs_pushT( q, q_max - q, szMacroFilePath );
						}
					}
					break;
				}
				++p;
			}
			break;
		//	Mar. 31, 2003 genta
		//	条件分岐
		//	${cond:string1$:string2$:string3$}
		//	
		case L'{':	// 条件分岐
			{
				int cond;
				cond = _ExParam_Evaluate( p + 1 );
				while( *p != '?' && *p != '\0' )
					++p;
				if( *p == '\0' )
					break;
				p = _ExParam_SkipCond( p + 1, cond );
			}
			break;
		case L':':	// 条件分岐の中間
			//	条件分岐の末尾までSKIP
			p = _ExParam_SkipCond( p + 1, -1 );
			break;
		case L'}':	// 条件分岐の末尾
			//	特にすることはない
			++p;
			break;
		case L'<':
			{
				// $<LongName>
				++p;
				const wchar_t *pBegin = p;
				while( *p != '>' && *p != '\0' ){
					++p;
				}
				if( *p == '\0' ){
					break;
				}
				int nParamNameIdx = EExpParamName_begin;
				for(; nParamNameIdx != EExpParamName_end; nParamNameIdx++ ){
					if( SExpParamNameTable[nParamNameIdx].m_nLen == p - pBegin &&
						0 == auto_strnicmp(SExpParamNameTable[nParamNameIdx].m_szName,
							pBegin, p - pBegin) ){
						q = ExParam_LongName( q, q_max, static_cast<EExpParamName>(nParamNameIdx) );
						break;
					}
				}
				++p; // skip '>'
				break;
			}
		default:
			*q++ = '$';
			*q++ = *p++;
			break;
		}
	}
	*q = '\0';
}



/*! @brief 処理の読み飛ばし

	条件分岐の構文 ${cond:A0$:A1$:A2$:..$} において，
	指定した番号に対応する位置の先頭へのポインタを返す．
	指定番号に対応する物が無ければ$}の次のポインタを返す．

	${が登場した場合にはネストと考えて$}まで読み飛ばす．

	@param pszSource [in] スキャンを開始する文字列の先頭．cond:の次のアドレスを渡す．
	@param part [in] 移動する番号＝読み飛ばす$:の数．-1を与えると最後まで読み飛ばす．

	@return 移動後のポインタ．該当領域の先頭かあるいは$}の直後．

	@author genta
	@date 2003.03.31 genta 作成
*/
const wchar_t* CSakuraEnvironment::_ExParam_SkipCond(const wchar_t* pszSource, int part)
{
	if( part == 0 )
		return pszSource;
	
	int nest = 0;	// 入れ子のレベル
	bool next = true;	// 継続フラグ
	const wchar_t *p;
	for( p = pszSource; next && *p != L'\0'; ++p ) {
		if( *p == L'$' && p[1] != L'\0' ){ // $が末尾なら無視
			switch( *(++p)){
			case L'{':	// 入れ子の開始
				++nest;
				break;
			case L'}':
				if( nest == 0 ){
					//	終了ポイントに達した
					next = false; 
				}
				else {
					//	ネストレベルを下げる
					--nest;
				}
				break;
			case L':':
				if( nest == 0 && --part == 0){ // 入れ子でない場合のみ
					//	目的のポイント
					next = false;
				}
				break;
			}
		}
	}
	return p;
}

/*!	@brief 条件の評価

	@param pCond [in] 条件種別先頭．'?'までを条件と見なして評価する
	@return 評価の値

	@note
	ポインタの読み飛ばし作業は行わないので，'?'までの読み飛ばしは
	呼び出し側で別途行う必要がある．

	@author genta
	@date 2003.03.31 genta 作成

*/
int CSakuraEnvironment::_ExParam_Evaluate( const wchar_t* pCond )
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0); //###

	switch( *pCond ){
	case L'R': // $R ビューモードおよび読み取り専用属性
		if( CAppMode::getInstance()->IsViewMode() ){
			return 0; // ビューモード
		}
		else if( !CEditDoc::GetInstance(0)->m_cDocLocker.IsDocWritable() ){
			return 1; // 上書き禁止
		}
		else{
			return 2; // 上記以外
		}
	case L'w': // $w Grepモード/Output Mode
		if( CEditApp::getInstance()->m_pcGrepAgent->m_bGrepMode ){
			return 0;
		}else if( CAppMode::getInstance()->IsDebugMode() ){
			return 1;
		}else {
			return 2;
		}
	case L'M': // $M キーボードマクロの記録中
		if( GetDllShareData().m_sFlags.m_bRecordingKeyMacro && GetDllShareData().m_sFlags.m_hwndRecordingKeyMacro==CEditWnd::getInstance()->GetHwnd() ){ /* ウィンドウ */
			return 0;
		}else {
			return 1;
		}
	case L'U': // $U 更新
		if( pcDoc->m_cDocEditor.IsModified()){
			return 0;
		}
		else {
			return 1;
		}
	case L'N': // $N 新規/(無題)		2012/12/2 Uchi
		if (!pcDoc->m_cDocFile.GetFilePathClass().IsValidPath()) {
			return 0;
		}
		else {
			return 1;
		}
	case L'I': // $I アイコン化されているか
		if( ::IsIconic( CEditWnd::getInstance()->GetHwnd() )){
			return 0;
		} else {
 			return 1;
 		}
	default:
		break;
	}
	return 0;
}

/*! 長い名前の設定 */
wchar_t* ExParam_LongName( wchar_t* q, wchar_t* q_max, EExpParamName eLongParam )
{
	switch( eLongParam ){
	case EExpParamName_profile:
		{
			LPCWSTR pszProf = CCommandLine::getInstance()->GetProfileName();
			q = wcs_pushW( q, q_max - q, pszProf );
		}
		break;
	default:
		assert( 0 );
		break;
	}
	return q;
}

/*!	@brief 初期フォルダ取得

	@param bControlProcess [in] trueのときはOPENDIALOGDIR_CUR->OPENDIALOGDIR_MRUに変更
	@return 初期フォルダ
*/
std::tstring CSakuraEnvironment::GetDlgInitialDir(bool bControlProcess)
{
	CEditDoc* pcDoc = CEditDoc::GetInstance(0); //######
	if( pcDoc && pcDoc->m_cDocFile.GetFilePathClass().IsValidPath() ){
		return to_tchar(pcDoc->m_cDocFile.GetFilePathClass().GetDirPath().c_str());
	}

	EOpenDialogDir eOpenDialogDir = GetDllShareData().m_Common.m_sEdit.m_eOpenDialogDir;
	if( bControlProcess && eOpenDialogDir == OPENDIALOGDIR_CUR ){
		eOpenDialogDir = OPENDIALOGDIR_MRU;
	}

	switch( eOpenDialogDir ){
	case OPENDIALOGDIR_CUR:
		{
			// 2002.10.25 Moca
			TCHAR szCurDir[_MAX_PATH];
			int nCurDir = ::GetCurrentDirectory( _countof(szCurDir), szCurDir );
			if( 0 == nCurDir || _MAX_PATH < nCurDir ){
				return _T("");
			}
			else{
				return szCurDir;
			}
		}
		break;
	case OPENDIALOGDIR_MRU:
		{
			const CMRUFolder cMRU;
			std::vector<LPCTSTR> vMRU = cMRU.GetPathList();
			int nCount = cMRU.Length();
			for( int i = 0; i < nCount ; i++ ){
				DWORD attr = GetFileAttributes( vMRU[i] );
				if( ( attr != -1 ) && ( attr & FILE_ATTRIBUTE_DIRECTORY ) != 0 ){
					return vMRU[i];
				}
			}

			TCHAR szCurDir[_MAX_PATH];
			int nCurDir = ::GetCurrentDirectory( _countof(szCurDir), szCurDir );
			if( 0 == nCurDir || _MAX_PATH < nCurDir ){
				return _T("");
			}
			else{
				return szCurDir;
			}
		}
		break;
	case OPENDIALOGDIR_SEL:
		{
			TCHAR szSelDir[_MAX_PATH];
			CFileNameManager::ExpandMetaToFolder( GetDllShareData().m_Common.m_sEdit.m_OpenDialogSelDir, szSelDir, _countof(szSelDir) );
			return szSelDir;
		}
		break;
	default:
		assert(0);
		return _T("");
	}
}

void CSakuraEnvironment::ResolvePath(TCHAR* pszPath)
{
	// pszPath -> pSrc
	TCHAR* pSrc = pszPath;

	// ショートカット(.lnk)の解決: pSrc -> szBuf -> pSrc
	TCHAR szBuf[_MAX_PATH];
	if( ResolveShortcutLink( NULL, pSrc, szBuf ) ){
		pSrc = szBuf;
	}

	// ロングファイル名を取得する: pSrc -> szBuf2 -> pSrc
	TCHAR szBuf2[_MAX_PATH];
	if( ::GetLongFileName( pSrc, szBuf2 ) ){
		pSrc = szBuf2;
	}

	// pSrc -> pszPath
	if(pSrc != pszPath){
		_tcscpy_s(pszPath, _MAX_PATH, pSrc);
	}
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      ウィンドウ管理                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


/* 指定ウィンドウが、編集ウィンドウのフレームウィンドウかどうか調べる */
BOOL IsSakuraMainWindow( HWND hWnd )
{
	TCHAR	szClassName[64];
	if( hWnd == NULL ){	// 2007.06.20 ryoji 条件追加
		return FALSE;
	}
	if( !::IsWindow( hWnd ) ){
		return FALSE;
	}
	if( 0 == ::GetClassName( hWnd, szClassName, _countof(szClassName) - 1 ) ){
		return FALSE;
	}
	if(0 == _tcscmp( GSTR_EDITWINDOWNAME, szClassName ) ){
		return TRUE;
	}else{
		return FALSE;
	}
}


