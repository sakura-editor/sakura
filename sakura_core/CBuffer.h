//	$Id$
/************************************************************************
	CBuffer.h
    メモリバッファクラス
	Copyright (C) 1998-2000, Norio Nakatani
************************************************************************/
class CBuffer;

#ifndef _CBUFFER_H_
#define _CBUFFER_H_

class CBuffer
{
public:
	/*
	||  Constructors
	*/
	CBuffer();
	CBuffer( const char* , int  );
	~CBuffer();

    /*
    || 関数
    */
    int GetLength() const;
	void SetData( const char*, int );	/* バッファの内容を置き換える */
	void Append( const char* pData, int nDataLen );
	static int IsEqual( CBuffer&, CBuffer& );	/* 等しい内容か */

	/*
	|| 変換関数
	*/
//	void BASE64Decode( void );	// Base64デコード
//	void UUDECODE( char* );	/* uudecode(デコード) */
	void Replace( char*, char* );	/* 文字列置換 */
//	void ToLower( void );	/* 英大文字→英小文字 */
//	void ToUpper( void );	/* 英小文字→英大文字 */
//	void SJIStoJIS( void );		/* コード変換 　SJIS→JIS　*/
//	void JIStoSJIS( void );		/* コード変換 　JIS→SJIS　*/
//	void SJISToUnicode( void );	/* コード変換   SJIS→Unicode */
//	void UnicodeToSJIS( void );	/* コード変換   Unicode→SJIS */
//	void SJISToEUC( void );		/* コード変換   SJIS→EUC */
//	void EUCToSJIS( void );		/* コード変換   EUC→SJIS */
	static const char* MemCharNext( const char*, int, const char* );	/* ポインタで示した文字の次にある文字の位置を返します */
	static const char* MemCharPrev( const char*, int, const char* );	/* ポインタで示した文字の直前にある文字の位置を返します */
//	static int MemSJISToUnicode( char** , const char*, int );	/* ASCII&SJIS文字列をUnicodeに変換 */
//	static int MemUnicodeToSJIS( char** , const char*, int );	/* Unicode文字列をASCII&SJISに変換 */

//	void ToZenkaku( int, int );	/* 半角→全角 */
//	void ToHankaku( void );	/* 全角→半角 */

//	/* ファイルの日本語コードセット判別 */
//	static int CheckKanjiCode( const char* );
//	/* ファイルの日本語コードセット判別:　EUCか？ */
//	static int CheckKanjiCode_EUC( const char* , int*, int* );
//	/* ファイルの日本語コードセット判別:　SJISか？ */
//	static int CheckKanjiCode_SJIS( const char* , int* , int* );
//	/* ファイルの日本語コードセット判別:　Unicodeか？ */
//	static CBuffer::CheckKanjiCode_UNICODE( const char*, int*, int* );
//	/* ファイルの日本語コードセット判別:　JISか？ */
//	static int CBuffer::CheckKanjiCode_JIS( const char*, int*, int* );
							
	/*
	|| 演算子
	*/
	const CBuffer& operator=( char );
//	const CBuffer& operator=( const char* );
	const CBuffer& operator=( const CBuffer& );
//	const CBuffer& operator+=( const char* );
	const CBuffer& operator+=( const CBuffer& );
	const CBuffer& operator+=( char );
//	operator char*() const;
//	operator const char*() const;
//	operator unsigned char*() const;
//	operator const unsigned char*() const;
//	operator void*() const;
//	operator const void*() const;
	const char operator[](int nIndex) const;
	char* GetPtr( int* ) const;	/* データへのポインタと有効長を返す */
//	void Append( const char* , int );	/* データの最後に追加 publicメンバ */
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
	void Init( void );
	void Empty( void );
	void AllocBuffer( int );
	void AddData( const char*, int );
//	static int IsEUCKan1(unsigned char );	/* EUC全角コードの１バイト目か */
//	static int IsEUCKan2(unsigned char );	/*  EUC全角コードの２バイト目か */
//	long MemBASE64_Decode(char* , long );	/* Base64デコード */
//	long QuotedPrintable_Decode(char* , long );	/* Quoted-Printableデコード	*/
//	long MemJIStoSJIS(unsigned char* , long );	/* JIS→SJIS変換 */
//	int StrSJIStoJIS( CBuffer*, unsigned char*, int );	/* SJIS→JISで新メモリ確保　*/
//	long MemSJIStoJIS( unsigned char* , long );	/* SJIS→JIS変換 */
//	int IsBASE64Char( char );	/* 文字がBase64のデータか */

};


///////////////////////////////////////////////////////////////////////
#endif /* _CBUFFER_H_ */

/*[EOF]*/
