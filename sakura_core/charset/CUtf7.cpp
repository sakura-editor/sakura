#include "stdafx.h"
#include "CUtf7.h"
#include "charcode.h"

//! UTF-7→Unicodeコード変換
// 2007.08.13 kobake 作成
EConvertResult CUtf7::UTF7ToUnicode( CMemory* pMem )
{
	//データ取得
	int nDataLen;
	const char* pData=(char*)pMem->GetRawPtr(&nDataLen);

	//必要なバッファサイズを調べる
	size_t dstlen=MultiByteToWideChar(
		CP_UTF7,
		0,
		pData,
		nDataLen,
		NULL,
		0
	);

	//バッファ確保
	std::vector<wchar_t> dst(dstlen+1);

	//変換
	int ret=MultiByteToWideChar(
		CP_UTF7,
		0,
		pData,
		nDataLen,
		&dst[0],
		dstlen
	);
	dst[dstlen]=L'\0';

	//設定
	const void* p=&dst[0];
	pMem->SetRawData(p,dstlen*sizeof(wchar_t));

	return RESULT_COMPLETE;
}


/*! コード変換 Unicode→UTF-7
	@date 2002.10.25 Moca UTF-7で直接エンコードできる文字をRFCに合わせて制限した
*/
EConvertResult CUtf7::UnicodeToUTF7( CMemory* pMem )
{
	//データ取得
	int nDataLen;
	const wchar_t* pData=(wchar_t*)pMem->GetRawPtr(&nDataLen);
	nDataLen/=sizeof(wchar_t);

	//出力先
	wchar_t*		pUniBuf;
	int				nUniBufLen = nDataLen / sizeof(wchar_t);
	pUniBuf = new wchar_t[nUniBufLen + 1];

	int				i;
	int				j;
	unsigned char*	pDes;
	int				k;
	BOOL			bBASE64;
	int				nBgn;
	char*			pszBase64Buf;
	int				nBase64BufLen;
	char*			pszWork;
	char			cWork;

//	setlocale( LC_ALL, "Japanese" ); // wctomb を使わなくなったためコメントアウト
	k = 0;
	bBASE64 = FALSE;
	nBgn = 0;
//	memset( pUniBuf, 0, (nUniBufLen + 1) * sizeof( wchar_t ) );
	memcpy( pUniBuf, pData, nUniBufLen * sizeof( wchar_t ) );
	pUniBuf[nUniBufLen] = L'\0';
	for( i = 0; i < nUniBufLen; ++i ){
		j = IsUTF7Direct( pUniBuf[i]);
		if( !bBASE64 ){
			if( 1 == j ){
				k++;
			}else
			if( L'+' == pUniBuf[i] ){
				k += 2;
			}else{
				bBASE64 = TRUE;
				nBgn = i;
			}
		}else{
			if( 1 == j ){
				/* 2バイトのUnicodeがあるという前提でLO/HIバイトを交換 */
				pszWork = (char*)(char*)&pUniBuf[nBgn];
				for( j = 0; j < (int)((i - nBgn) * sizeof( wchar_t )); j += 2 ){
					cWork = pszWork[j + 1];
					pszWork[j + 1] = pszWork[j];
					pszWork[j] = cWork;
				}
				/* Base64エンコード */
				pszBase64Buf = NULL;
				nBase64BufLen = 0;
				nBase64BufLen = MemBASE64_Encode(
					(char*)&pUniBuf[nBgn],			// エンコード対象データ
					(i - nBgn) * sizeof( wchar_t ), // エンコード対象データ長
					&pszBase64Buf,					// 結果データ格納メモリポインタのアドレス
					-1,		// エンコード後のデータを自動的にCRLFで折り返す場合の１行最大文字数 (-1が指定された場合は折り返さない)
					FALSE	// パディングするか
				);
				//////////////
				k++;
				k += nBase64BufLen;
				k++;
				//////////////
				delete [] pszBase64Buf;
				pszBase64Buf = NULL;
				nBase64BufLen = 0;
				bBASE64 = FALSE;
				i--;
			}else{
			}
		}
	}
	if( bBASE64 && 0 < (i - nBgn) ){
		/* 2バイトのUnicodeがあるという前提でLO/HIバイトを交換 */
		pszWork = (char*)(char*)&pUniBuf[nBgn];
		for( j = 0; j < (int)((i - nBgn) * sizeof( wchar_t )); j += 2 ){
			cWork = pszWork[j + 1];
			pszWork[j + 1] = pszWork[j];
			pszWork[j] = cWork;
		}
		/* Base64エンコード */
		pszBase64Buf = NULL;
		nBase64BufLen = 0;
		nBase64BufLen = MemBASE64_Encode(
			(char*)&pUniBuf[nBgn],			// エンコード対象データ
			(i - nBgn) * sizeof( wchar_t ), // エンコード対象データ長
			&pszBase64Buf,					// 結果データ格納メモリポインタのアドレス
			-1,		// エンコード後のデータを自動的にCRLFで折り返す場合の１行最大文字数 (-1が指定された場合は折り返さない)
			FALSE	// パディングするか
		);
		//////////////
		k++;
		k += nBase64BufLen;
		k++;
		//////////////
		delete [] pszBase64Buf;
		pszBase64Buf = NULL;
		nBase64BufLen = 0;
		bBASE64 = FALSE;
	}
	delete [] pUniBuf;

	pDes = new unsigned char[k + 1];
	memset( pDes, 0, k + 1 );
	k = 0;
	bBASE64 = FALSE;
	nBgn = 0;
	pUniBuf = (wchar_t*)pData;
	for( i = 0; i < nUniBufLen; ++i ){
		j = IsUTF7Direct( pUniBuf[i] );
		if( !bBASE64 ){
			if( 1 == j ){
				pDes[k] = (unsigned char)(pUniBuf[i] & 0x007f);
				k++;
			}else
			if( L'+' == pUniBuf[i] ){
				pDes[k    ] = '+';
				pDes[k + 1] = '-';
				k += 2;
			}else{
				bBASE64 = TRUE;
				nBgn = i;
			}
		}else{
			if( 1 == j ){
				/* 2バイトのUnicodeがあるという前提でLO/HIバイトを交換 */
				pszWork = (char*)(char*)&pUniBuf[nBgn];
				for( j = 0; j < (int)((i - nBgn) * sizeof( wchar_t )); j += 2 ){
					cWork = pszWork[j + 1];
					pszWork[j + 1] = pszWork[j];
					pszWork[j] = cWork;
				}
				char*	pszBase64Buf;
				int		nBase64BufLen;
				/* Base64エンコード */
				nBase64BufLen = MemBASE64_Encode(
					(char*)&pUniBuf[nBgn],			// エンコード対象データ
					(i - nBgn) * sizeof( wchar_t ), // エンコード対象データ長
					&pszBase64Buf,					// 結果データ格納メモリポインタのアドレス
					-1,		// エンコード後のデータを自動的にCRLFで折り返す場合の１行最大文字数 (-1が指定された場合は折り返さない)
					FALSE	// パディングするか
				);
				//////////////
				pDes[k] = '+';
				k++;
				memcpy( &pDes[k], pszBase64Buf, nBase64BufLen );
				k += nBase64BufLen;
				pDes[k] = '-';
				k++;
				//////////////
				delete [] pszBase64Buf;
				bBASE64 = FALSE;
				i--;
			}else{
			}
		}
	}
	if( bBASE64 && 0 < (i - nBgn) ){
		/* 2バイトのUnicodeがあるという前提でLO/HIバイトを交換 */
		pszWork = (char*)(char*)&pUniBuf[nBgn];
		for( j = 0; j < (int)((i - nBgn) * sizeof( wchar_t )); j += 2 ){
			cWork = pszWork[j + 1];
			pszWork[j + 1] = pszWork[j];
			pszWork[j] = cWork;
		}
		/* Base64エンコード */
		pszBase64Buf = NULL;
		nBase64BufLen = 0;
		nBase64BufLen = MemBASE64_Encode(
			(char*)&pUniBuf[nBgn],			// エンコード対象データ
			(i - nBgn) * sizeof( wchar_t ), // エンコード対象データ長
			&pszBase64Buf,					// 結果データ格納メモリポインタのアドレス
			-1,		// エンコード後のデータを自動的にCRLFで折り返す場合の１行最大文字数 (-1が指定された場合は折り返さない)
			FALSE	// パディングするか
		);
		//////////////
		pDes[k] = '+';
		k++;
		memcpy( &pDes[k], pszBase64Buf, nBase64BufLen );
		k += nBase64BufLen;
		pDes[k] = '-';
		k++;
		//////////////
		delete [] pszBase64Buf;
		pszBase64Buf = NULL;
		nBase64BufLen = 0;
		bBASE64 = FALSE;
	}
	pMem->SetRawData( pDes, k );
	delete [] pDes;

	return RESULT_COMPLETE;
}


/*!
	Unicodeの文字がUTF-7で直接エンコードできるか調べる
	@author Moca
	@date 2002.10.25 新規作成

	TAB SP CR LF は 直接エンコード可能
	基本セット
	         '(),-./012...789:?ABC...XYZabc...xyz
	以下はオプションでメールでは支障をきたす場合がある
	         !"#$%&*;<=>@[\]^_`{|}
	とりあえず無条件でオプションは直接変換できないと判断する
*/
int CUtf7::IsUTF7Direct( wchar_t wc )
{
	int nret = 0;
	if( (wc & 0xff00) == 0 ){
		nret = Charcode::IsUtf7SetDChar( (unsigned char)wc );
	}
	return nret;
}



/*
* Base64エンコード
*
*	符号化したデータは、新たに確保したメモリに格納されます
*	終了時に、そのメモリハンドルを指定されたアドレスに格納します
*	符号化されたデータ列は、一応NULL終端文字列になっています
*
*/
int CUtf7::MemBASE64_Encode(
	const char*	pszSrc		,	// エンコード対象データ
	int			nSrcLen		,	// エンコード対象データ長
	char**		ppszDes		,	// 結果データ格納メモリポインタのアドレス
	int			nWrap		,	// エンコード後のデータを自動的にCRLFで折り返す場合の１行最大文字数 (-1が指定された場合は折り返さない)
	int			bPadding		// パディングするか
)
{
	int			i, j, k, m, n;
	long		nDesIdx;
//	char*		pszDes;
	long		nDataDes;
	long		nDataSrc;
	long		nLineLen;
	char		cw;
	const char	szBASE64CODE[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	int			nBASE64CODE_Num = _countof( szBASE64CODE );
	int			nDesLen;

	// 符号化後の長さを算出（ヘッダー・フッターを除く）
	//=?ISO-2022-JP?B? GyRC QXc/ Lkgi JE4/ NiRq Siwk MRso Qg== ?=
	nDesLen = ((nSrcLen / 3) + ((nSrcLen % 3)? 1:0)) * 4;
	if( -1 != nWrap ){
		nDesLen += 2 * ( nDesLen / nWrap + ((nDesLen % nWrap)? 1:0 ));
	}

	(*ppszDes) = new char[nDesLen + 1];
	memset( (*ppszDes), 0, nDesLen + 1 );

	nDesIdx = 0;
	nLineLen = 0;
	for( i = 0; i < nSrcLen; i += 3 ){
		memcpy( &nDataDes, "====", 4 );
		nDataSrc = 0;
		if( nSrcLen - i < 3 ){
			k = (nSrcLen % 3) * 8 / 6 + 1;
			for( m = 0; m < nSrcLen % 3; m++ ){
				((char*)&nDataSrc)[3 - m] = pszSrc[i + m];
			}
		}else{
			k = 4;
			for( m = 0; m < 3; m++ ){
				((char*)&nDataSrc)[3 - m] = pszSrc[i + m];
			}
		}
		for( j = 0; j < k; j++ ){
			cw = (char)((nDataSrc >> (6 * (3 - j) + 8)) & 0x0000003f);
			((char*)&nDataDes)[j] = szBASE64CODE[(int)cw];
		}
		if( bPadding ){		// パディングするか
			k = 4;
		}else{
			nDesLen -= (4 - k);
		}
		for( n = 0; n < k; n++ ){
			// 自動折り返しの処理
			if( nWrap != -1 && nLineLen == nWrap ){
				(*ppszDes)[nDesIdx + 0] = ACODE::CR;
				(*ppszDes)[nDesIdx + 1] = ACODE::LF;
				nDesIdx += 2;
				nLineLen = 0;
			}
			(*ppszDes)[nDesIdx] = ((char*)&nDataDes)[n];
			nDesIdx++;
			nLineLen++;
		}
	}
//	GlobalUnlock( *phgDes );
	return nDesLen;
}
