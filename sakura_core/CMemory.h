//	$Id$
/************************************************************************

	CMemory.h

    メモリバッファクラスCMemory
	Copyright (C) 1998-2000, Norio Nakatani

    UPDATE:
    CREATE: 1998/3/6  新規作成


************************************************************************/
//#include "CBuffer.h"
class CMemory;

#ifndef _CMEMORY_H_
#define _CMEMORY_H_

/* ファイル文字コードセット判別時の先読み最大サイズ */
#define CheckKanjiCode_MAXREADLENGTH 16384

#include "global.h"
#include "debug.h"
/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
class SAKURA_CORE_API CMemory
{
public:
	/*
	||  Constructors
	*/
	CMemory();
	CMemory( const char*, int );
	~CMemory();

    /*
    || 関数
    */
//  int GetLength() const;
	int GetLength() const { return m_nDataLen; }
	void AllocBuffer( int );	/* バッファサイズの調整 */
	void SetData( const char*, int );	/* バッファの内容を置き換える */
	void SetDataSz( const char* );	/* バッファの内容を置き換える */
	void SetData( CMemory* );	/* バッファの内容を置き換える */
	const char* Append( const char* pData, int nDataLen );/* バッファの最後にデータを追加する（publicメンバ）*/
	void AppendSz( const char* pszData );/* バッファの最後にデータを追加する（publicメンバ）*/
	void Append( CMemory* );/* バッファの最後にデータを追加する（publicメンバ）*/
//	void InsertTop( const char*, int );	/* バッファの先頭にデータを挿入する */

	static int IsEqual( CMemory&, CMemory& );	/* 等しい内容か */

	/*
	|| 変換関数
	*/
	void Replace( char*, char* );	/* 文字列置換 */
	void ToLower( void );	/* 英大文字→英小文字 */
	void ToUpper( void );	/* 英小文字→英大文字 */

	void AUTOToSJIS( void );	/* 自動判別→SJISコード変換 */
	void SJIStoJIS( void );		/* SJIS→JISコード変換 */
	void JIStoSJIS( bool base64decode = false);		/* E-Mail(JIS→SJIS)コード変換 */
	void SJISToUnicode( void );	/* SJIS→Unicodeコード変換 */
	void SJISToEUC( void );		/* SJIS→EUCコード変換 */
	void EUCToSJIS( void );		/* EUC→SJISコード変換 */
	void UnicodeToSJIS( void );	/* Unicode→SJISコード変換 */
	void UTF8ToSJIS( void );	/* UTF-8→SJISコード変換 */
	void UTF7ToSJIS( void );	/* UTF-7→SJISコード変換 */
	void SJISToUTF8( void );	/* SJIS→UTF-8コード変換 */
	void SJISToUTF7( void );	/* SJIS→UTF-7コード変換  */
	void UnicodeToUTF8( void );	/* Unicode→UTF-8コード変換 */
	void UnicodeToUTF7( void );	/* Unicode→UTF-7コード変換 */
	void TABToSPACE( int );	/* TAB→空白 */

	void BASE64Decode( void );	// Base64デコード
	void UUDECODE( char* );		/* uudecode(デコード) */
//	static const char* MemCharNext( const char*, int, const char* );	/* ポインタで示した文字の次にある文字の位置を返します */
//	static const char* MemCharNext( /*const char*, int,*/ const char* );	/* ポインタで示した文字の次にある文字の位置を返します */
	static const char* MemCharNext( const char*, int, const char* );	/* ポインタで示した文字の次にある文字の位置を返します */
	static const char* MemCharPrev( const char*, int, const char* );	/* ポインタで示した文字の直前にある文字の位置を返します */
	static int MemSJISToUnicode( char**, const char*, int );	/* ASCII&SJIS文字列をUnicode に変換 */
	static int MemUnicodeToSJIS( char**, const char*, int );	/* Unicode文字列をASCII&SJIS に変換 */
	static int DecodeUTF8toUnicode( const unsigned char*, int, unsigned char* );
	static int IsUTF8( const unsigned char*, int ); /* UTF-8の文字か */
	void ToZenkaku( int, int );	/* 半角→全角 */
	void ToHankaku( void );	/* 全角→半角 */

	/* ファイルの日本語コードセット判別 */
	static int CheckKanjiCodeOfFile( const char* );
	/* 日本語コードセット判別 */
	static int CheckKanjiCode( const unsigned char*, int );
	/* 日本語コードセット判別:　EUCか？ */
	static int CheckKanjiCode_EUC( const unsigned char*, int, int*, int* );
	/* 日本語コードセット判別:　SJISか？ */
	static int CheckKanjiCode_SJIS( const unsigned char*, int, int*, int* );
	/* 日本語コードセット判別:　Unicodeか？ */
	static int CheckKanjiCode_UNICODE( const unsigned char*, int, int*, int* );
	/* 日本語コードセット判別:　JISか？ */
	static int CMemory::CheckKanjiCode_JIS( const unsigned char*, int, int*, int* );
	/* 日本語コードセット判別:　UTF-8Sか？ */
	static int CheckKanjiCode_UTF8( const unsigned char*, int, int*, int* );
	/* 日本語コードセット判別:　UTF-7Sか？ */
	static int CheckKanjiCode_UTF7( const unsigned char*, int, int*, int* );
	static int IsZenHiraOrKata( unsigned short );

	/*
	|| 演算子
	*/
	const CMemory& operator=( char );
//	const CMemory& operator=( const char* );
	const CMemory& operator=( const CMemory& );
//	const CMemory& operator+=( const char* );
	const CMemory& operator+=( const CMemory& );
	const CMemory& operator+=( char );
//	operator char*() const;
//	operator const char*() const;
//	operator unsigned char*() const;
//	operator const unsigned char*() const;
//	operator void*() const;
//	operator const void*() const;
	const char operator[](int nIndex) const;

//	char* GetPtr( int* ) const;	/* データへのポインタと有効長を返す */
	/* データへのポインタと長さ返す */
	__forceinline char* GetPtr( int* pnLength ) const
	{
		if( NULL != pnLength ){
			*pnLength = GetLength();
		}
		return (char*)m_pData;
	}

	__forceinline char* GetPtr2( void ) const
	{
		return (char*)m_pData;
	}

	
//	void Append( const char*, int );	/* データの最後に追加 publicメンバ */
//protected:
	/*
	|| メンバ変数
	*/
	int		m_nDataBufSize;
	char*	m_pData;
	int		m_nDataLen;

	/*
	||  実装ヘルパ関数
	*/
//	void Init( void );
	void Empty( void );
	void AddData( const char*, int );
	static int IsEUCKan1(unsigned char );	/* EUC全角コードの１バイト目か */
	static int IsEUCKan2(unsigned char );	/*  EUC全角コードの２バイト目か */
	static long MemBASE64_Decode( unsigned char*, long );	/* Base64デコード */
	int MemBASE64_Encode( const char*, int, char**, int, int );/* Base64エンコード */
	long QuotedPrintable_Decode(char*, long );	/* Quoted-Printableデコード	*/
	long MemJIStoSJIS(unsigned char*, long );	/* JIS→SJIS変換 */
	int StrSJIStoJIS( CMemory*, unsigned char*, int );	/* SJIS→JISで新メモリ確保　*/
	long MemSJIStoJIS( unsigned char*, long );	/* SJIS→JIS変換 */
	static int IsBASE64Char( char );	/* 文字がBaseE64のデータか */


};


///////////////////////////////////////////////////////////////////////
#endif /* _CMEMORY_H_ */

/*[EOF]*/
