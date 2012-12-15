#include "StdAfx.h"
#include "CViewCommander.h"
#include "CPropertyManager.h"
#include "view/CEditView.h"
#include "doc/CEditDoc.h"
#include "CEditApp.h"
#include "CPropertyManager.h"

/*!	入力補完
	Ctrl+Spaceでここに到着。
	CEditView::m_bHokan： 現在補完ウィンドウが表示されているかを表すフラグ。
	m_Common.m_sHelper.m_bUseHokan：現在補完ウィンドウが表示されているべきか否かをあらわすフラグ。

    @date 2001/06/19 asa-o 英大文字小文字を同一視する
                     候補が1つのときはそれに確定する
	@date 2001/06/14 asa-o 参照データ変更
	                 開くプロパティシートをタイプ別に変更
	@date 2000/09/15 JEPRO [Esc]キーと[x]ボタンでも中止できるように変更
	@date 2005/01/10 genta CEditView_Commandから移動
*/
void CViewCommander::Command_HOKAN( void )
{
	if(!GetDllShareData().m_Common.m_sHelper.m_bUseHokan){
		GetDllShareData().m_Common.m_sHelper.m_bUseHokan = TRUE;
	}
#if 0
// 2011.06.24 Moca Plugin導入に従い未設定の確認をやめる
retry:;
	/* 補完候補一覧ファイルが設定されていないときは、設定するように促す。 */
	// 2003.06.22 Moca ファイル内から検索する場合には補完ファイルの設定は必須ではない
	if( GetDocument()->m_cDocType.GetDocumentAttribute().m_bUseHokanByFile == FALSE &&
		_T('\0') == GetDocument()->m_cDocType.GetDocumentAttribute().m_szHokanFile[0]
	){
		ConfirmBeep();
		if( IDYES == ::ConfirmMessage( GetMainWindow(),
			_T("補完候補一覧ファイルが設定されていません。\n今すぐ設定しますか?")
		) ){
			/* タイプ別設定 プロパティシート */
			if( !CEditApp::getInstance()->m_pcPropertyManager->OpenPropertySheetTypes( 2, GetDocument()->m_cDocType.GetDocumentType() ) ){
				return;
			}
			goto retry;
		}
	}
#endif
	CNativeW	cmemData;
	/* カーソル直前の単語を取得 */
	if( 0 < m_pCommanderView->GetParser().GetLeftWord( &cmemData, 100 ) ){
		m_pCommanderView->ShowHokanMgr( cmemData, TRUE );
	}else{
		InfoBeep(); //2010.04.03 Error→Info
		m_pCommanderView->SendStatusMessage(_T("補完対象がありません")); // 2010.05.29 ステータスで表示
		GetDllShareData().m_Common.m_sHelper.m_bUseHokan = FALSE;	//	入力補完終了のお知らせ
	}
	return;
}
