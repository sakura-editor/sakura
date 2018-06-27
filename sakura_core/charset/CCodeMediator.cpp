#include "StdAfx.h"
#include "charset/CCodeMediator.h"
#include "charset/charcode.h"
#include "charset/CESI.h"
#include "io/CBinaryStream.h"
#include "types/CType.h"



/*!
	文字列の先頭にUnicode系BOMが付いているか？

	@retval CODE_UNICODE   UTF-16 LE
	@retval CODE_UTF8      UTF-8
	@retval CODE_UNICODEBE UTF-16 BE
	@retval CODE_NONE      未検出

	@date 2007.08.11 charcode.cpp から移動
*/
ECodeType CCodeMediator::DetectUnicodeBom( const char* pS, const int nLen )
{
	uchar_t *pBuf;

	if( NULL == pS ){ return CODE_NONE; }

	pBuf = (uchar_t *) pS;
	if( 2 <= nLen ){
		if( pBuf[0] == 0xff && pBuf[1] == 0xfe ){
			return CODE_UNICODE;
		}
		if( pBuf[0] == 0xfe && pBuf[1] == 0xff ){
			return CODE_UNICODEBE;
		}
		if( 3 <= nLen ){
			if( pBuf[0] == 0xef && pBuf[1] == 0xbb && pBuf[2] == 0xbf ){
				return CODE_UTF8;
			}
		}
	}
#if 0
// 2015.03.05 Moca UTF-7 BOMは無効に変更
// もしデータがASCII互換でUTF-7として正しければ、文字コード比較でUTF-7になるはず
	if( 4 <= nLen ){
		if( memcmp( pBuf, "+/v", 3 ) == 0
			&& ( pBuf[3] == '8' || pBuf[3] == '9' || pBuf[3] == '+' || pBuf[3] == '/' ) ){
			return CODE_UTF7;
		}
	}
#endif
	return CODE_NONE;
}




/*!
	SJIS, JIS, EUCJP, UTF-8, UTF-7 を判定 (改)

	@return SJIS, JIS, EUCJP, UTF-8, UTF-7 の何れかの ID を返す．

	@note 適切な検出が行われた場合は、m_dwStatus に CESI_MB_DETECTED フラグが格納される。
*/
ECodeType CCodeMediator::DetectMBCode( CESI* pcesi )
{
//	pcesi->m_dwStatus = ESI_NOINFORMATION;

	if( pcesi->GetDataLen() < (pcesi->m_apMbcInfo[0]->nSpecific - pcesi->m_apMbcInfo[0]->nPoints) * 2000 ){
		// 不正バイトの割合が、全体の 0.05% 未満であることを確認。
		// 全体の0.05%ほどの不正バイトは、無視する。
		pcesi->SetStatus( ESI_NODETECTED );
		return CODE_NONE;
	}
	if( pcesi->m_apMbcInfo[0]->nPoints <= 0 ){
		pcesi->SetStatus( ESI_NODETECTED );
		return CODE_NONE;
	}

	/*
		判定状況を確認
	*/
	pcesi->SetStatus( ESI_MBC_DETECTED );
	return pcesi->m_apMbcInfo[0]->eCodeID;
}



/*!
	UTF-16 LE/BE を判定.

	@retval CODE_UNICODE    UTF-16 LE が検出された
	@retval CODE_UNICODEBE  UTF-16 BE が検出された
	@retval 0               UTF-16 LE/BE ともに検出されなかった

*/
ECodeType CCodeMediator::DetectUnicode( CESI* pcesi )
{
//	pcesi->m_dwStatus = ESI_NOINFORMATION;

	EBOMType ebom_type = pcesi->GetBOMType();
	int ndatalen;
	int nlinebreak;

	if( ebom_type == ESI_BOMTYPE_UNKNOWN ){
		pcesi->SetStatus( ESI_NODETECTED );
		return CODE_NONE;
	}

	// 1行の平均桁数が200を超えている場合はUnicode未検出とする
	ndatalen = pcesi->GetDataLen();
	nlinebreak = pcesi->m_aWcInfo[ebom_type].nSpecific;  // 改行数を nlinebreakに取得
	if( static_cast<double>(ndatalen) / nlinebreak > 200 ){
		pcesi->SetStatus( ESI_NODETECTED );
		return CODE_NONE;
	}

	pcesi->SetStatus( ESI_WC_DETECTED );
	return pcesi->m_aWcInfo[ebom_type].eCodeID;
}




/*
	日本語コードセット判定
*/
ECodeType CCodeMediator::CheckKanjiCode( CESI* pcesi )
{
	ECodeType nret;

	/*
		判定状況は、
		DetectMBCode(), DetectUnicode() 内で
		cesi.m_dwStatus に記録する。
	*/

	if( pcesi == NULL ){
		return CODE_DEFAULT;
	}
	if( pcesi->GetMetaName() != CODE_NONE ){
		return pcesi->GetMetaName();
	}
	nret = DetectUnicode( pcesi );
	if( nret != CODE_NONE && pcesi->GetStatus() != ESI_NODETECTED ){
		return nret;
	}
	nret = DetectMBCode( pcesi );
	if( nret != CODE_NONE && pcesi->GetStatus() != ESI_NODETECTED ){
		return nret;
	}

	// デフォルト文字コードを返す
	return pcesi->m_pEncodingConfig->m_eDefaultCodetype;
}


/*
	日本語コードセット判別

	戻り値】2007.08.14 kobake 戻り値をintからECodeTypeへ変更
	SJIS		CODE_SJIS
	JIS			CODE_JIS
	EUC			CODE_EUC
	Unicode		CODE_UNICODE
	UTF-8		CODE_UTF8
	UTF-7		CODE_UTF7
	UnicodeBE	CODE_UNICODEBE
*/
ECodeType CCodeMediator::CheckKanjiCode( const char* pBuf, int nBufLen )
{
	CESI cesi(*m_pEncodingConfig);

	/*
		判定状況は、
		DetectMBCode(), DetectUnicode() 内で
		cesi.m_dwStatus に記録する。
	*/

	cesi.SetInformation( pBuf, nBufLen/*, CODE_SJIS*/ );
	return CheckKanjiCode( &cesi );
}



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

	// 0バイトならタイプ別のデフォルト設定
	if( 0 == nBufLen ){
		return m_pEncodingConfig->m_eDefaultCodetype;
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
	ECodeType nCodeType = DetectUnicodeBom( reinterpret_cast<const char*>(pBuf), nBufLen );
	if( nCodeType == CODE_NONE ){
		// Unicode BOM は検出されませんでした．
		nCodeType = CheckKanjiCode( reinterpret_cast<const char*>(pBuf), nBufLen );
	}

	return nCodeType;
}
