//	$Id$
/*!	@file
	@brief 強調キーワード管理

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, MIK

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "CKeyWordSetMgr.h"
#include <stdlib.h>
#include <malloc.h>


CKeyWordSetMgr::CKeyWordSetMgr( void )
{
	m_nCurrentKeyWordSetIdx = 0;
	m_nKeyWordSetNum = 0;
	return;
}

CKeyWordSetMgr::~CKeyWordSetMgr( void)
{
	m_nKeyWordSetNum = 0;
	return;
}

const CKeyWordSetMgr& CKeyWordSetMgr::operator=( CKeyWordSetMgr& cKeyWordSetMgr )
{
//	int		nDataLen;
//	char*	pData;
//	int		i;
	if( this == &cKeyWordSetMgr ){
		return *this;
	}
	m_nCurrentKeyWordSetIdx = cKeyWordSetMgr.m_nCurrentKeyWordSetIdx;
	m_nKeyWordSetNum = cKeyWordSetMgr.m_nKeyWordSetNum;
	memcpy( m_szSetNameArr, cKeyWordSetMgr.m_szSetNameArr, sizeof( m_szSetNameArr ) );
	memcpy( m_nKEYWORDCASEArr, cKeyWordSetMgr.m_nKEYWORDCASEArr, sizeof( m_nKEYWORDCASEArr ) );
	memcpy( m_nKeyWordNumArr, cKeyWordSetMgr.m_nKeyWordNumArr, sizeof( m_nKeyWordNumArr ) );
	memcpy( m_szKeyWordArr, cKeyWordSetMgr.m_szKeyWordArr, sizeof( m_szKeyWordArr ) );
	memcpy( m_IsSorted, cKeyWordSetMgr.m_IsSorted, sizeof( m_IsSorted ) );	//MIK 2000.12.01 binary search

	return *this;
}




/* セットの追加 */
BOOL CKeyWordSetMgr::AddKeyWordSet( const char* pszSetName, BOOL nKEYWORDCASE )
{
	if( MAX_SETNUM <= m_nKeyWordSetNum ){
		return FALSE;
	}
	strcpy( m_szSetNameArr[m_nKeyWordSetNum], pszSetName );
	m_nKEYWORDCASEArr[m_nKeyWordSetNum] = nKEYWORDCASE;
	m_nKeyWordNumArr[m_nKeyWordSetNum] = 0;

	m_IsSorted[m_nKeyWordSetNum] = 0;	//MIK 2000.12.01 binary search
	m_nKeyWordSetNum++;
	return TRUE;
}

/* ｎ番目のセットを削除 */
BOOL CKeyWordSetMgr::DelKeyWordSet( int nIdx )
{
	int		i;
	if( m_nKeyWordSetNum <= nIdx ||
		0 > nIdx
	){
		return FALSE;
	}
	for( i = nIdx; i < m_nKeyWordSetNum - 1; ++i ){
		memcpy( m_szSetNameArr[i], m_szSetNameArr[i + 1], sizeof( m_szSetNameArr[0] ) );
		m_nKEYWORDCASEArr[i] = m_nKEYWORDCASEArr[i + 1];
		m_nKeyWordNumArr[i] = m_nKeyWordNumArr[i + 1];
		memcpy( m_szKeyWordArr[i], m_szKeyWordArr[i + 1], sizeof( m_szKeyWordArr[0] ) );
		m_IsSorted[i] = m_IsSorted[i+1];	//MIK 2000.12.01 binary search
	}
	m_nKeyWordSetNum--;
	if( m_nKeyWordSetNum <= m_nCurrentKeyWordSetIdx ){
		m_nCurrentKeyWordSetIdx = m_nKeyWordSetNum - 1;
//セットが無くなったとき、m_nCurrentKeyWordSetIdxをわざと-1にするため、コメント化
//		if( 0 > m_nCurrentKeyWordSetIdx ){
//			m_nCurrentKeyWordSetIdx = 0;
//		}
	}
	return TRUE;
}



/* ｎ番目のセットのセット名を返す */
char* CKeyWordSetMgr::GetTypeName( int nIdx )
{
	if( m_nKeyWordSetNum <= nIdx ){
		return NULL;
	}
	return m_szSetNameArr[nIdx];
}

/* ｎ番目のセットのキーワードの数を返す */
int CKeyWordSetMgr::GetKeyWordNum( int nIdx )
{
	if( m_nKeyWordSetNum <= nIdx ){
		return 0;
	}
	return m_nKeyWordNumArr[nIdx];
}

/* ｎ番目のセットのｍ番目のキーワードを返す */
char* CKeyWordSetMgr::GetKeyWord( int nIdx, int nIdx2 )
{
	if( m_nKeyWordSetNum <= nIdx ){
		return NULL;
	}
	if( m_nKeyWordNumArr[nIdx] <= nIdx2 ){
		return NULL;
	}
	return m_szKeyWordArr[nIdx][nIdx2];
}

/* ｎ番目のセットのｍ番目のキーワードを編集 */
char* CKeyWordSetMgr::UpdateKeyWord( int nIdx, int nIdx2, const char* pszKeyWord )
{
	int i;
	if( m_nKeyWordSetNum <= nIdx ){
		return NULL;
	}
	if( m_nKeyWordNumArr[nIdx] <= nIdx2 ){
		return NULL;
	}
	/* 0バイトの長さのキーワードは編集しない */
	if( 0 == strlen( pszKeyWord ) ){
		return NULL;
	}
	/* 重複したキーワードは編集しない */
	for( i = 0; i < m_nKeyWordNumArr[nIdx]; ++i ){
		if( 0 == strcmp( m_szKeyWordArr[nIdx][i], pszKeyWord ) ){
			return NULL;
		}
	}
	m_IsSorted[nIdx] = 0;	//MIK 2000.12.01 binary search
	return strcpy( m_szKeyWordArr[nIdx][nIdx2], pszKeyWord );
}

/* ｎ番目のセットにキーワードを追加 */
BOOL CKeyWordSetMgr::AddKeyWord( int nIdx, const char* pszKeyWord )
{
	int  i;
	if( m_nKeyWordSetNum <= nIdx ){
		return FALSE;
	}
	if( MAX_KEYWORDNUM <= m_nKeyWordNumArr[nIdx] ){
		return FALSE;
	}
	/* 0バイトの長さのキーワードは登録しない */
	if( 0 == strlen( pszKeyWord ) ){
		return FALSE;
	}
	/* 重複したキーワードは登録しない */
	for( i = 0; i < m_nKeyWordNumArr[nIdx]; ++i ){
		if( 0 == strcmp( m_szKeyWordArr[nIdx][i], pszKeyWord ) ){
			return FALSE;
		}
	}
	/* MAX_KEYWORDLENより長いキーワードは切り捨てる */
	if( MAX_KEYWORDLEN < strlen( pszKeyWord ) ){
		memcpy( m_szKeyWordArr[nIdx][m_nKeyWordNumArr[nIdx]], pszKeyWord, MAX_KEYWORDLEN );
		m_szKeyWordArr[nIdx][m_nKeyWordNumArr[nIdx]][MAX_KEYWORDLEN] = '\0';
	}else{
		strcpy( m_szKeyWordArr[nIdx][m_nKeyWordNumArr[nIdx]], pszKeyWord );
	}
	m_nKeyWordNumArr[nIdx]++;
	m_IsSorted[nIdx] = 0;	//MIK 2000.12.01 binary search
	return TRUE;
}


/* ｎ番目のセットのｍ番目のキーワードを削除 */
BOOL CKeyWordSetMgr::DelKeyWord( int nIdx, int nIdx2 )
{
	int		i;
	if( m_nKeyWordSetNum <= nIdx ){
		return FALSE;
	}
	if( m_nKeyWordNumArr[nIdx] <= nIdx2 ){
		return FALSE;
	}
	if( 0 >= m_nKeyWordNumArr[nIdx]	){
		return FALSE;
	}
	for( i = nIdx2; i < m_nKeyWordNumArr[nIdx] - 1; ++i ){
		strcpy( m_szKeyWordArr[nIdx][i], m_szKeyWordArr[nIdx][i + 1] );
	}
	m_nKeyWordNumArr[nIdx]--;
	m_IsSorted[nIdx] = 0;	//MIK 2000.12.01 binary search
	return TRUE;
}

///* ｎ番目のセットから指定キーワードをサーチ 無いときは-1を返す */
//int CKeyWordSetMgr::SearchKeyWord( int nIdx, const char* pszKeyWord )
//{
//	int i;
//	if( m_nKEYWORDCASEArr[nIdx] ){	/* キーワードの英大文字小文字区別 */
//		for( i = 0; i < m_nKeyWordNumArr[nIdx]; ++i ){
//			if( 0 == strcmp( m_szKeyWordArr[nIdx][i], pszKeyWord ) ){
//				return i;
//			}
//		}
//	}else{
//		for( i = 0; i < m_nKeyWordNumArr[nIdx]; ++i ){
//			if( 0 == stricmp( m_szKeyWordArr[nIdx][i], pszKeyWord ) ){
//				return i;
//			}
//		}
//	}
//	returb -1;
//}
/* ｎ番目のセットから指定キーワードをサーチ 無いときは-1を返す */
int CKeyWordSetMgr::SearchKeyWord( int nIdx, const char* pszKeyWord, int nKeyWordLen )
{
	int i;
	if( m_nKEYWORDCASEArr[nIdx] ){	/* キーワードの英大文字小文字区別 */
		for( i = 0; i < m_nKeyWordNumArr[nIdx]; ++i ){
			if( (int)strlen( m_szKeyWordArr[nIdx][i] ) == nKeyWordLen ){
				if( 0 == memcmp( m_szKeyWordArr[nIdx][i], pszKeyWord, nKeyWordLen ) ){
					return i;
				}
			}
		}
	}else{
		for( i = 0; i < m_nKeyWordNumArr[nIdx]; ++i ){
			if( (int)strlen( m_szKeyWordArr[nIdx][i] ) == nKeyWordLen ){
				if( 0 == memicmp( m_szKeyWordArr[nIdx][i], pszKeyWord, nKeyWordLen ) ){
					return i;
				}

			}
		}
	}
	return -1;

}

/* 変更状況を調査 */
BOOL CKeyWordSetMgr::IsModify( CKeyWordSetMgr& cKeyWordSetMgrNew, BOOL* pnModifyFlagArr )
{
	BOOL	bModifyAll;
	int		i;


	for( i = 0; i < MAX_SETNUM; ++i ){
		pnModifyFlagArr[i] = FALSE;
	}
	if( this == &cKeyWordSetMgrNew ){
		return FALSE;
	}

	bModifyAll = FALSE;
	if( m_nCurrentKeyWordSetIdx != cKeyWordSetMgrNew.m_nCurrentKeyWordSetIdx
	 || m_nKeyWordSetNum != cKeyWordSetMgrNew.m_nKeyWordSetNum
	 || 0 != memcmp( &m_szSetNameArr, &cKeyWordSetMgrNew.m_szSetNameArr, sizeof( m_szSetNameArr ) )
	 || 0 != memcmp( &m_nKEYWORDCASEArr, &cKeyWordSetMgrNew.m_nKEYWORDCASEArr, sizeof( m_nKEYWORDCASEArr ) )
	 || 0 != memcmp( &m_nKeyWordNumArr, &cKeyWordSetMgrNew.m_nKeyWordNumArr, sizeof( m_nKeyWordNumArr ) )
	 || 0 != memcmp( &m_szKeyWordArr, &cKeyWordSetMgrNew.m_szKeyWordArr, sizeof( m_szKeyWordArr ) )
	 ){
		bModifyAll = TRUE;
		for( i = 0; i < MAX_SETNUM; ++i ){
			if( 0 != memcmp( &m_szSetNameArr[i], &cKeyWordSetMgrNew.m_szSetNameArr[i], sizeof( m_szSetNameArr[i] ) )
			 || 0 != memcmp( &m_nKEYWORDCASEArr[i], &cKeyWordSetMgrNew.m_nKEYWORDCASEArr[i], sizeof( m_nKEYWORDCASEArr[i] ) )
			 || 0 != memcmp( &m_nKeyWordNumArr[i], &cKeyWordSetMgrNew.m_nKeyWordNumArr[i], sizeof( m_nKeyWordNumArr[i] ) )
			 || 0 != memcmp( &m_szKeyWordArr[i], &cKeyWordSetMgrNew.m_szKeyWordArr[i], sizeof( m_szKeyWordArr[i] ) )
			){
				pnModifyFlagArr[i] = TRUE;
			}
		}
	}
	return bModifyAll;

}

//MIK START 2000.12.01 binary search
//ソート
void CKeyWordSetMgr::SortKeyWord( int nIdx )
{
	//nIdxのセットをソートする。
	if( m_nKEYWORDCASEArr[nIdx] ) {
		qsort( &m_szKeyWordArr[nIdx],
				m_nKeyWordNumArr[nIdx],
				sizeof(m_szKeyWordArr[nIdx][0]),
				(int (__cdecl *)(const void *, const void *))strcmp
			);
	} else {
		qsort( &m_szKeyWordArr[nIdx],
				m_nKeyWordNumArr[nIdx],
				sizeof(m_szKeyWordArr[nIdx][0]),
				(int (__cdecl *)(const void *, const void *))stricmp
			);
	}

	m_IsSorted[nIdx] = 1;
	return;
}
/* ｎ番目のセットから指定キーワードをバイナリサーチ 無いときは-1を返す */
int CKeyWordSetMgr::SearchKeyWord2( int nIdx, const char* pszKeyWord, int nKeyWordLen )
{
	int pc, pr, pl, ret, wcase;

	//sort
	if(m_IsSorted[nIdx] == 0) SortKeyWord(nIdx);

	pl = 0;
	pr = m_nKeyWordNumArr[nIdx] - 1;
	if( pr < 0 ) return -1;
	pc = (pr + 1 - pl) / 2 + pl;
	wcase = m_nKEYWORDCASEArr[nIdx];
	while(pl <= pr) {
		if( wcase ) {
			ret = strncmp( pszKeyWord, m_szKeyWordArr[nIdx][pc], nKeyWordLen );
		} else {
			ret = strnicmp( pszKeyWord, m_szKeyWordArr[nIdx][pc], nKeyWordLen );
		}
		if( ret == 0 ) {
			if( (int)strlen( m_szKeyWordArr[nIdx][pc] ) > nKeyWordLen ) {
				ret = -1;
			} else {
				return pc;
			}
		}

		if( ret < 0 ) {
			pr = pc - 1;
		} else {
			pl = pc + 1;
		}

		pc = (pr + 1 - pl) / 2 + pl;
	}
	return -1;
}
//MIK END
//MIK START 2000.12.01 START
void CKeyWordSetMgr::SetKeyWordCase( int nIdx, int nCase )
{
	//大文字小文字判断は１ビットあれば実現できる。
	//今はint型(sizeof(int) * セット数 = 4 * 100 = 400)だが,
	//char型(sizeof(char) * セット数 = 1 * 100 = 100)で十分だし
	//ビット操作してもいい。
	if(nCase) {
		m_nKEYWORDCASEArr[nIdx] = TRUE;
	} else {
		m_nKEYWORDCASEArr[nIdx] = FALSE;
	}

	m_IsSorted[nIdx] = 0;
	return;
}

int CKeyWordSetMgr::GetKeyWordCase( int nIdx )
{
	return 	m_nKEYWORDCASEArr[nIdx];
}
//MIK END


/*[EOF]*/
