#include "stdafx.h"
#include "CCodeMediator.h"
#include "charcode.h"
#include "CESI.h"
#include "io/CBinaryStream.h"

/*
|| ファイルの日本語コードセット判別
||
|| 【戻り値】2007.08.14 kobake 戻り値をintからECodeTypeへ変更
||	SJIS		CODE_SJIS
||	JIS			CODE_JIS
||	EUC			CODE_EUC
||	Unicode		CODE_UNICODE
||	UTF-8		CODE_UTF8
||	UTF-7		CODE_UTF7
||	UnicodeBE	CODE_UNICODEBE
||	エラー		CODE_ERROR
*/
ECodeType CCodeMediator::CheckKanjiCodeOfFile( const TCHAR* pszFile )
{
	// オープン
	CBinaryInputStream in(pszFile);
	if(!in){
		return CODE_ERROR;
	}

	// データ長取得
	int nBufLen = in.GetLength();
	if( nBufLen > CheckKanjiCode_MAXREADLENGTH ){
		nBufLen = CheckKanjiCode_MAXREADLENGTH;
	}

	// 0バイトならSJIS扱い
	if( 0 == nBufLen ){
		return CODE_SJIS;
	}

	// データ確保
	CMemory cMem;
	cMem.AllocBuffer(nBufLen);
	void* pBuf = cMem.GetRawPtr();

	// 読み込み
	nBufLen = in.Read(pBuf, nBufLen);

	// クローズ
	in.Close();

	// 日本語コードセット判別
	ECodeType nCodeType = Charcode::DetectUnicodeBom( reinterpret_cast<const char*>(pBuf), nBufLen );
	if( nCodeType == CODE_NONE ){
		// Unicode BOM は検出されませんでした．
		nCodeType = CheckKanjiCode( reinterpret_cast<const uchar_t*>(pBuf), nBufLen );
	}

	return nCodeType;
}


/*
|| 日本語コードセット判別
||
|| 【戻り値】2007.08.14 kobake 戻り値をintからECodeTypeへ変更
||	SJIS		CODE_SJIS
||	JIS			CODE_JIS
||	EUC			CODE_EUC
||	Unicode		CODE_UNICODE
||	UTF-8		CODE_UTF8
||	UTF-7		CODE_UTF7
||	UnicodeBE	CODE_UNICODEBE
*/

// 2006.12.16  rastiv   アルゴリズムを改定．
ECodeType CCodeMediator::CheckKanjiCode( const unsigned char* pBuf, int nBufLen )
{
	CESI cesi;
	WCCODE_INFO wci;
	MBCODE_INFO mbci;
	int nPt;	// 
	
	if( !cesi.ScanEncoding(reinterpret_cast<const char*>(pBuf), nBufLen) ){
		// スキャンに失敗しました．
		return CODE_SJIS;  // ←デフォルト文字コードを返却．
	}
	
	nPt = cesi.DetectUnicode( &wci );
	if( 0 != nPt ){
		// UNICODE が検出されました.
		return wci.eCodeID;
	}
	nPt = cesi.DetectMultibyte( &mbci );
	//nPt := 特有バイト数 － 不正バイト数
	if( 0 < nPt ){
		return mbci.eCodeID;
	}

	return CODE_SJIS;  // ←デフォルト文字コードを返却．
}
