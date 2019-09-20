﻿/*! @file */
/*
	Copyright (C) 2008, kobake

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
#include <time.h>
#include <io.h>	// _access
#include "CBackupAgent.h"
#include "window/CEditWnd.h"
#include "util/format.h" //GetDateTimeFormat

/*! セーブ前おまけ処理
	@param pSaveInfo [in] 保存ファイル情報

	@retval CALLBACK_CONTINUE 続ける
	@retval CALLBACK_INTERRUPT 中断
*/
ECallbackResult CBackupAgent::OnPreBeforeSave(SSaveInfo* pSaveInfo)
{
	CEditDoc* pcDoc = GetListeningDoc();

	//新しくファイルを作る場合は何もしない
	if(!fexist(pSaveInfo->cFilePath))return CALLBACK_CONTINUE;

	//共通設定：保存時にバックアップを作成する
	if( GetDllShareData().m_Common.m_sBackup.m_bBackUp ){
		//	Jun.  5, 2004 genta ファイル名を与えるように．戻り値に応じた処理を追加．
		// ファイル保存前にバックアップ処理
		int nBackupResult = 0;
		{
			pcDoc->m_cDocFileOperation.DoFileUnlock();	//バックアップ作成前にロックを解除する #####スマートじゃないよ！
			nBackupResult = MakeBackUp( pSaveInfo->cFilePath );
			pcDoc->m_cDocFileOperation.DoFileLock();	//バックアップ作成後にロックを戻す #####スマートじゃないよ！
		}
		switch( nBackupResult ){
		case 2:	//	中断指示
			return CALLBACK_INTERRUPT;
		case 3: //	ファイルエラー
			if( IDYES != ::MYMESSAGEBOX(
				CEditWnd::getInstance()->GetHwnd(),
				MB_YESNO | MB_ICONQUESTION | MB_TOPMOST,
				LS(STR_BACKUP_ERR_TITLE),
				LS(STR_BACKUP_ERR_MSG)
			)){
				return CALLBACK_INTERRUPT;
			}
			break;
		}
	}
	return CALLBACK_CONTINUE;
}

/*! バックアップの作成
	@author genta
	@date 2001.06.12 asa-o
		ファイルの時刻を元にバックアップファイル名を作成する機能
	@date 2001.12.11 MIK バックアップファイルをゴミ箱に入れる機能
	@date 2004.06.05 genta バックアップ対象ファイルを引数で与えるように．
		名前を付けて保存の時は自分のバックアップを作っても無意味なので．
		また，バックアップも保存も行わない選択肢を追加．
	@date 2005.11.26 aroka ファイル名生成をFormatBackUpPathに分離

	@param target_file [in] バックアップ元パス名

	@retval 0 バックアップ作成失敗．
	@retval 1 バックアップ作成成功．
	@retval 2 バックアップ作成失敗．保存中断指示．
	@retval 3 ファイル操作エラーによるバックアップ作成失敗．

	@todo Advanced modeでの世代管理
*/
int CBackupAgent::MakeBackUp(
	const WCHAR* target_file
)
{
	int		nRet;

	/* バックアップソースの存在チェック */
	//	Aug. 21, 2005 genta 書き込みアクセス権がない場合も
	//	ファイルがない場合と同様に何もしない
	if( (_waccess( target_file, 2 )) == -1 ){
		return 0;
	}

	const CommonSetting_Backup& bup_setting = GetDllShareData().m_Common.m_sBackup;

	WCHAR	szPath[_MAX_PATH]; // バックアップ先パス名
	if( !FormatBackUpPath( szPath, _countof(szPath), target_file ) ){
		int nMsgResult = ::TopConfirmMessage(
			CEditWnd::getInstance()->GetHwnd(),
			LS(STR_BACKUP_ERR_PATH_CRETE)
		);
		if( nMsgResult == IDYES ){
			return 0;//	保存継続
		}
		return 2;// 保存中断
	}

	//@@@ 2002.03.23 start ネットワーク・リムーバブルドライブの場合はごみ箱に放り込まない
	bool dustflag = false;
	if( bup_setting.m_bBackUpDustBox ){
		dustflag = !IsLocalDrive( szPath );
	}
	//@@@ 2002.03.23 end

	if( bup_setting.m_bBackUpDialog ){	/* バックアップの作成前に確認 */
		ConfirmBeep();
		if( bup_setting.m_bBackUpDustBox && !dustflag ){	//共通設定：バックアップファイルをごみ箱に放り込む	//@@@ 2001.12.11 add start MIK	//2002.03.23
			nRet = ::MYMESSAGEBOX(
				CEditWnd::getInstance()->GetHwnd(),
				MB_YESNO/*CANCEL*/ | MB_ICONQUESTION | MB_TOPMOST,
				LS(STR_BACKUP_CONFORM_TITLE1),
				LS(STR_BACKUP_CONFORM_MSG1),
				target_file,
				szPath
			);
		}
		else{	//@@@ 2001.12.11 add end MIK
			nRet = ::MYMESSAGEBOX(
				CEditWnd::getInstance()->GetHwnd(),
				MB_YESNOCANCEL | MB_ICONQUESTION | MB_TOPMOST,
				LS(STR_BACKUP_CONFORM_TITLE2),
				LS(STR_BACKUP_CONFORM_MSG2),
				target_file,
				szPath
			);	//Jul. 06, 2001 jepro [名前を付けて保存] の場合もあるのでメッセージを修正
		}	//@@@ 2001.12.11 add MIK
		//	Jun.  5, 2005 genta 戻り値変更
		if( IDNO == nRet ){
			return 0;//	保存継続
		}else if( IDCANCEL == nRet ){
			return 2;// 保存中断
		}
	}

	//	From Here Aug. 16, 2000 genta
	//	Jun.  5, 2005 genta 1の拡張子を残す版を追加
	if( bup_setting.GetBackupType() == 3 ||
		bup_setting.GetBackupType() == 6 ){
		//	既に存在するBackupをずらす処理
		int				i;

		//	ファイル検索用
		HANDLE			hFind;
		WIN32_FIND_DATA	fData;

		WCHAR*	pBase = szPath + wcslen( szPath ) - 2;	//	2: 拡張子の最後の2桁の意味

		//------------------------------------------------------------------
		//	1. 該当ディレクトリ中のbackupファイルを1つずつ探す
		for( i = 0; i <= 99; i++ ){	//	最大値に関わらず，99（2桁の最大値）まで探す
			//	ファイル名をセット
			auto_sprintf( pBase, L"%02d", i );

			hFind = ::FindFirstFile( szPath, &fData );
			if( hFind == INVALID_HANDLE_VALUE ){
				//	検索に失敗した == ファイルは存在しない
				break;
			}
			::FindClose( hFind );
			//	見つかったファイルの属性をチェック
			//	は面倒くさいからしない．
			//	同じ名前のディレクトリがあったらどうなるのだろう...
		}
		--i;

		//------------------------------------------------------------------
		//	2. 最大値から制限数-1番までを削除
		int boundary = bup_setting.GetBackupCount();
		boundary = boundary > 0 ? boundary - 1 : 0;	//	最小値は0

		for( ; i >= boundary; --i ){
			//	ファイル名をセット
			auto_sprintf( pBase, L"%02d", i );
			if( ::DeleteFile( szPath ) == 0 ){
				::MessageBox( CEditWnd::getInstance()->GetHwnd(), szPath, LS(STR_BACKUP_ERR_DELETE), MB_OK );
				//	Jun.  5, 2005 genta 戻り値変更
				//	失敗しても保存は継続
				return 0;
				//	失敗した場合
				//	後で考える
			}
		}

		//	この位置でiは存在するバックアップファイルの最大番号を表している．

		//	3. そこから0番まではコピーしながら移動
		WCHAR szNewPath[MAX_PATH];
		WCHAR *pNewNrBase;

		wcscpy( szNewPath, szPath );
		pNewNrBase = szNewPath + wcslen( szNewPath ) - 2;

		for( ; i >= 0; --i ){
			//	ファイル名をセット
			auto_sprintf( pBase, L"%02d", i );
			auto_sprintf( pNewNrBase, L"%02d", i + 1 );

			//	ファイルの移動
			if( ::MoveFile( szPath, szNewPath ) == 0 ){
				//	失敗した場合
				//	後で考える
				::MessageBox( CEditWnd::getInstance()->GetHwnd(), szPath, LS(STR_BACKUP_ERR_MOVE), MB_OK );
				//	Jun.  5, 2005 genta 戻り値変更
				//	失敗しても保存は継続
				return 0;
			}
		}
	}
	//	To Here Aug. 16, 2000 genta

	/* バックアップの作成 */
	//	Aug. 21, 2005 genta 現在のファイルではなくターゲットファイルをバックアップするように
	WCHAR	szDrive[_MAX_DIR];
	WCHAR	szDir[_MAX_DIR];
	WCHAR	szFname[_MAX_FNAME];
	WCHAR	szExt[_MAX_EXT];
	_wsplitpath( szPath, szDrive, szDir, szFname, szExt );
	WCHAR	szPath2[MAX_PATH];
	auto_sprintf( szPath2, L"%s%s", szDrive, szDir );

	HANDLE			hFind;
	WIN32_FIND_DATA	fData;

	hFind = ::FindFirstFile( szPath2, &fData );
	if( hFind == INVALID_HANDLE_VALUE ){
		//	検索に失敗した == ファイルは存在しない
		::CreateDirectory( szPath2, NULL );
	}
	::FindClose( hFind );

	if( ::CopyFile( target_file, szPath, FALSE ) ){
		/* 正常終了 */
		//@@@ 2001.12.11 start MIK
		if( bup_setting.m_bBackUpDustBox && !dustflag ){	//@@@ 2002.03.23 ネットワーク・リムーバブルドライブでない
			WCHAR	szDustPath[_MAX_PATH+1];
			wcscpy(szDustPath, szPath);
			szDustPath[wcslen(szDustPath) + 1] = L'\0';
			SHFILEOPSTRUCT	fos;
			fos.hwnd   = CEditWnd::getInstance()->GetHwnd();
			fos.wFunc  = FO_DELETE;
			fos.pFrom  = szDustPath;
			fos.pTo    = NULL;
			fos.fFlags = FOF_ALLOWUNDO | FOF_SIMPLEPROGRESS | FOF_NOCONFIRMATION;	//ダイアログなし
			fos.fAnyOperationsAborted = true; //false;
			fos.hNameMappings = NULL;
			fos.lpszProgressTitle = NULL; //"バックアップファイルをごみ箱に移動しています...";
			if( ::SHFileOperation(&fos) == 0 ){
				/* 正常終了 */
			}else{
				/* エラー終了 */
			}
		}
		//@@@ 2001.12.11 end MIK
	}else{
		/* エラー終了 */
		//	Jun.  5, 2005 genta 戻り値変更
		return 3;
	}
	//	Jun.  5, 2005 genta 戻り値変更
	return 1;
}

/*! バックアップパスの作成

	@author aroka

	@param szNewPath [out] バックアップ先パス名
	@param newPathCount [in] szNewPathのサイズ
	@param target_file [in] バックアップ元パス名

	@retval true  成功
	@retval false バッファ不足

	@date 2005.11.29 aroka
		MakeBackUpから分離．書式を元にバックアップファイル名を作成する機能追加
	@date 2013.04.15 novice 指定フォルダのメタ文字列展開サポート

	@todo Advanced modeでの世代管理
*/
bool CBackupAgent::FormatBackUpPath(
	WCHAR*			szNewPath,
	size_t 			newPathCount,
	const WCHAR*	target_file
)
{
	WCHAR	szDrive[_MAX_DIR];
	WCHAR	szDir[_MAX_DIR];
	WCHAR	szFname[_MAX_FNAME];
	WCHAR	szExt[_MAX_EXT];
	WCHAR*	psNext;

	const CommonSetting_Backup& bup_setting = GetDllShareData().m_Common.m_sBackup;

	/* パスの分解 */
	_wsplitpath( target_file, szDrive, szDir, szFname, szExt );

	if( bup_setting.m_bBackUpFolder
	  && (!bup_setting.m_bBackUpFolderRM || !IsLocalDrive( target_file ))) {	/* 指定フォルダにバックアップを作成する */	// m_bBackUpFolderRM 追加	2010/5/27 Uchi
		WCHAR selDir[_MAX_PATH];
		CFileNameManager::ExpandMetaToFolder( bup_setting.m_szBackUpFolder, selDir, _countof(selDir) );
		if (GetFullPathName(selDir, _MAX_PATH, szNewPath, &psNext) == 0) {
			// うまく取れなかった
			wcscpy( szNewPath, selDir );
		}
		/* フォルダの最後が半角かつ'\\'でない場合は、付加する */
		AddLastYenFromDirectoryPath( szNewPath );
	}
	else{
		auto_sprintf( szNewPath, L"%s%s", szDrive, szDir );
	}

	/* 相対フォルダを挿入 */
	if( !bup_setting.m_bBackUpPathAdvanced ){
		__time64_t 	ltime = 0;
		struct	tm result = {0};
		wchar_t	szTime[64];
		wchar_t	szForm[64];

		WCHAR*	pBase;
		int     nBaseCount;
		pBase = szNewPath + wcslen( szNewPath );
		nBaseCount = newPathCount - wcslen( szNewPath );

		/* バックアップファイル名のタイプ 1=(.bak) 2=*_日付.* */
		switch( bup_setting.GetBackupType() ){
		case 1:
			if( -1 == auto_snprintf_s( pBase, nBaseCount, L"%s.bak", szFname ) ){
				return false;
			}
			break;
		case 5: //	Jun.  5, 2005 genta 1の拡張子を残す版
			if( -1 == auto_snprintf_s( pBase, nBaseCount, L"%s%s.bak", szFname, szExt ) ){
				return false;
			}
			break;
		case 2:	//	日付，時刻
			_tzset();
			_wstrdate( szTime );
			_time64( &ltime );				/* システム時刻を得ます */
			_localtime64_s( &result, &ltime );/* 現地時間に変換する */

			szForm[0] = L'\0';
			if( bup_setting.GetBackupOpt(BKUP_YEAR) ){	/* バックアップファイル名：日付の年 */
				wcscat( szForm, L"%Y" );
			}
			if( bup_setting.GetBackupOpt(BKUP_MONTH) ){	/* バックアップファイル名：日付の月 */
				wcscat( szForm, L"%m" );
			}
			if( bup_setting.GetBackupOpt(BKUP_DAY) ){	/* バックアップファイル名：日付の日 */
				wcscat( szForm, L"%d" );
			}
			if( bup_setting.GetBackupOpt(BKUP_HOUR) ){	/* バックアップファイル名：日付の時 */
				wcscat( szForm, L"%H" );
			}
			if( bup_setting.GetBackupOpt(BKUP_MIN) ){	/* バックアップファイル名：日付の分 */
				wcscat( szForm, L"%M" );
			}
			if( bup_setting.GetBackupOpt(BKUP_SEC) ){	/* バックアップファイル名：日付の秒 */
				wcscat( szForm, L"%S" );
			}
			/* YYYYMMDD時分秒 形式に変換 */
			wcsftime( szTime, _countof( szTime ) - 1, szForm, &result );
			if( -1 == auto_snprintf_s( pBase, nBaseCount, L"%s_%ls%s", szFname, szTime, szExt ) ){
				return false;
			}
			break;
	//	2001/06/12 Start by asa-o: ファイルに付ける日付を前回の保存時(更新日時)にする
		case 4:	//	日付，時刻
			{
				CFileTime ctimeLastWrite;
				GetLastWriteTimestamp( target_file, &ctimeLastWrite );

				szTime[0] = L'\0';
				if( bup_setting.GetBackupOpt(BKUP_YEAR) ){	/* バックアップファイル名：日付の年 */
					auto_sprintf(szTime,L"%d",ctimeLastWrite->wYear);
				}
				if( bup_setting.GetBackupOpt(BKUP_MONTH) ){	/* バックアップファイル名：日付の月 */
					auto_sprintf(szTime,L"%ls%02d",szTime,ctimeLastWrite->wMonth);
				}
				if( bup_setting.GetBackupOpt(BKUP_DAY) ){	/* バックアップファイル名：日付の日 */
					auto_sprintf(szTime,L"%ls%02d",szTime,ctimeLastWrite->wDay);
				}
				if( bup_setting.GetBackupOpt(BKUP_HOUR) ){	/* バックアップファイル名：日付の時 */
					auto_sprintf(szTime,L"%ls%02d",szTime,ctimeLastWrite->wHour);
				}
				if( bup_setting.GetBackupOpt(BKUP_MIN) ){	/* バックアップファイル名：日付の分 */
					auto_sprintf(szTime,L"%ls%02d",szTime,ctimeLastWrite->wMinute);
				}
				if( bup_setting.GetBackupOpt(BKUP_SEC) ){	/* バックアップファイル名：日付の秒 */
					auto_sprintf(szTime,L"%ls%02d",szTime,ctimeLastWrite->wSecond);
				}
				if( -1 == auto_sprintf_s( pBase, nBaseCount, L"%s_%ls%s", szFname, szTime, szExt ) ){
					return false;
				}
			}
			break;
	// 2001/06/12 End

		case 3: //	?xx : xx = 00~99, ?は任意の文字
		case 6: //	Jun.  5, 2005 genta 3の拡張子を残す版
			//	Aug. 15, 2000 genta
			//	ここでは作成するバックアップファイル名のみ生成する．
			//	ファイル名のRotationは確認ダイアログの後で行う．
			{
				//	Jun.  5, 2005 genta 拡張子を残せるように処理起点を操作する
				WCHAR* ptr;
				if( bup_setting.GetBackupType() == 3 ){
					ptr = szExt;
				}
				else {
					ptr = szExt + wcslen( szExt );
				}
				*ptr   = L'.';
				*++ptr = bup_setting.GetBackupExtChar();
				*++ptr = L'0';
				*++ptr = L'0';
				*++ptr = L'\0';
			}
			if( -1 == auto_snprintf_s( pBase, nBaseCount, L"%s%s", szFname, szExt ) ){
				return false;
			}
			break;
		}

	}else{ // 詳細設定使用する
		WCHAR szFormat[1024];

		switch( bup_setting.GetBackupTypeAdv() ){
		case 4:	//	ファイルの日付，時刻
			{
				// 2005.10.20 ryoji FindFirstFileを使うように変更
				CFileTime ctimeLastWrite;
				GetLastWriteTimestamp( target_file, &ctimeLastWrite );
				if( !GetDateTimeFormat( szFormat, _countof(szFormat), bup_setting.m_szBackUpPathAdvanced , ctimeLastWrite.GetSYSTEMTIME() ) ){
					return false;
				}
			}
			break;
		case 2:	//	現在の日付，時刻
		default:
			{
				// 2012.12.26 aroka	詳細設定のファイル保存日時と現在時刻で書式を合わせる
				SYSTEMTIME	SystemTime;
				// 2016.07.28 UTC→ローカル時刻に変更
				::GetLocalTime(&SystemTime);			// 現在時刻を取得

				if( !GetDateTimeFormat( szFormat, _countof(szFormat), bup_setting.m_szBackUpPathAdvanced , SystemTime ) ){
					return false;
				}
			}
			break;
		}

		{
			// make keys
			// $0-$9に対応するフォルダ名を切り出し
			WCHAR keybuff[1024];
			wcscpy( keybuff, szDir );
			CutLastYenFromDirectoryPath( keybuff );

			WCHAR *folders[10];
			{
				//	Jan. 9, 2006 genta VC6対策
				int idx;
				for( idx=0; idx<10; ++idx ){
					folders[idx] = 0;
				}
				folders[0] = szFname;

				for( idx=1; idx<10; ++idx ){
					WCHAR *cp = wcsrchr(keybuff, L'\\');
					if( cp != NULL ){
						folders[idx] = cp+1;
						*cp = L'\0';
					}
					else{
						break;
					}
				}
			}
			{
				// $0-$9を置換
				//wcscpy( szNewPath, L"" );
				WCHAR *q= szFormat;
				WCHAR *q2 = szFormat;
				while( *q ){
					if( *q==L'$' ){
						++q;
						if( isdigit(*q) ){
							q[-1] = L'\0';
							wcscat( szNewPath, q2 );
//							if( newPathCount <  auto_strlcat( szNewPath, q2, newPathCount ) ){
//								return false;
//							}
							if( folders[*q-L'0'] != 0 ){
								wcscat( szNewPath, folders[*q-L'0'] );
//								if( newPathCount < auto_strlcat( szNewPath, folders[*q-L'0'], newPathCount ) ){
//									return false;
//								}
							}
							q2 = q+1;
						}
					}
					++q;
				}
				wcscat( szNewPath, q2 );
//				if( newPathCount < auto_strlcat( szNewPath, q2, newPathCount ) ){
//					return false;
//				}
			}
		}
		{
			WCHAR temp[1024];
			WCHAR *cp;
			//	2006.03.25 Aroka szExt[0] == '\0'のときのオーバラン問題を修正
			WCHAR *ep = (szExt[0]!=0) ? &szExt[1] : &szExt[0];
			assert( newPathCount <= _countof(temp) );

			// * を拡張子にする
			while( wcschr( szNewPath, L'*' ) ){
				wcscpy( temp, szNewPath );
				cp = wcschr( temp, L'*' );
				*cp = 0;
				if( -1 == auto_snprintf_s( szNewPath, newPathCount, L"%s%s%s", temp, ep, cp+1 ) ){
					return false;
				}
			}
			//	??はバックアップ連番にしたいところではあるが，
			//	連番処理は末尾の2桁にしか対応していないので
			//	使用できない文字?を_に変換してお茶を濁す
			while(( cp = wcschr( szNewPath, L'?' ) ) != NULL){
				*cp = L'_';
			}
		}
	}
	return true;
}

