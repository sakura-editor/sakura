#pragma once

#include "doc/CDocListener.h"
#include "COpeBuf.h"

class CEditDoc;
class CDocLineMgr;

class CDocEditor : public CDocListenerEx{
public:
	CDocEditor(CEditDoc* pcDoc);

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         イベント                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//ロード前後
	void OnBeforeLoad(SLoadInfo* sLoadInfo);
	void OnAfterLoad(const SLoadInfo& sLoadInfo);

	//セーブ前後
	void OnAfterSave(const SSaveInfo& sSaveInfo);



	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           状態                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//	Jan. 22, 2002 genta Modified Flagの設定
	void SetModified( bool flag, bool redraw);
	//! ファイルが修正中かどうか
	bool IsModified() const { return m_bIsDocModified; }

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           設定                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//	Nov. 20, 2000 genta
	void SetImeMode(int mode);	//	IME状態の設定

	//	May 15, 2000 genta
	CEol  GetNewLineCode() const { return m_cNewLineCode; }
	void  SetNewLineCode(const CEol& t){ m_cNewLineCode = t; }

	//	Oct. 2, 2005 genta 挿入モードの設定
	bool IsInsMode() const { return m_bInsMode; }
	void SetInsMode(bool mode) { m_bInsMode = mode; }

	bool IsEnableUndo( void );				/* Undo(元に戻す)可能な状態か？ */
	bool IsEnableRedo( void );				/* Redo(やり直し)可能な状態か？ */
	bool IsEnablePaste( void );				/* クリップボードから貼り付け可能か？ */

public:
	CEditDoc*		m_pcDocRef;
	CEol 			m_cNewLineCode;				//!< Enter押下時に挿入する改行コード種別
	COpeBuf			m_cOpeBuf;					//!< アンドゥバッファ
	bool			m_bInsMode;					//!< 挿入・上書きモード Oct. 2, 2005 genta
	bool			m_bIsDocModified;
};


class CDocEditAgent{
public:
	CDocEditAgent(CDocLineMgr* pcDocLineMgr) : m_pcDocLineMgr(pcDocLineMgr) { }

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           操作                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//	May 15, 2000 genta
	void AddLineStrX( const wchar_t*, int );	/* 末尾に行を追加 Ver1.5 */

	void DeleteData_CDocLineMgr(
		CLogicInt	nLine,
		CLogicInt	nDelPos,
		CLogicInt	nDelLen,
		CLogicInt*	pnModLineOldFrom,	/* 影響のあった変更前の行(from) */
		CLogicInt*	pnModLineOldTo,		/* 影響のあった変更前の行(to) */
		CLogicInt*	pnDelLineOldFrom,	/* 削除された変更前論理行(from) */
		CLogicInt*	pnDelLineOldNum,	/* 削除された行数 */
		CNativeW*	cmemDeleted			/* 削除されたデータ */
	);

	/* データの挿入 */
	void InsertData_CDocLineMgr(
		CLogicInt		nLine,
		CLogicInt		nInsPos,
		const wchar_t*	pInsData,
		CLogicInt		nInsDataLen,
		CLogicInt*		pnInsLineNum,	// 挿入によって増えた行の数
		CLogicPoint*	pptNewPos		// 挿入された部分の次の位置
	);
private:
	CDocLineMgr* m_pcDocLineMgr;
};
