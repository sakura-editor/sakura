/*!	@file
	@brief 文書ウィンドウの管理

	@author Norio Nakatani
	@date	1998/03/13 作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta, jepro
	Copyright (C) 2001, asa-o, MIK, hor, Misaka, Stonee, YAZAKI
	Copyright (C) 2002, genta, hor, YAZAKI, Azumaiya, KK, novice, minfu, ai, aroka, MIK
	Copyright (C) 2003, genta, MIK, Moca
	Copyright (C) 2004, genta, Moca, novice, Kazika, isearch
	Copyright (C) 2005, genta, Moca, MIK, ryoji, maru
	Copyright (C) 2006, genta, aroka, fon, yukihane, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _CEDITVIEW_H_
#define _CEDITVIEW_H_

#define _CARETMARGINRATE 20

#include <windows.h>
#include "CShareData.h"
#include "CTipWnd.h"
#include "CDicMgr.h"
#include "CHokanMgr.h"
//	Jun. 26, 2001 genta	正規表現ライブラリの差し替え
#include "CBregexp.h"
#include "CEOL.h"

class CDropTarget; /// 2002/2/3 aroka ヘッダ軽量化
class CMemory;///
class COpe;///
class COpeBlk;///
class CSplitBoxWnd;///
class CDlgCancel;///
class CRegexKeyword;///
class CAutoMarkMgr; /// 2002/2/3 aroka ヘッダ軽量化 to here
class CEditDoc;	//	2002/5/13 YAZAKI ヘッダ軽量化
class CLayout;	//	2002/5/13 YAZAKI ヘッダ軽量化
class CDocLine;
class CMigemo;	// 2004.09.14 isearch

#ifndef IDM_COPYDICINFO
#define IDM_COPYDICINFO 2000
#endif
#ifndef IDM_JUMPDICT
#define IDM_JUMPDICT 2001	// 2006.04.10 fon
#endif

#if !defined(RECONVERTSTRING) && (WINVER < 0x040A)
typedef struct tagRECONVERTSTRING {
    DWORD dwSize;
    DWORD dwVersion;
    DWORD dwStrLen;
    DWORD dwStrOffset;
    DWORD dwCompStrLen;
    DWORD dwCompStrOffset;
    DWORD dwTargetStrLen;
    DWORD dwTargetStrOffset;
} RECONVERTSTRING, *PRECONVERTSTRING;
#endif // RECONVERTSTRING

///	マウスからコマンドが実行された場合の上位ビット
///	@date 2006.05.19 genta
const int CMD_FROM_MOUSE = 2;

class CEditView;

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
class SAKURA_CORE_API CCaretUnderLine
{
public:
	CCaretUnderLine(){
		m_nLockCounter = 0;
	};
	void Lock(){	//	表示非表示を切り替えられないようにする
		m_nLockCounter++;
	}
	void UnLock(){	//	表示非表示を切り替えられるようにする
		m_nLockCounter--;
		if (m_nLockCounter < 0){
			m_nLockCounter = 0;
		};
	}
	void CaretUnderLineON( BOOL );								/* カーソル行アンダーラインのON */
	void CaretUnderLineOFF( BOOL );								/* カーソル行アンダーラインのOFF */
	void SetView( CEditView* pcEditView ){
		m_pcEditView = pcEditView;
	};
protected:
	/* ロックカウンタ。0のときは、ロックされていない。UnLockが呼ばれすぎても負にはならない */
	int m_nLockCounter;
	CEditView* m_pcEditView;
};

/*!
	@brief 文書ウィンドウの管理
	
	1つの文書ウィンドウにつき1つのCEditDocオブジェクトが割り当てられ、
	1つのCEditDocオブジェクトにつき、4つのCEditViweオブジェクトが割り当てられる。
	ウィンドウメッセージの処理、コマンドメッセージの処理、
	画面表示などを行う。
	
	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
*/
class SAKURA_CORE_API CEditView/* : public CDropTarget*/
{
public:
	/* Constructors */
	CEditView();
	~CEditView();
	/* 初期化系メンバ関数 */
	BOOL Create( HINSTANCE, HWND, CEditDoc*, int,/* BOOL,*/ BOOL );
	/* 状態 */
	//<< 2002/03/29 Azumaiya インライン関数化
	BOOL IsTextSelected( void )		/* テキストが選択されているか */
	{
		// ジャンプ回数を減らして、一気に判定。
		// すべてを or 演算した後に、ビット反転して最上位ビット(符号フラグ)を返す。
		/* すなわち、いずれかがひとつでも-1(0xFFFF)なら、FALSEを返す？ */
		return ~((DWORD)(m_nSelectLineFrom|m_nSelectLineTo|m_nSelectColmFrom|m_nSelectColmTo)) >> 31;
	};
	BOOL IsTextSelecting( void )	/* テキストの選択中か */
	{
		// ジャンプ回数を減らして、一気に判定。
		return m_bSelectingLock|IsTextSelected();
	};
	//>> 2002/03/29 Azumaiya
	//	Oct. 2, 2005 genta 挿入モードの設定・取得
	bool IsInsMode() const;
	void SetInsMode(bool);

	/* メッセージディスパッチャ */
	LRESULT DispatchEvent( HWND, UINT, WPARAM, LPARAM );
	void OnMove( int, int, int, int );
	void OnSetFocus( void );
	void OnKillFocus( void );
	void OnTimer( HWND, UINT, UINT, DWORD );
	BOOL HandleCommand( int, BOOL, LPARAM, LPARAM, LPARAM, LPARAM );
	/* コマンド操作 */
	void CaretUnderLineON( BOOL );								/* カーソル行アンダーラインのON */
	void CaretUnderLineOFF( BOOL );								/* カーソル行アンダーラインのOFF */
	void AdjustScrollBars( void );								/* スクロールバーの状態を更新する */
	int  MoveCursor( int, int, BOOL, int = _CARETMARGINRATE );	/* 行桁指定によるカーソル移動 */
	// 2006.07.09 genta 行桁指定によるカーソル移動(選択領域を考慮)
	void MoveCursorSelecting( int, int, BOOL, int = _CARETMARGINRATE );
	BOOL GetAdjustCursorPos( int *, int *);	// 正しいカーソル位置を算出する
	BOOL DetectWidthOfLineNumberArea( BOOL );					/* 行番号表示に必要な幅を設定 */
	int DetectWidthOfLineNumberArea_calculate( void );			/* 行番号表示に必要な桁数を計算 */
	void DisableSelectArea( BOOL );								/* 現在の選択範囲を非選択状態に戻す */
	void OnChangeSetting( void );								/* 設定変更を反映させる */
	void SetFont( void );										/* フォントの変更 */
	void RedrawAll( void );										/* フォーカス移動時の再描画 */
	void Redraw( void );										// 2001/06/21 asa-o 再描画
	void CopyViewStatus( CEditView* );							/* 自分の表示状態を他のビューにコピー */
	void SplitBoxOnOff( BOOL, BOOL, BOOL );						/* 縦・横の分割ボックス・サイズボックスのＯＮ／ＯＦＦ */
	DWORD DoGrep( const CMemory*, const CMemory*, const CMemory*, BOOL, BOOL, BOOL, int, BOOL, BOOL, int );	/* Grep実行 */
	/* Grep実行 */	//	Jun. 26, 2001 genta	正規表現ライブラリの差し替え
	int DoGrepTree( CDlgCancel*, HWND, const char*, int*, const char*, const char*, BOOL, BOOL, BOOL, int, BOOL, BOOL, int, CBregexp*, int, int* );
	/* Grep実行 */	//	Jun. 26, 2001 genta	正規表現ライブラリの差し替え
	//	Mar. 28, 2004 genta 不要な引数を削除
	int DoGrepFile( CDlgCancel*, HWND, const char*, int*, const char*, BOOL, BOOL, int, BOOL, BOOL, int, CBregexp*, int*, const char*, CMemory& );
	/* Grep結果をpszWorkに格納 */
	void SetGrepResult(
		/* データ格納先 */
		char*		pWork,
		int*		pnWorkLen,
		/* マッチしたファイルの情報 */
		const char*		pszFullPath,	//	フルパス
		const char*		pszCodeName,	//	文字コード情報"[SJIS]"とか
		/* マッチした行の情報 */
		int			nLine,			//	マッチした行番号
		int			nColm,			//	マッチした桁番号
		const char*		pCompareData,	//	行の文字列
		int			nLineLen,		//	行の文字列の長さ
		int			nEolCodeLen,	//	EOLの長さ
		/* マッチした文字列の情報 */
		const char*		pMatchData,		//	マッチした文字列
		int			nMatchLen,		//	マッチした文字列の長さ
		/* オプション */
		BOOL		bGrepOutputLine,
		int			nGrepOutputStyle
	);
	
	//	Jun. 16, 2000 genta
	bool  SearchBracket( int PosX, int PosY, int* NewX, int* NewY, int* mode );	// 対括弧の検索		// modeの追加 02/09/18 ai
	bool  SearchBracketForward( int PosX, int PosY, int* NewX, int* NewY,
						char* upChar, char* dnChar, int* mode );	//	対括弧の前方検索	// modeの追加 02/09/19 ai
	bool  SearchBracketBackward( int PosX, int PosY, int* NewX, int* NewY,
						char* dnChar, char* upChar, int* mode );	//	対括弧の後方検索	// modeの追加 02/09/19 ai
//@@@ 2001.02.03 Start by MIK: 全角の対括弧
	bool  SearchBracketForward2( int PosX, int PosY, int* NewX, int* NewY,
								 char* upChar, char* dnChar, int* mode );	//	対括弧の前方検索	// modeの追加 02/09/19 ai
	bool  SearchBracketBackward2( int PosX, int PosY, int* NewX, int* NewY,
								  char* dnChar, char* upChar, int* mode );	//	対括弧の後方検索	// modeの追加 02/09/19 ai
//@@@ 2001.02.03 End

//	2001/06/18 asa-o
	bool  ShowKeywordHelp( POINT po, LPCTSTR pszHelp, LPRECT prcHokanWin);	// 補完ウィンドウ用のキーワードヘルプ表示

// 2002/01/19 novice public属性に変更
	BOOL GetSelectedData( CMemory&, BOOL, const char*, BOOL, BOOL bAddCRLFWhenCopy, enumEOLType neweol = EOL_UNKNOWN);/* 選択範囲のデータを取得 */
	//	Aug. 25, 2002 genta protected->publicに移動
	bool IsImeON( void );	// IME ONか	// 2006.12.04 ryoji
	void CreateEditCaret( COLORREF crCaret, COLORREF crBack, int nWidth, int nHeight );		/* キャレットの作成 */	// 2006.12.07 ryoji
	void ShowEditCaret( void );									/* キャレットの表示・更新 */
	int HokanSearchByFile( const char*, BOOL, CMemory**, int, int ); // 2003.06.25 Moca

	/*!	CEditView::KeyWordHelpSearchDictのコール元指定用ローカルID
		@date 2006.04.10 fon 新規作成
	*/
	enum LID_SKH {
		LID_SKH_ONTIMER		= 1,	/*!< CEditView::OnTimer */
		LID_SKH_POPUPMENU_R = 2,	/*!< CEditView::CreatePopUpMenu_R */
	};
	BOOL KeyWordHelpSearchDict( LID_SKH nID, POINT* po, RECT* rc );	// 2006.04.10 fon
	BOOL KeySearchCore( const CMemory* pcmemCurText );	// 2006.04.10 fon
	
	//	Jan. 10, 2005 インクリメンタルサーチ
	bool IsISearchEnabled(int nCommand) const;

	//	Jan.  8, 2006 genta 折り返しトグル動作判定
	enum TOGGLE_WRAP_ACTION {
		TGWRAP_NONE = 0,
		TGWRAP_FULL,
		TGWRAP_WINDOW,
		TGWRAP_PROP,
	};
	TOGGLE_WRAP_ACTION GetWrapMode( int& );

public: /* テスト用にアクセス属性を変更 */
	CDropTarget*	m_pcDropTarget;
	BOOL			m_bDrawSWITCH;
	BOOL			m_bDragSource;	/* 選択テキストのドラッグ中か */
	BOOL			m_bDragMode;	/* 選択テキストのドラッグ中か */

	/* 単語検索の状態 */

	//	Jun. 26, 2001 genta	正規表現ライブラリの差し替え
	CBregexp	m_CurRegexp;			/*!< コンパイルデータ */
	BOOL	m_bCurSrchKeyMark;			/* 検索文字列のマーク */
	char	m_szCurSrchKey[_MAX_PATH];	/* 検索文字列 */
	int		m_bCurSrchRegularExp;		/* 検索／置換  1==正規表現 */
	int		m_bCurSrchLoHiCase;			/* 検索／置換  1==英大文字小文字の区別 */
	int		m_bCurSrchWordOnly;			/* 検索／置換  1==単語のみ検索 */

	BOOL	m_bExecutingKeyMacro;		/* キーボードマクロの実行中 */
//	BOOL	m_bGrepRunning;
//	HANDLE	m_hThreadGrep;	@@@ 2002.01.03 YAZAKI 使用していないため
	HWND	m_hWnd;				/* 編集ウィンドウハンドル */
	int		m_nViewTopLine;		/* 表示域の一番上の行(0開始) */
	int		m_nViewLeftCol;		/* 表示域の一番左の桁(0開始) */
	/*!	@brief 直前のX座標記憶用

		フリーカーソルモードでない場合にカーソルを上下に移動させた場合
		カーソル位置より短い行では行末にカーソルを移動するが，
		さらに移動を続けた場合に長い行で移動起点のX位置を復元できるように
		するための変数．
		
		@par 使い方
		読み出しはCEditView::Cursor_UPDOWN()のみで行う．
		カーソル上下移動以外でカーソル移動を行った場合には
		直ちにm_nCaretPosXの値を設定する．そうしないと
		その直後のカーソル上下移動で移動前のX座標に戻ってしまう．
	
		ビュー左端からのカーソル桁位置(０開始)
		
		@date 2004.04.09 genta 説明文追加
	*/
	int		m_nCaretPosX_Prev;
	int		m_nCaretPosX;		/* ビュー左端からのカーソル桁位置（０開始）*/
	int		m_nCaretPosY;		/* ビュー上端からのカーソル行位置（０開始）*/
	int		m_nCaretPosX_PHY;	/* カーソル位置  改行単位行先頭からのバイト数（０開始）*/
	int		m_nCaretPosY_PHY;	/* カーソル位置  改行単位行の行番号（０開始）*/

	int		m_nSrchStartPosX_PHY;	/* 検索/置換開始時のカーソル位置  改行単位行先頭からのバイト数(0開始) */	// 02/06/26 ai
	int		m_nSrchStartPosY_PHY;	/* 検索/置換開始時のカーソル位置  改行単位行の行番号(0開始) */				// 02/06/26 ai
	BOOL	m_bSearch;				/* 検索/置換開始位置を登録するか */											// 02/06/26 ai
	int		m_nCharSize;			/* 対括弧の文字サイズ */	// 02/09/18 ai 
	int		m_nBracketCaretPosX_PHY;/* 前カーソル位置の括弧の位置 改行単位行先頭からのバイト数(0開始) */	// 03/02/18 ai
	int		m_nBracketCaretPosY_PHY;/* 前カーソル位置の括弧の位置 改行単位行の行番号(0開始) */				// 03/02/18 ai
	int		m_nBracketPairPosX_PHY;	/* 対括弧の位置 改行単位行先頭からのバイト数(0開始) */	// 02/12/13 ai
	int		m_nBracketPairPosY_PHY;	/* 対括弧の位置 改行単位行の行番号(0開始) */			// 02/12/13 ai
	BOOL	m_bDrawBracketPairFlag;	/* 対括弧の強調表示を行なうか */						// 03/02/18 ai
	BOOL	m_bDrawSelectArea;		/* 選択範囲を描画したか */	// 02/12/13 ai

	/*
	||  メンバ変数
	*/
	char*			m_pszAppName;	/* Mutex作成用・ウィンドウクラス名 */
	HINSTANCE		m_hInstance;	/* インスタンスハンドル */
	HWND			m_hwndParent;	/* 親ウィンドウハンドル */

	DLLSHAREDATA*	m_pShareData;
//	int				m_nSettingType;

	CEditDoc*		m_pcEditDoc;		/* ドキュメント */
	COpeBlk*		m_pcOpeBlk;			/* 操作ブロック */
	BOOL			m_bDoing_UndoRedo;	/* アンドゥ・リドゥの実行中か */
	HWND			m_hwndVScrollBar;	/* 垂直スクロールバーウィンドウハンドル */
	int				m_nVScrollRate;		/* 垂直スクロールバーの縮尺 */
	HWND			m_hwndHScrollBar;	/* 水平スクロールバーウィンドウハンドル */
	HWND			m_hwndSizeBox;		/* サイズボックスウィンドウハンドル */
	CSplitBoxWnd*	m_pcsbwVSplitBox;	/* 垂直分割ボックス */
	CSplitBoxWnd*	m_pcsbwHSplitBox;	/* 水平分割ボックス */
	//	Dec. 4, 2002 genta
	//	メニューバーへのメッセージ表示機能はCEditWndへ移管
	//HFONT			m_hFontCaretPosInfo;/* キャレットの行桁位置表示用フォント */
	//int			m_nCaretPosInfoCharWidth;
	//int			m_nCaretPosInfoCharHeight;
	//int			m_pnCaretPosInfoDx[64];	/* 文字列描画用文字幅配列 */
	HDC				m_hdcCompatDC;		/* 再描画用コンパチブルＤＣ */
	HBITMAP			m_hbmpCompatBMP;	/* 再描画用メモリＢＭＰ */
	HBITMAP			m_hbmpCompatBMPOld;	/* 再描画用メモリＢＭＰ(OLD) */
//@@@2002.01.14 YAZAKI staticにしてメモリの節約（(10240+10) * 3 バイト）
	static int		m_pnDx[MAXLINESIZE + 10];	/* 文字列描画用文字幅配列 */
	HFONT			m_hFont_HAN;		/* 現在のフォントハンドル */
	HFONT			m_hFont_HAN_FAT;	/* 現在のフォントハンドル */
	HFONT			m_hFont_HAN_UL;		/* 現在のフォントハンドル */
	HFONT			m_hFont_HAN_FAT_UL;	/* 現在のフォントハンドル */
//	2004.02.14 Moca m_hFont_ZENは、未使用につき削除
//	HFONT			m_hFont_ZEN;		/* 現在のフォントハンドル */
	HFONT			m_hFontOld;
	BOOL			m_bCommandRunning;	/* コマンドの実行中 */
	/* 分割状態 */
	int				m_nMyIndex;
	/* キーリピート状態 */
	int				m_bPrevCommand;
	/* 入力状態 */
	int		m_nCaretWidth;			/* キャレットの幅 */
	int		m_nCaretHeight;			/* キャレットの高さ */
	COLORREF	m_crCaret;			/* キャレットの色 */			// 2006.12.07 ryoji
	COLORREF	m_crBack;			/* テキストの背景色 */			// 2006.12.07 ryoji
	HBITMAP	m_hbmpCaret;			/* キャレットのビットマップ */	// 2006.11.28 ryoji
	CCaretUnderLine m_cUnderLine;	/* アンダーライン */
	int		m_nOldUnderLineY;

	int		m_nOldCaretPosX;	// 前回描画したルーラーのキャレット位置 2002.02.25 Add By KK
	int		m_nOldCaretWidth;	// 前回描画したルーラーのキャレット幅   2002.02.25 Add By KK
	bool	m_bRedrawRuler;		// ルーラー全体を描き直す時 = true      2002.02.25 Add By KK

	/* 選択状態 */
	int		m_bSelectingLock;		/* 選択状態のロック */
	int		m_bBeginSelect;			/* 範囲選択中 */
	int		m_bBeginBoxSelect;		/* 矩形範囲選択中 */
	int		m_bBeginLineSelect;		/* 行単位選択中 */
	int		m_bBeginWordSelect;		/* 単語単位選択中 */
	/* 選択範囲を保持するための変数群
		これらはすべて折り返し行と、折り返し桁を保持している。
	*/
	int		m_nSelectLineBgnFrom;	/* 範囲選択開始行(原点) */
	int		m_nSelectColmBgnFrom;	/* 範囲選択開始桁(原点) */
	int		m_nSelectLineBgnTo;		/* 範囲選択開始行(原点) */
	int		m_nSelectColmBgnTo;		/* 範囲選択開始桁(原点) */
	int		m_nSelectLineFrom;		/* 範囲選択開始行 */
	int		m_nSelectColmFrom;		/* 範囲選択開始桁 */
	int		m_nSelectLineTo;		/* 範囲選択終了行 */
	int		m_nSelectColmTo;		/* 範囲選択終了桁 */
	/* DrawSelectArea()に現在の選択範囲を教えて差分のみ描画するためのもの
	   現在の選択範囲をOldへコピーした上で新しい選択範囲をSelectに設定して
	   DrawSelectArea()を呼びだすことで新しい範囲が描かれる．
	*/
	int		m_nSelectLineFromOld;	/* 範囲選択開始行 */
	int		m_nSelectColmFromOld;	/* 範囲選択開始桁 */
	int		m_nSelectLineToOld;		/* 範囲選択終了行 */
	int		m_nSelectColmToOld;		/* 範囲選択終了桁 */
	int		m_nMouseRollPosXOld;	/* マウス範囲選択前回位置(X座標) */
	int		m_nMouseRollPosYOld;	/* マウス範囲選択前回位置(Y座標) */
	/* 画面情報 */
	int		m_nViewAlignLeft;		/* 表示域の左端座標 */
	int		m_nViewAlignLeftCols;	/* 行番号域の桁数 */
	int		m_nViewAlignTop;		/* 表示域の上端座標 */
	int		m_nTopYohaku;
	int		m_nViewCx;				/* 表示域の幅 */
	int		m_nViewCy;				/* 表示域の高さ */
	int		m_nViewColNum;			/* 表示域の桁数 */
	int		m_nViewRowNum;			/* 表示域の行数 */
	/* 表示方法 */
	int		m_nCharWidth;			/* 半角文字の幅 */
	int		m_nCharHeight;			/* 文字の高さ */
	/* 辞書Tip関連 */
	DWORD	m_dwTipTimer;			/* Tip起動タイマー */
	CTipWnd	m_cTipWnd;				/* Tip表示ウィンドウ */
	POINT	m_poTipCurPos;			/* Tip起動時のマウスカーソル位置 */
	BOOL	m_bInMenuLoop;			/* メニュー モーダル ループに入っています */
	CDicMgr	m_cDicMgr;				/* 辞書マネージャ */
	/* 入力補完 */
//	CHokanMgr	m_cHokanMgr;
	BOOL		m_bHokan;			//	補完中か？＝補完ウィンドウが表示されているか？かな？
	//	Aug. 31, 2000 genta
	CAutoMarkMgr	*m_cHistory;	//	Jump履歴
	CRegexKeyword	*m_cRegexKeyword;	//@@@ 2001.11.17 add MIK

	/*
	||  実装ヘルパ関数
	*/
	void GetCurrentTextForSearch( CMemory& );			/* 現在カーソル位置単語または選択範囲より検索等のキーを取得 */
	void DestroyCaret( void ){	/* キャレットを破棄する（内部的にも破棄）*/
		::DestroyCaret();
		m_nCaretWidth = 0;
	}
protected:
//	CEOL GetCurrentInsertEOL( void );					/* 現在、Enterなどで挿入する改行コードの種類を取得 */

	BOOL MyGetClipboardData( CMemory&, BOOL* );			/* クリップボードからデータを取得 */
	BOOL MySetClipboardData( const char*, int, BOOL );	/* クリップボードにデータを設定 */
	int GetLeftWord( CMemory*, int );					/* カーソル直前の単語を取得 */
	BOOL GetCurrentWord( CMemory* );					/* キャレット位置の単語を取得 */	// 2006.03.24 fon
//	void PrintBitmap( HDC, int, int, const char* );		/* ビットマップファイル表示 */
//	HANDLE OpenDIB ( LPCSTR );							/* DIBファイルを開いてメモリDIBを作成 */
//	HANDLE ReadDibBitmapInfo ( int );					/* DIB形式のファイルを読む */
//	BOOL DibInfo ( HANDLE, LPBITMAPINFOHEADER );		/* CF_DIB形式のメモリブロックに関連付けられているDIB情報を取得します */
//	WORD PaletteSize ( VOID FAR * pv );					/* パレットのバイト数を計算します */
//	WORD DibNumColors ( VOID FAR * );					/* 情報ブロックのBitCountメンバを参照して、DIBの色数を判断します */
//	DWORD lread ( int, void*, DWORD );					/* データをすべて読み取る */
//	void TraceRgn( HRGN );								/* デバッグ用 リージョン矩形のダンプ */
//	void OnPaintOld( HDC, PAINTSTRUCT *, BOOL );		/* 通常の描画処理 */
	void OnPaint( HDC, PAINTSTRUCT *, BOOL );			/* 通常の描画処理 */
//	int DispLine( HDC, int, int, int, const unsigned char*, int, BOOL );/		* 行のテキスト／選択状態の描画 */
	int DispLineNew( HDC, const CLayout*, int&, int, int&, BOOL, int, BOOL );	/* 行のテキスト／選択状態の描画 */
	void DispLineNumber( HDC, const CLayout*, int, int );		/* 行番号表示 */
	void SetCurrentColor( HDC, int );							/* 現在の色を指定 */
	void DispRuler( HDC );										/* ルーラー描画 */
	void DrawRulerCaret( HDC hdc );								// ルーラーのキャレットのみ描画 2002.02.25 Add By KK
	int	DispText( HDC, int, int, const char*, int );	/* テキスト表示 */	//@@@ 2002.09.22 YAZAKI
	void DispTextSelected( HDC, int, int, int, int );			/* テキスト反転 */
//	int	DispCtrlCode( HDC, int, int, const unsigned char*, int );	/* コントロールコード表示 */
	void TwoPointToRect( RECT*, int, int, int, int );			/* 2点を対角とする矩形を求める */
	void DrawSelectArea( void );								/* 指定行の選択領域の描画 */
	void DrawSelectAreaLine( HDC, int, int, int, int, int );	/* 指定行の選択領域の描画 */
	int  LineColmnToIndex( const CLayout* pcLayout, int nColumn );		/* 指定された桁に対応する行のデータ内の位置を調べる Ver1 */		// @@@ 2002.09.28 YAZAKI
	int  LineColmnToIndex( const CDocLine* pcDocLine, int nColumn );		/* 指定された桁に対応する行のデータ内の位置を調べる Ver1 */		// @@@ 2002.09.28 YAZAKI
	int  LineColmnToIndex2( const CLayout* pcLayout, int nColumn, int& pnLineAllColLen );	/* 指定された桁に対応する行のデータ内の位置を調べる Ver0 */		// @@@ 2002.09.28 YAZAKI
	int  LineIndexToColmn( const CLayout* pcLayout, int nIndex );		/* 指定された行のデータ内の位置に対応する桁の位置を調べる */	// @@@ 2002.09.28 YAZAKI
	int  LineIndexToColmn( const CDocLine* pcLayout, int nIndex );		/* 指定された行のデータ内の位置に対応する桁の位置を調べる */	// @@@ 2002.09.28 YAZAKI
	void CopySelectedAllLines( const char*, BOOL );				/* 選択範囲内の全行をクリップボードにコピーする */
	void ConvSelectedArea( int );								/* 選択エリアのテキストを指定方法で変換 */
	void ConvMemory( CMemory*, int );							/* 機能種別によるバッファの変換 */
	void OnSize( int, int );									/* ウィンドウサイズの変更処理 */
	int  OnVScroll( int, int );								/* 垂直スクロールバーメッセージ処理 */
	int  OnHScroll( int, int );								/* 水平スクロールバーメッセージ処理 */
	void OnLBUTTONDOWN( WPARAM, int, int );						/* マウス左ボタン押下 */
	void OnMOUSEMOVE( WPARAM, int, int );						/* マウス移動のメッセージ処理 */
	void OnLBUTTONUP( WPARAM, int, int );						/* マウス左ボタン開放のメッセージ処理 */
	void OnLBUTTONDBLCLK( WPARAM, int , int );					/* マウス左ボタンダブルクリック */
	void OnRBUTTONDOWN( WPARAM, int, int );						/* マウス右ボタン押下 */
	void OnRBUTTONUP( WPARAM, int, int );						/* マウス右ボタン開放 */
// novice 2004/10/11 マウス中ボタン対応
	void OnMBUTTONDOWN( WPARAM, int, int );						/* マウス中ボタン押下 */
// novice 2004/10/10 マウスサイドボタン対応
	void OnXLBUTTONDOWN( WPARAM, int, int );					/* マウス左サイドボタン押下 */
	void OnXRBUTTONDOWN( WPARAM, int, int );					/* マウス右サイドボタン押下 */
	LRESULT OnMOUSEWHEEL( WPARAM, LPARAM );						/* マウスホイールのメッセージ処理 */
	/*! 選択範囲を指定する(原点未選択)

		@date 2005.06.24 Moca
	*/
	void SetSelectArea( int nLineFrom, int nColmFrom, int nLineTo, int nColmTo ){
		m_nSelectLineBgnFrom = nLineFrom;
		m_nSelectColmBgnFrom = nColmFrom;
		m_nSelectLineBgnTo = nLineFrom;
		m_nSelectColmBgnTo = nColmFrom;
		m_nSelectLineFrom = nLineFrom;
		m_nSelectColmFrom = nColmFrom;
		m_nSelectLineTo = nLineTo;
		m_nSelectColmTo = nColmTo;
	}
	void BeginSelectArea( void );								/* 現在のカーソル位置から選択を開始する */
	void ChangeSelectAreaByCurrentCursor( int, int );			/* 現在のカーソル位置によって選択範囲を変更 */
	void ChangeSelectAreaByCurrentCursorTEST( int, int, int&, int&, int&, int& );/* 現在のカーソル位置によって選択範囲を変更 */
	void PrintSelectionInfoMsg(void);		// 選択情報データの作成	2005.07.09 genta
	int  MoveCursorToPoint( int, int );							/* マウス等による座標指定によるカーソル移動 */
	int  ScrollAtV( int );										/* 指定上端行位置へスクロール */
	int  ScrollAtH( int );										/* 指定左端桁位置へスクロール */
	//	From Here Sep. 11, 2004 genta ずれ維持の同期スクロール
	int  ScrollByV( int vl ){	return ScrollAtV( m_nViewTopLine + vl );}			/* 指定行スクロール*/
	int  ScrollByH( int hl ){	return ScrollAtH( m_nViewLeftCol + hl );}					/* 指定桁スクロール */
	void SyncScrollV( int );									/* 垂直同期スクロール */
	void SyncScrollH( int );									/* 水平同期スクロール */
	//	To Here Sep. 11, 2004 genta ずれ維持の同期スクロール
	int Cursor_UPDOWN( int, int );								/* カーソル上下移動処理 */
	void DrawBracketPair( bool );								/* 対括弧の強調表示 02/09/18 ai */
	void SetBracketPairPos( bool );								/* 対括弧の強調表示位置設定 03/02/18 ai */
	int GetColorIndex( HDC, const CLayout*, int );				/* 指定位置のColorIndexの取得 02/12/13 ai */
	bool IsBracket( const char*, int, int );					/* 括弧判定 03/01/09 ai */
public:
	void SetIMECompFormPos( void );								/* IME編集エリアの位置を変更 */
protected:
	void SetIMECompFormFont( void );							/* IME編集エリアの表示フォントを変更 */
	void SetParentCaption( BOOL = FALSE );						/* 親ウィンドウのタイトルを更新 */
public:
	void DrawCaretPosInfo( void );								/* キャレットの行桁位置を表示する */
protected:
	BOOL IsCurrentPositionURL( int, int, int*, int*, int*, char** );/* カーソル位置にURLが有る場合のその範囲を調べる */
	int IsCurrentPositionSelected( int, int );					/* 指定カーソル位置が選択エリア内にあるか */
	int IsCurrentPositionSelectedTEST( int, int, int, int, int, int );/* 指定カーソル位置が選択エリア内にあるか */
	BOOL IsSearchString( const char*, int, int, int*, int* );	/* 現在位置が検索文字列に該当するか */	//2002.02.08 hor 引数追加
	HFONT ChooseFontHandle( BOOL bFat, BOOL bUnderLine );		/* フォントを選ぶ */
	void ExecCmd(const char*, BOOL ) ;							// 子プロセスの標準出力をリダイレクトする
	void AddToCmdArr( const char* );
	BOOL ChangeCurRegexp(void);									// 2002.01.16 hor 正規表現の検索パターンを必要に応じて更新する(ライブラリが使用できないときはFALSEを返す)
	void SendStatusMessage( const char* msg );					// 2002.01.26 hor 検索／置換／ブックマーク検索時の状態をステータスバーに表示する
	void SendStatusMessage2( const char* msg );					// Jul. 9, 2005 genta
//  以下の二つはつかわなくなりました。 minfu 2002.04.10
//	LRESULT RequestedReconversion( PRECONVERTSTRING pReconv);	/*  IMEからの再変換要求に答える minfu 2002.03.27 */
//	LRESULT RequestedReconversionW( PRECONVERTSTRING pReconv);	/*  IMEからの再変換要求に答える for 95/NT 20020331 aroka */
	LRESULT SetReconvertStruct(PRECONVERTSTRING pReconv, bool bUnicode);	/* 再変換用構造体を設定する 2002.04.09 minfu */
	LRESULT SetSelectionFromReonvert(PRECONVERTSTRING pReconv, bool bUnicode);				/* 再変換用構造体の情報を元に選択範囲を変更する 2002.04.09 minfu */

public: /* テスト用にアクセス属性を変更 */
	/* IDropTarget実装 */
	STDMETHODIMP DragEnter( LPDATAOBJECT, DWORD, POINTL, LPDWORD );
	STDMETHODIMP DragOver(DWORD, POINTL, LPDWORD );
	STDMETHODIMP DragLeave( void );
	STDMETHODIMP Drop( LPDATAOBJECT, DWORD, POINTL, LPDWORD );
protected:
//	LPDATAOBJECT	m_pDataObject;
//	REFIID			m_owniid;

	/* ファイル操作系 */
	void Command_FILENEW( void );				/* 新規作成 */
	/* ファイルを開く */
	// Oct. 2, 2001 genta マクロ用に機能拡張
	// Mar. 30, 2003 genta 引数追加
	void Command_FILEOPEN( const char *filename = NULL,
		int nCharCode = CODE_AUTODETECT, BOOL bReadOnly = FALSE );
	
	/* 上書き保存 */ // Feb. 28, 2004 genta 引数追加, Jan. 24, 2005 genta 引数追加
	BOOL Command_FILESAVE( bool warnbeep = true, bool askname = true );	
	BOOL Command_FILESAVEAS_DIALOG();			/* 名前を付けて保存 */
	BOOL Command_FILESAVEAS( const char *filename );			/* 名前を付けて保存 */
	BOOL Command_FILESAVEALL( void );	/* 全て上書き保存 */ // Jan. 23, 2005 genta
	void Command_FILECLOSE( void );				/* 開じて(無題) */	//Oct. 17, 2000 jepro 「ファイルを閉じる」というキャプションを変更
	/* 閉じて開く*/
	// Mar. 30, 2003 genta 引数追加
	void Command_FILECLOSE_OPEN( const char *filename = NULL,
		int nCharCode = CODE_AUTODETECT, BOOL bReadOnly = FALSE );
	
	void Command_FILE_REOPEN( int, int );			/* 再オープン */	//Dec. 4, 2002 genta 引数追加
#if 0
	2002/04/19 YAZAKI
	void Command_FILE_REOPEN_SJIS( void );		/* SJISで開き直す */
	void Command_FILE_REOPEN_JIS( void );		/* JISで開き直す */
	void Command_FILE_REOPEN_EUC( void );		/* EUCで開き直す */
	void Command_FILE_REOPEN_UNICODE( void );	/* Unicodeで開き直す */
	void Command_FILE_REOPEN_UTF8( void );		/* UTF-8で開き直す */
	void Command_FILE_REOPEN_UTF7( void );		/* UTF-7で開き直す */
#endif
	void Command_PRINT( void );					/* 印刷*/
	void Command_PRINT_PREVIEW( void );			/* 印刷プレビュー*/
	void Command_PRINT_PAGESETUP( void );		/* 印刷ページ設定 */	//Sept. 14, 2000 jepro 「印刷のページレイアウトの設定」から変更
	BOOL Command_OPEN_HfromtoC( BOOL );			/* 同名のC/C++ヘッダ(ソース)を開く */	//Feb. 7, 2001 JEPRO 追加
	BOOL Command_OPEN_HHPP( BOOL bCheckOnly, BOOL bBeepWhenMiss );				/* 同名のC/C++ヘッダファイルを開く */	//Feb. 9, 2001 jepro「.cまたは.cppと同名の.hを開く」から変更
	BOOL Command_OPEN_CCPP( BOOL bCheckOnly, BOOL bBeepWhenMiss );				/* 同名のC/C++ソースファイルを開く */	//Feb. 9, 2001 jepro「.hと同名の.c(なければ.cpp)を開く」から変更
	void Command_ACTIVATE_SQLPLUS( void );		/* Oracle SQL*Plusをアクティブ表示 */
	void Command_PLSQL_COMPILE_ON_SQLPLUS( void );/* Oracle SQL*Plusで実行 */
	void Command_BROWSE( void );				/* ブラウズ */
	void Command_READONLY( void );				/* 読み取り専用 */
	void Command_PROPERTY_FILE( void );			/* ファイルのプロパティ */
	void Command_EXITALL( void );				/* サクラエディタの全終了 */	//Dec. 27, 2000 JEPRO 追加

	/* 編集系 */
	void Command_CHAR( char );				/* 文字入力 */
	void Command_IME_CHAR( WORD );			/* 全角文字入力 */
	void Command_UNDO( void );				/* 元に戻す(Undo) */
	void Command_REDO( void );				/* やり直し(Redo) */
	void Command_DELETE( void );			/* カーソル位置または選択エリアを削除 */
	void Command_DELETE_BACK( void );		/* カーソル前を削除 */
	void Command_WordDeleteToStart( void );	/* 単語の左端まで削除 */
	void Command_WordDeleteToEnd( void );	/* 単語の右端まで削除 */
	void Command_WordCut( void );			/* 単語切り取り */
	void Command_WordDelete( void );		/* 単語削除 */
	void Command_LineCutToStart( void );	//行頭まで切り取り(改行単位)
	void Command_LineCutToEnd( void );		//行末まで切り取り(改行単位)
	void Command_LineDeleteToStart( void );	/* 行頭まで削除(改行単位) */
	void Command_LineDeleteToEnd( void );  	//行末まで削除(改行単位)
	void Command_CUT_LINE( void );			/* 行切り取り(折り返し単位) */
	void Command_DELETE_LINE( void );		/* 行削除(折り返し単位) */
	void Command_DUPLICATELINE( void );		/* 行の二重化(折り返し単位) */
	void Command_INDENT( char cChar );		/* インデント ver 1 */
// From Here 2001.12.03 hor
//	void Command_INDENT( const char*, int );/* インデント ver0 */
	void Command_INDENT( const char*, int ,BOOL=FALSE );/* インデント ver0 */
	void Command_INDENT_TAB( void );		/* インデント ver 2 */
// To Here 2001.12.03 hor
	void Command_UNINDENT( char cChar );	/* 逆インデント */
//	void Command_WORDSREFERENCE( void );	/* 単語リファレンス */
	void Command_TRIM(BOOL);				// 2001.12.03 hor
	void Command_TRIM2(CMemory*,BOOL);		// 2001.12.03 hor
	void Command_SORT(BOOL);				// 2001.12.06 hor
	void Command_MERGE(void);				// 2001.12.06 hor
	void Command_Reconvert(void);			/* メニューからの再変換対応 minfu 2002.04.09 */
	void Command_CtrlCode_Dialog(void);		/* コントロールコードの入力(ダイアログ) */	//@@@ 2002.06.02 MIK

	/* 指定位置の指定長データ削除 */
	void CEditView::DeleteData2(
		int			nCaretX,
		int			nCaretY,
		int			nDelLen,
		CMemory*	pcMem,
		COpe*		pcOpe		/* 編集操作要素 COpe */
//		BOOL		bRedraw,
//		BOOL		bRedraw2
	);
	void DeleteData( BOOL bRedraw );/* 現在位置のデータ削除 */
	/* 現在位置にデータを挿入 */
	void InsertData_CEditView(
		int			nX,
		int			nY,
		const char*	pData,
		int			nDataLen,
		int*		pnNewLine,			/* 挿入された部分の次の位置の行 */
		int*		pnNewPos,			/* 挿入された部分の次の位置のデータ位置 */
		COpe*		pcOpe,				/* 編集操作要素 COpe */
		BOOL		bRedraw
	);
	void SmartIndent_CPP( char );	/* C/C++スマートインデント処理 */
	void RTrimPrevLine( void );		/* 2005.10.11 ryoji 前の行にある末尾の空白を削除 */

	/* カーソル移動系 */
	//	Oct. 24, 2001 genta 機能拡張のため引数追加
	int Command_UP( int, BOOL, int line = 0 );			/* カーソル上移動 */
	int Command_DOWN( int, BOOL );			/* カーソル下移動 */
	int  Command_LEFT( int, BOOL );			/* カーソル左移動 */
	void Command_RIGHT( int, int, BOOL );	/* カーソル右移動 */
	void Command_UP2( int );				/* カーソル上移動（２行づつ） */
	void Command_DOWN2( int );				/* カーソル下移動（２行づつ） */
	void Command_WORDLEFT( int );			/* 単語の左端に移動 */
	void Command_WORDRIGHT( int );			/* 単語の右端に移動 */
	//	Oct. 29, 2001 genta マクロ向け機能拡張
	void Command_GOLINETOP( int bSelect, int lparam );	/* 行頭に移動（折り返し単位） */
	void Command_GOLINEEND( int, int );		/* 行末に移動（折り返し単位） */
//	void Command_ROLLDOWN( int );			/* スクロールダウン */
//	void Command_ROLLUP( int );				/* スクロールアップ */
	void Command_HalfPageUp( int );			//半ページアップ	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
	void Command_HalfPageDown( int );		//半ページダウン	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
	void Command_1PageUp( int );			//１ページアップ	//Oct. 10, 2000 JEPRO 従来のページアップを半ページアップと名称変更し１ページアップを追加
	void Command_1PageDown( int );			//１ページダウン	//Oct. 10, 2000 JEPRO 従来のページダウンを半ページダウンと名称変更し１ページダウンを追加
	void Command_GOFILETOP( int );			/* ファイルの先頭に移動 */
	void Command_GOFILEEND( int );			/* ファイルの最後に移動 */
	void Command_CURLINECENTER( void );		/* カーソル行をウィンドウ中央へ */
	void Command_JUMPHIST_PREV(void);		// 移動履歴: 前へ
	void Command_JUMPHIST_NEXT(void);		// 移動履歴: 次へ
	void Command_JUMPHIST_SET(void);		// 現在位置を移動履歴に登録
	void Command_WndScrollDown(void);		// テキストを１行下へスクロール	// 2001/06/20 asa-o
	void Command_WndScrollUp(void);			// テキストを１行上へスクロール	// 2001/06/20 asa-o
	void Command_GONEXTPARAGRAPH( int bSelect );	// 次の段落へ進む
	void Command_GOPREVPARAGRAPH( int bSelect );	// 前の段落へ戻る

	/* 選択系 */
	bool Command_SELECTWORD( void );		/* 現在位置の単語選択 */
	void Command_SELECTALL( void );			/* すべて選択 */
	void Command_BEGIN_SELECT( void );		/* 範囲選択開始 */

	/* 矩形選択系 */
//	void Command_BOXSELECTALL( void );		/* 矩形ですべて選択 */
	void Command_BEGIN_BOXSELECT( void );	/* 矩形範囲選択開始 */
	int Command_UP_BOX( BOOL );				/* (矩形選択)カーソル上移動 */
//	int Command_DOWN( int, BOOL );			/* カーソル下移動 */
//	int  Command_LEFT( int, BOOL );			/* カーソル左移動 */
//	void Command_RIGHT( int, int, BOOL );	/* カーソル右移動 */
//	void Command_UP2( int );				/* カーソル上移動（２行づつ） */
//	void Command_DOWN2( int );				/* カーソル下移動（２行づつ） */
//	void Command_WORDLEFT( int );			/* 単語の左端に移動 */
//	void Command_WORDRIGHT( int );			/* 単語の右端に移動 */
//	void Command_GOLINETOP( int, BOOL );	/* 行頭に移動（折り返し単位） */
//	void Command_GOLINEEND( int, int );		/* 行末に移動（折り返し単位） */
//	void Command_HalfPageUp( int );			//半ページアップ	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
//	void Command_HalfPageDown( int );		//半ページダウン	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
//	void Command_1PageUp( int );			//１ページアップ	//Oct. 10, 2000 JEPRO 従来のページアップを半ページアップと名称変更し１ページアップを追加
//	void Command_1PageDown( int );			//１ページダウン	//Oct. 10, 2000 JEPRO 従来のページダウンを半ページダウンと名称変更し１ページダウンを追加
//	void Command_GOFILETOP( int );			/* ファイルの先頭に移動 */
//	void Command_GOFILEEND( int );			/* ファイルの最後に移動 */

	/* クリップボード系 */
	void Command_CUT( void );						/* 切り取り（選択範囲をクリップボードにコピーして削除）*/
	void Command_COPY( int, BOOL bAddCRLFWhenCopy, enumEOLType neweol = EOL_UNKNOWN );/* コピー(選択範囲をクリップボードにコピー) */
	void Command_PASTE( void );						/* 貼り付け（クリップボードから貼り付け）*/
	void Command_PASTEBOX( void );					/* 矩形貼り付け（クリップボードから矩形貼り付け）*/
	//<< 2002/03/29 Azumaiya
	/* 矩形貼り付け（引数渡しでの張り付け）*/
	void Command_PASTEBOX( const char *szPaste, int nPasteSize );
	//>> 2002/03/29 Azumaiya
//	void Command_INSTEXT( BOOL, const char*, int );	/* テキストを貼り付け ver0 */
	//void Command_INSTEXT( BOOL, const char*, BOOL );/* テキストを貼り付け ver1 */
	void Command_INSTEXT( BOOL, const char*, int, BOOL ); // 2004.05.14 Moca テキストを貼り付け '\0'対応
	void Command_ADDTAIL( const char*, int );		/* 最後にテキストを追加 */
	void Command_COPYFILENAME( void );				/* このファイル名をクリップボードにコピー */ //2002/2/3 aroka
	void Command_COPYPATH( void );					/* このファイルのパス名をクリップボードにコピー */
	void Command_COPYTAG( void );					/* このファイルのパス名とカーソル位置をコピー */
	void Command_COPYLINES( void );					/* 選択範囲内全行コピー */
	void Command_COPYLINESASPASSAGE( void );		/* 選択範囲内全行引用符付きコピー */
	void Command_COPYLINESWITHLINENUMBER( void );	/* 選択範囲内全行行番号付きコピー */
	void Command_CREATEKEYBINDLIST( void );			// キー割り当て一覧をコピー //Sept. 15, 2000 JEPRO	Command_の作り方がわからないので殺してある


	/* データ置換 削除&挿入にも使える */
void ReplaceData_CEditView(
	int			nDelLineFrom,		/* 削除範囲行  From レイアウト行番号 */
	int			nDelColmFrom,		/* 削除範囲位置From レイアウト行桁位置 */
	int			nDelLineTo,			/* 削除範囲行  To   レイアウト行番号 */
	int			nDelColmTo,			/* 削除範囲位置To   レイアウト行桁位置 */
	CMemory*	pcmemCopyOfDeleted,	/* 削除されたデータのコピー(NULL可能) */
	const char*	pInsData,			/* 挿入するデータ */
	int			nInsDataLen,		/* 挿入するデータの長さ */
//	int*		pnAddLineNum,		/* 再描画ヒント レイアウト行の増減 */
//	int*		pnModLineFrom,		/* 再描画ヒント 変更されたレイアウト行From(レイアウト行の増減が0のとき使う) */
//	int*		pnModLineTo,		/* 再描画ヒント 変更されたレイアウト行From(レイアウト行の増減が0のとき使う) */
//	BOOL		bDispSSTRING,		/* シングルクォーテーション文字列を表示する */
//	BOOL		bDispWSTRING,		/* ダブルクォーテーション文字列を表示する */
//	int			nX,
//	int			nY,
//	const char*	pData,
//	int			nDataLen,
//	int*		pnNewLine,	/* 挿入された部分の次の位置の行 */
//	int*		pnNewPos,	/* 挿入された部分の次の位置のデータ位置 */
//	COpe*		pcOpe,		/* 編集操作要素 COpe */
	BOOL		bRedraw
);

	/* 挿入系 */
	void Command_INS_DATE( void );	//日付挿入
	void Command_INS_TIME( void );	//時刻挿入

	/* 変換系 */
	void Command_TOLOWER( void );				/* 英大文字→英小文字 */
	void Command_TOUPPER( void );				/* 英小文字→英大文字 */
	void Command_TOZENKAKUKATA( void );			/* 半角＋全ひら→全角・カタカナ */	//Sept. 17, 2000 jepro 説明を「半角→全角カタカナ」から変更
	void Command_TOZENKAKUHIRA( void );			/* 半角＋全カタ→全角・ひらがな */	//Sept. 17, 2000 jepro 説明を「半角→全角ひらがな」から変更
	void Command_TOHANKAKU( void );				/* 全角→半角 */
	void Command_TOHANKATA( void );				/* 全角カタカナ→半角カタカナ */	//Aug. 29, 2002 ai
	void Command_TOZENEI( void );				/* 半角英数→全角英数 */ //July. 30, 2001 Misaka
	void Command_TOHANEI( void );				/* 全角英数→半角英数 */ //@@@ 2002.2.11 YAZAKI
	void Command_HANKATATOZENKAKUKATA( void );	/* 半角カタカナ→全角カタカナ */
	void Command_HANKATATOZENKAKUHIRA( void );	/* 半角カタカナ→全角ひらがな */
	void Command_TABTOSPACE( void );			/* TAB→空白 */
	void Command_SPACETOTAB( void );			/* 空白→TAB */  //#### Stonee, 2001/05/27
	void Command_CODECNV_AUTO2SJIS( void );		/* 自動判別→SJISコード変換 */
	void Command_CODECNV_EMAIL( void );			/* E-Mail(JIS→SJIS)コード変換 */
	void Command_CODECNV_EUC2SJIS( void );		/* EUC→SJISコード変換 */
	void Command_CODECNV_UNICODE2SJIS( void );	/* Unicode→SJISコード変換 */
	void Command_CODECNV_UNICODEBE2SJIS( void );	/* UnicodeBE→SJISコード変換 */
	void Command_CODECNV_UTF82SJIS( void );		/* UTF-8→SJISコード変換 */
	void Command_CODECNV_UTF72SJIS( void );		/* UTF-7→SJISコード変換 */
	void Command_CODECNV_SJIS2JIS( void );		/* SJIS→JISコード変換 */
	void Command_CODECNV_SJIS2EUC( void );		/* SJIS→EUCコード変換 */
	void Command_CODECNV_SJIS2UTF8( void );		/* SJIS→UTF-8コード変換 */
	void Command_CODECNV_SJIS2UTF7( void );		/* SJIS→UTF-7コード変換 */
	void Command_BASE64DECODE( void );			/* Base64デコードして保存 */
	void Command_UUDECODE( void );				/* uudecodeして保存 */	//Oct. 17, 2000 jepro 説明を「選択部分をUUENCODEデコード」から変更

	/* 検索系 */
	void Command_SEARCH_BOX( void );					/* 検索(ボックス) */	// 2006.06.04 yukihane
	void Command_SEARCH_DIALOG( void );					/* 検索(単語検索ダイアログ) */
	void Command_SEARCH_NEXT( bool, BOOL, HWND, const char* );/* 次を検索 */
	void Command_SEARCH_PREV( BOOL, HWND );				/* 前を検索 */
	void Command_REPLACE_DIALOG( void );				/* 置換(置換ダイアログ) */
	void Command_REPLACE( HWND hwndParent );			/* 置換(実行) 2002/04/08 YAZAKI 親ウィンドウを指定するように変更 */
	enum REPLACE_ALL_MODE { REP_NORMAL = 0, REP_LINE = 1 };
	void Command_REPLACE_ALL( int nMode = REP_NORMAL );	/* すべて置換(実行) */
	void Command_SEARCH_CLEARMARK( void );				/* 検索マークのクリア */
	void Command_JUMP_SRCHSTARTPOS( void );				/* 検索開始位置へ戻る */	// 02/06/26 ai

	//	Jan. 10, 2005 genta HandleCommandからgrep関連処理を分離
	void TranslateCommand_grep( int&, BOOL&, LPARAM&, LPARAM&, LPARAM&, LPARAM& );
	void Command_GREP_DIALOG( void );					/* Grepダイアログの表示 */
	void Command_GREP( void );							/* Grep */
	void Command_JUMP_DIALOG( void );					/* 指定行ヘジャンプダイアログの表示 */
	void Command_JUMP( void );							/* 指定行ヘジャンプ */
// From Here 2001.12.03 hor
//	BOOL Command_FUNCLIST( BOOL );						/* アウトライン解析 */
//	BOOL Command_FUNCLIST( BOOL ,int=OUTLINE_DEFAULT );	/* アウトライン解析 */
	BOOL Command_FUNCLIST( int ,int=OUTLINE_DEFAULT );	/* アウトライン解析 */ // 20060201 aroka
// To Here 2001.12.03 hor
	// Apr. 03, 2003 genta 引数追加
	bool Command_TAGJUMP( bool bClose = false );		/* タグジャンプ機能 */
	void Command_TAGJUMPBACK( void );					/* タグジャンプバック機能 */
	bool Command_TagJumpByTagsFile( void );				//ダイレクトタグジャンプ	//@@@ 2003.04.13 MIK
	//@@@ 2003.04.13 MIK, Apr. 21, 2003 genta bClose追加
	bool TagJumpSub( const char *pszJumpToFile, int nLine, int nColumn, bool bClose = false );
	bool Command_TagsMake( void );						//タグファイルの作成	//@@@ 2003.04.13 MIK
	bool Command_TagJumpByTagsFileKeyword( const char* keyword );	//	@@ 2005.03.31 MIK
	void Command_COMPARE( void );						/* ファイル内容比較 */
	void Command_Diff_Dialog( void );					/* DIFF差分表示ダイアログ */	//@@@ 2002.05.25 MIK
	void Command_Diff( const char*, int );	/* DIFF差分表示 */	//@@@ 2002.05.25 MIK	// 2005.10.03 maru
	void Command_Diff_Next( void );						/* 次の差分へ */	//@@@ 2002.05.25 MIK
	void Command_Diff_Prev( void );						/* 前の差分へ */	//@@@ 2002.05.25 MIK
	void Command_Diff_Reset( void );					/* 差分の全解除 */	//@@@ 2002.05.25 MIK
	void Command_BRACKETPAIR( void );					/* 対括弧の検索 */
// From Here 2001.12.03 hor
	void Command_BOOKMARK_SET( void );					/* ブックマーク設定・解除 */
	void Command_BOOKMARK_NEXT( void );					/* 次のブックマークへ */
	void Command_BOOKMARK_PREV( void );					/* 前のブックマークへ */
	void Command_BOOKMARK_RESET( void );				/* ブックマークの全解除 */
// To Here 2001.12.03 hor
	void Command_BOOKMARK_PATTERN( void );				// 2002.01.16 hor 指定パターンに一致する行をマーク

//2004.10.13 インクリメンタルサーチ関係
	void TranslateCommand_isearch( int&, BOOL&, LPARAM&, LPARAM&, LPARAM&, LPARAM& );
	bool ProcessCommand_isearch( int, BOOL, LPARAM, LPARAM, LPARAM, LPARAM );

	/* モード切り替え系 */
	void Command_CHGMOD_INS( void );	/* 挿入／上書きモード切り替え */
	void Command_CHGMOD_EOL( enumEOLType );	/* 入力する改行コードを設定 2003.06.23 moca */
	void Command_CANCEL_MODE( void );	/* 各種モードの取り消し */

	/* 設定系 */
	void Command_SHOWTOOLBAR( void );		/* ツールバーの表示/非表示 */
	void Command_SHOWFUNCKEY( void );		/* ファンクションキーの表示/非表示 */
	void Command_SHOWTAB( void );			/* タブの表示/非表示 */	//@@@ 2003.06.10 MIK
	void Command_SHOWSTATUSBAR( void );		/* ステータスバーの表示/非表示 */
	void Command_TYPE_LIST( void );			/* タイプ別設定一覧 */
	void Command_OPTION_TYPE( void );		/* タイプ別設定 */
	void Command_OPTION( void );			/* 共通設定 */
	void Command_FONT( void );				/* フォント設定 */
	void Command_WRAPWINDOWWIDTH( void );	/* 現在のウィンドウ幅で折り返し */	//Oct. 7, 2000 JEPRO WRAPWINDIWWIDTH を WRAPWINDOWWIDTH に変更
	void Command_Favorite( void );	//お気に入り	//@@@ 2003.04.08 MIK
	void Command_SET_QUOTESTRING( const char* );	//	Jan. 29, 2005 genta 引用符の設定

	/* マクロ系 */
	void Command_RECKEYMACRO( void );	/* キーマクロの記録開始／終了 */
	void Command_SAVEKEYMACRO( void );	/* キーマクロの保存 */
	void Command_LOADKEYMACRO( void );	/* キーマクロの読み込み */
	void Command_EXECKEYMACRO( void );	/* キーマクロの実行 */
//	From Here Sept. 20, 2000 JEPRO 名称CMMANDをCOMMANDに変更
//	void Command_EXECCMMAND( void );	/* 外部コマンド実行 */
	//	Oct. 9, 2001 genta マクロ対応のため機能拡張
	void Command_EXECCOMMAND_DIALOG( const char* cmd );	/* 外部コマンド実行ダイアログ表示 */
	void Command_EXECCOMMAND( const char* cmd );	/* 外部コマンド実行 */
//	To Here Sept. 20, 2000

	/* カスタムメニュー */
	void Command_MENU_RBUTTON( void );	/* 右クリックメニュー */
	int	CreatePopUpMenu_R( void );		/* ポップアップメニュー(右クリック) */
	int Command_CUSTMENU( int );		/* カスタムメニュー表示 */

	/* ウィンドウ系 */
	void Command_SPLIT_V( void );		/* 上下に分割 */	//Sept. 17, 2000 jepro 説明の「縦」を「上下に」に変更
	void Command_SPLIT_H( void );		/* 左右に分割 */	//Sept. 17, 2000 jepro 説明の「横」を「左右に」に変更
	void Command_SPLIT_VH( void );		/* 縦横に分割 */	//Sept. 17, 2000 jepro 説明に「に」を追加
	void Command_WINCLOSE( void );		/* ウィンドウを閉じる */
	void Command_FILECLOSEALL( void );	/* すべてのウィンドウを閉じる */	//Oct. 7, 2000 jepro 「編集ウィンドウの全終了」という説明を左記のように変更
	void Command_BIND_WINDOW( void );	/* 結合して表示 */	//2004.07.14 Kazika 新規追加
	void Command_CASCADE( void );		/* 重ねて表示 */
	void Command_TILE_V( void );		/* 上下に並べて表示 */
	void Command_TILE_H( void );		/* 左右に並べて表示 */
	void Command_MAXIMIZE_V( void );	/* 縦方向に最大化 */
	void Command_MAXIMIZE_H( void );	/* 横方向に最大化 */  //2001.02.10 by MIK
	void Command_MINIMIZE_ALL( void );	/* すべて最小化 */
	void Command_REDRAW( void );		/* 再描画 */
	void Command_WIN_OUTPUT( void );	//アウトプットウィンドウ表示
	void Command_TRACEOUT( const char* outputstr , int );	//マクロ用アウトプットウィンドウに表示 maru 2006.04.26
	void Command_WINTOPMOST( LPARAM );		// 常に手前に表示 2004.09.21 Moca
	void Command_WINLIST( int nCommandFrom );		/* ウィンドウ一覧ポップアップ表示処理 */	// 2006.03.23 fon // 2006.05.19 genta 引数追加

	/* 支援 */
	//	Jan. 10, 2005 genta HandleCommandから補完関連処理を分離
	void PreprocessCommand_hokan( int nCommand );
	void PostprocessCommand_hokan(void);
	void Command_ToggleKeySearch( void );	/* キャレット位置の単語を辞書検索する機能ON-OFF */	// 2006.03.24 fon

	void ShowHokanMgr( CMemory& cmemData, BOOL bAutoDecided );	//	補完ウィンドウを表示する。Ctrl+Spaceや、文字の入力/削除時に呼び出されます。 YAZAKI 2002/03/11
	void Command_HOKAN( void );			/* 入力補完 */
	void Command_HELP_CONTENTS( void );	/* ヘルプ目次 */			//Nov. 25, 2000 JEPRO added
	void Command_HELP_SEARCH( void );	/* ヘルプキーワード検索 */	//Nov. 25, 2000 JEPRO added
	void Command_MENU_ALLFUNC( void );	/* コマンド一覧 */
	void Command_EXTHELP1( void );		/* 外部ヘルプ１ */
	//	Jul. 5, 2002 genta
	void Command_EXTHTMLHELP( const char* helpfile = NULL, const char* kwd = NULL );	/* 外部HTMLヘルプ */
	void Command_ABOUT( void );			/* バージョン情報 */	//Dec. 24, 2000 JEPRO 追加

	/* その他 */
//@@@ 2002.01.14 YAZAKI 不使用のため
//	void Command_SENDMAIL( void );		/* メール送信 */

	//	May 23, 2000 genta
	//	画面描画補助関数
//@@@ 2001.12.21 YAZAKI 改行記号の書きかたが変だったので修正
//	void DrawEOL(HDC hdc, HANDLE hPen, int nPosX, int nPosY, int nWidth, int nHeight,
//		CEOL cEol, int bBold );
	void DrawEOL(HDC hdc, int nPosX, int nPosY, int nWidth, int nHeight,
		CEOL cEol, int bBold, COLORREF pColor );
	//タブ矢印描画関数	//@@@ 2003.03.26 MIK
	void DrawTabArrow( HDC hdc, int nPosX, int nPosY, int nWidth, int nHeight, int bBold, COLORREF pColor );
	//	EOF描画関数	//	2004.05.29 genta
	int  DispEOF( HDC hdc, int x, int y, int nCharWidth, int nLineHeight, UINT fuOptions, const ColorInfo& EofColInfo );
	//!	指定桁縦線描画関数	// 2005.11.08 Moca
	void DispVerticalLines( HDC, int, int, int, int );

	//	Aug. 31, 2000 genta
	void AddCurrentLineToHistory(void);	//現在行を履歴に追加する

	void AnalyzeDiffInfo( const char*, int );	/* DIFF情報の解析 */	//@@@ 2002.05.25 MIK
	BOOL MakeDiffTmpFile( char*, HWND );	/* DIFF一時ファイル作成 */	//@@@ 2002.05.28 MIK	//2005.10.29 maru
	void ViewDiffInfo( const char*, const char*, int );		/* DIFF差分表示 */		//2005.10.29 maru
	
	BOOL OPEN_ExtFromtoExt( BOOL, BOOL, const char* [], const char* [], int, int, const char* ); // 指定拡張子のファイルに対応するファイルを開く補助関数 // 2003.08.12 Moca

// 2002/07/22 novice
	void ShowCaret_( HWND hwnd );
	void HideCaret_( HWND hwnd );

private:
	UINT	m_uMSIMEReconvertMsg;
	UINT	m_uATOKReconvertMsg;
	UINT	m_uWM_MSIME_RECONVERTREQUEST;
	
	int		m_nLastReconvLine;             //2002.04.09 minfu 再変換情報保存用;
	int		m_nLastReconvIndex;            //2002.04.09 minfu 再変換情報保存用;

	//ATOK専用再変換のAPI
	HMODULE m_hAtokModule;
	BOOL (WINAPI *AT_ImmSetReconvertString)( HIMC , int ,PRECONVERTSTRING , DWORD  );
	
	bool	m_bUnderLineON;
	bool	m_bCaretShowFlag;

	/* インクリメンタルサーチ */ 
	//2004.10.24 isearch migemo
	CMigemo* m_pcmigemo;
	void ISearchEnter( int mode  ,int direction);
	void ISearchExit();
	void ISearchExec(WORD wChar);
	void ISearchExec(const char* pszText);
	void ISearchExec(bool bNext);
	void ISearchBack(void) ;
	void ISearchWordMake(void);
	void ISearchSetStatusMsg(CMemory* msg) const;
	char* m_pszMigemoWord;
	int m_nISearchDirection;
	int m_nISearchMode;
	bool m_bISearchWrap;
	int m_nISearchX1History[256];
	int m_nISearchY1History[256];
	int m_nISearchX2History[256];
	int m_nISearchY2History[256];
	bool m_bISearchFlagHistory[256];
	int m_nISearchHistoryCount;
	bool m_bISearchFirst;
};



///////////////////////////////////////////////////////////////////////
#endif /* _CEDITVIEW_H_ */


/*[EOF]*/
