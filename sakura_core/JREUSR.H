// begin of jreusr.h
// ライブラリの使用者用ヘッダファイル

#ifndef		JREUSR
#define		JREUSR

// ---------------------------------------------------
// Prevent multiple includes.

// _os_os2はこれ(jreusr.h)をインクルードするユーザーが定義する
#if defined(_OS_OS2)
// in case of os/2
	// __OS2_H__ is for bc 2.0 e
	// __OS2_H__ is for bc 1.5 e
	#if !defined(__IBMC__) || !defined(__OS2_H__)
		// 他のクラスでtype.hをインクルードさせないように先にincludeする.
		// INCL_???など必要な定数は先に定義しておく.
		#include <os2.h>
	#endif	// defined(__IBMC__) || defined(__OS2_H__)

	#define JRE_BUILDTYPE_OS2PM

#else
// in case of windows

	// bc4.5 e __WINDOWS_H,
	// bc4.0 j __WINDOWS_H,
	// bc3.1 j __WINDOWS_H,
	// msvc4.1 j _INC_WINDOWS, _WINDOWS_
	// msvc4.0 j _INC_WINDOWS,
	// msvc2.0 j ???, ???
	// msvc1.0 j _INC_WINDOWS,
	// ddk _INC_WINDOWS
	// WinCE SDK - WIN32 and PEGASUS

	#if !defined(__WINDOWS_H) && !defined(_INC_WINDOWS)
		#ifndef STRICT
			#define STRICT
		#endif	// STRICT
		// 他のクラスでtype.hをインクルードさせないように先にincludeする.
		#include <windows.h>
	#endif	// !defined(__WINDOWS_H) && !defined(_INC_WINDOWS)

	#if defined(__WIN32__) || defined(WIN32)
		#define JRE_BUILDTYPE_WIN32
		#if defined(PEGASUS)
			#define JRE_SUBSET_WINCE
		#endif
	#else
		#define JRE_BUILDTYPE_WIN16
	#endif
	#define JRE_BUILDTYPE_WINDOWS

#endif	// os/2 or windows

// ---------------------------------------------------
#if defined(JRE_BUILDTYPE_OS2PM)
	// in case of JRE_BUILDTYPE_OS2PM
	// 以下，jre/os2ローカルの定義
	#if !defined(EXTAPI)
		#define EXTAPI _export EXPENTRY
	#endif	// !defined(EXTAPI)
	#if !defined(MEMID)
		typedef void* MEMID;
	#endif	// !defined(MEMID)

	// os2にはPSZがある.
	typedef PSZ PSTR, LPSTR;
	typedef int (*FARPROC)(void);	// farproc
	//os2にはUSHORT，ULONGがある.
	typedef USHORT WORD;
	typedef ULONG DWORD;
	// OS/2にはSHANDLEとLHANDLEがある.
	typedef LHANDLE HANDLE;	// 汎用ハンドルを作っておいて……
	typedef HANDLE GLOBALHANDLE;
	typedef HANDLE*PHANDLE, *LPHANDLE;

#else
	// in case of JRE_BUILDTYPE_WIN32 or JRE_BUILDTYPE_WIN16 (= JRE_BUILDTYPE_WINDOWS)
	// 以下，jre/winローカルの定義
	#if !defined(EXTAPI)
		#if defined(_MSC_VER)
			#define EXTAPI WINAPI	// VC2では_exportが通らないので注意.
		#endif
		#if defined(__BORLANDC__)
			#if defined(_TEST_EXE)
				#define EXTAPI WINAPI
			#else
				#define EXTAPI _export WINAPI
			#endif
		#endif
	#endif	// !defined(EXTAPI)

	#if !defined(MEMID)
		#if defined(JRE_BUILDTYPE_WIN32)
			typedef void*MEMID;	// 明示的にポインタにしました.
		#else
			typedef HGLOBAL MEMID;
		#endif	// __WIN32__
	#endif	// !defined(MEMID)

#endif

// --------------------------------------------------- グローバル変数



// ------------------------------------------------------------- 定数
// _JRE_ERR_CODEと_JRE_WARN_CODEのシンボルは山田がデバッグ時に使用しています.

#define _JRE_ERR_CODE
// エラーコード.nErrorが1～63まで. int型
#define		CantAllocate	( 1)	//	メモリの確保ができない.絶対的なメモリ不足.
#define		MemoryTooFew1	( 2)	//	メモリが足りない1.(実行時に決まるもの.メモリサイズの予測に失敗した)
#define		MemoryTooFew2	( 3)	//  メモリが足りない2.(DLL作成時に決まるもの)
#define		ReTooLong		(10)	//	正規表現が長すぎる.
#define		TooComplex		(13)	//	もっと簡単な表現にして下さい.このDLLの弱いパターンで,見つけられない.ただし滅多な事では発生しない.
#define		MismatchBracket	(20)	//	括弧の対応が正しくない.
#define		InvalidChClass	(21)	//	キャラクタクラスの内容が解釈できない.殆どの場合範囲指定が正しくない.
#define		EscErr			(24)	//	エスケープシークェンスが解釈できない.
#define		Unknown			(31)	//  なんだか良くわからないエラー.複合的な要因で発生するエラー.殆どがメモリサイズの予測に失敗.
#define		NoReString		(32)	//  検索パターンを指定して下さい.正規表現長が0バイト.
#define		IncorrectUsing	(33)	//  パラメータがおかしい.DLLの不正な使用法.
#define		ReNotExist		(34)	//	検索する前にコンパイルして下さい.
#define		InternalErr		(35)	//  DLLのバグを検出した.これが発生したら作者に連絡して欲しい.
#define		UsrAbort		(36)	//  ユーザー(アプリケーション)による中断.
#define		OldVersion		(37)	//	古い(解釈できない)バージョンの呼び出し手順を使用した.
// CantAllocate, TooComplex, MemoryTooFew2, ReNotExist, IncorrectUsing, UsrAbortは検索時にも発生するもの

#define _JRE_WARN_CODE
// 警告コード. レポートコード. jre2構造体のnWarningメンバ.
// これはそれぞれのビットにマッピングする可能性が高いので，
// if (CwInlinePattern | jre2.nWarning){
// }
// で評価してください.
#define		CwInlinePattern	(2)	// 行内の処理につき，再呼び出しの必要はない.

#define JGC_SHORT (1)
#define JGC_LONG (2)

// jre.dllで使用するメッセージの最大長(保証).このサイズのバッファに収まらない文字列は渡しません(nullまで含めてこのサイズ).
#define JRE_MAXLEN (128)

// GetJreMessageの言語番号.
#define GJM_JPN (0)
#define GJM_ENG (1)

// ----------------------------------------------------------- 構造体
// JRE構造体. バージョンにかかわらずjre構造体のアラインメントは8bit(1byte)です。
#pragma pack(1)
// JRE構造体は極力使用しないでください.今後のﾊﾞｰｼﾞｮﾝで廃止します(バージョン2.xxまででｻﾎﾟｰﾄを止めます).
// これに代わる構造体はJRE2構造体です.
typedef struct tagJRE{
	BOOL bConv;						// この構造体の使用中を示すフラグ.
	int nStart;						// 検索開始位置.バイト数.先頭は0.
	int nWarning;					// ウォーニングコード.
	int nError;						// エラーコード.
	int nLength;					// マッチ長.バイト数.
	int nPosition;					// マッチ位置.先頭は0.(お恥ずかしい)
	WORD wTranslate;				// 変換テーブル番号.
	LPSTR lpszTable;				// 変換テーブル.
	FARPROC lpfnUsrFunc;			// コールバック関数へのFARポインタ.
	int nCompData1;					// コンパイルデータ1.
	MEMID hCompData2;		// コンパイルデータ2.ハンドル.
	MEMID hCompData3;		// コンパイルデータ3.ハンドル.
	MEMID hCompData4;		// コンパイルデータ4.ハンドル.
} JRE, *PJRE, NEAR*NPJRE, FAR*LPJRE;
#pragma pack()


#pragma pack(1)
// バージョン1.06以降はこの構造体を使用してください.
// ただし,バージョン1.xxでは仕様が流動的です(どないせーちゅうねん).
typedef struct tagJRE2{
	DWORD dwSize;					// この構造体のサイズ.
	BOOL bConv;						// この構造体の使用中を示すフラグ.
	UINT nStart;					// 検索開始位置.バイト数.先頭は0.
	UINT nWarning;					// ウォーニングコード. (v1.11からUINTに変更)
	int nError;						// エラーコード.
	UINT nLength;					// マッチ長.バイト数.
	UINT nPosition;					// マッチ位置.先頭は0.(お恥ずかしい)
	WORD wTranslate;				// 変換テーブル番号.
	LPSTR lpszTable;				// 変換テーブル.
	FARPROC lpfnUsrFunc;			// コールバック関数へのFARポインタ(使わないように).
	UINT nCompData1;					// コンパイルデータ1.
	MEMID hCompData2;		// コンパイルデータ2.ハンドル.
	MEMID hCompData3;		// コンパイルデータ3.ハンドル.
	MEMID hCompData4;		// コンパイルデータ4.ハンドル.
} JRE2, *PJRE2, NEAR*NPJRE2, FAR*LPJRE2;
#pragma pack()

// ------------------------------------------------ ﾌﾟﾛﾄﾀｲﾌﾟおよびﾏｸﾛ
#ifdef __cplusplus	// caution! it's NOT cpulspuls!!!
	extern "C"{
#endif	// __cplusplus

// バージョンに関係ないAPI
BOOL EXTAPI IsMatch(LPSTR lpszStr, LPSTR lpszRe);
int EXTAPI GlobalReplace(LPSTR lpszRe, LPSTR lpszObj, LPSTR lpszStr, LPHANDLE lphGMemTo);
WORD EXTAPI JreGetVersion(void);
#if defined(JRE_BUILDTYPE_WIN16)
	MEMID EXTAPI DecodeEscSeqAlloc2(LPSTR lpszRe);	// 16bit Windowsの遺物.
#endif	// defined(JRE_BUILDTYPE_WIN16)
UINT EXTAPI DecodeEscSeq(LPSTR lpszRe, LPSTR lpszBuff, UINT uiSize);	// new!
int EXTAPI GetJreMessage(int nMessageType, int nLanguage, LPSTR lpszBuff, int cbBuff);
// バージョン1API
BOOL EXTAPI JreOpen(LPJRE lpjreJre);
BOOL EXTAPI JreCompile(LPJRE lpjreJre, LPSTR lpszRe);
BOOL EXTAPI JreGetMatchInfo(LPJRE lpjreJre, LPSTR lpszStr);
BOOL EXTAPI JreClose(LPJRE lpjreJre);
// バージョン2API
BOOL EXTAPI Jre2Open(LPJRE2 lpjreJre);
BOOL EXTAPI Jre2Compile(LPJRE2 lpjreJre, LPSTR lpszRe);
BOOL EXTAPI Jre2GetMatchInfo(LPJRE2 lpjreJre, LPSTR lpszStr);
BOOL EXTAPI Jre2Close(LPJRE2 lpjreJre);


#ifdef __cplusplus
	}
#endif	// __cplusplus

// ----------------------------------------------------------- 型定義
// GetProcAddressを使用する際のポインタ変数を宣言しやすくしておく.
typedef BOOL (EXTAPI*LPISMATCH) (LPSTR, LPSTR);
typedef int (EXTAPI*LPGLOBALREPLACE) (LPSTR, LPSTR, LPSTR, LPHANDLE);
typedef WORD (EXTAPI*LPJREGETVERSION) (VOID);
#if defined(JRE_BUILDTYPE_WIN16)
	typedef MEMID (EXTAPI*LPDECODEESCSEQALLOC2) (LPSTR);
#endif	// defined(JRE_BUILDTYPE_WIN16)
typedef UINT (EXTAPI*LPDECODEESCSEQ) (LPSTR lpszRe, LPSTR lpszBuff, UINT uiSize);
typedef int (EXTAPI*LPGETJREMESSAGE) (int, int, LPSTR, int);
typedef BOOL (EXTAPI*LPJRE2OPEN) (LPJRE2);
typedef BOOL (EXTAPI*LPJRE2COMPILE) (LPJRE2, LPSTR);
typedef BOOL (EXTAPI*LPJRE2GETMATCHINFO) (LPJRE2, LPSTR);
typedef BOOL (EXTAPI*LPJRE2CLOSE) (LPJRE2);




#endif	// JREUSR
// end of jreusr.h

/*[EOF]*/
