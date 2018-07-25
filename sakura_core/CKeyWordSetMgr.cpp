/*!	@file
	@brief 強調キーワード管理

	@author Norio Nakatani
	
	@date 2000.12.01 MIK binary search
	@date 2004.07.29-2005.01.27 Moca キーワードの可変長記憶
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, MIK
	Copyright (C) 2002, genta, Moca
	Copyright (C) 2004, Moca
	Copyright (C) 2005, Moca, genta

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose, 
	including commercial applications, and to alter it and redistribute it 
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such, 
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/
#include "StdAfx.h"
#include "CKeyWordSetMgr.h"
#include <limits>

//! 1ブロック当たりのキーワード数
static const int nKeyWordSetBlockSize = 50;

//! ブロックサイズで整列
inline int GetAlignmentSize( int nSize )
{
	return (nKeyWordSetBlockSize - 1 + nSize) / nKeyWordSetBlockSize * nKeyWordSetBlockSize;
}

/*!
	@note CKeyWordSetMgrは共有メモリ構造体に埋め込まれているため，
	そのままではコンストラクタが動かないことに注意．
*/
CKeyWordSetMgr::CKeyWordSetMgr( void )
{
	m_nCurrentKeyWordSetIdx = 0;
	m_nKeyWordSetNum = 0;
	m_nStartIdx[0] = 0;
	m_nStartIdx[1] = 0;
	m_nStartIdx[MAX_SETNUM] = 0;
	return;
}

CKeyWordSetMgr::~CKeyWordSetMgr( void)
{
	m_nKeyWordSetNum = 0;
	return;
}

/*!
	@brief 全キーワードセットの削除と初期化

	キーワードセットのインデックスを全て0とする．
	
	@date 2004.07.29 Moca 可変長記憶
*/
void CKeyWordSetMgr::ResetAllKeyWordSet( void )
{
	m_nKeyWordSetNum = 0;
	int i;
	for( i = 0; i < MAX_SETNUM+1; i++ ){
		m_nStartIdx[i] = 0;
	}
	for( i = 0; i < MAX_SETNUM; i++ ){
		m_nKeyWordNumArr[i] = 0;
	}
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
	//配列まるごとコピー
	memcpy_raw( m_szSetNameArr   , cKeyWordSetMgr.m_szSetNameArr   , sizeof( m_szSetNameArr )    );
	memcpy_raw( m_bKEYWORDCASEArr, cKeyWordSetMgr.m_bKEYWORDCASEArr, sizeof( m_bKEYWORDCASEArr ) );
	memcpy_raw( m_nStartIdx      , cKeyWordSetMgr.m_nStartIdx      , sizeof( m_nStartIdx )       );// 2004.07.29 Moca
	memcpy_raw( m_nKeyWordNumArr , cKeyWordSetMgr.m_nKeyWordNumArr , sizeof( m_nKeyWordNumArr )  );
	memcpy_raw( m_szKeyWordArr   , cKeyWordSetMgr.m_szKeyWordArr   , sizeof( m_szKeyWordArr )    );
	memcpy_raw( m_IsSorted       , cKeyWordSetMgr.m_IsSorted       , sizeof( m_IsSorted )        ); //MIK 2000.12.01 binary search
	memcpy_raw( m_nKeyWordMaxLenArr, cKeyWordSetMgr.m_nKeyWordMaxLenArr, sizeof( m_nKeyWordMaxLenArr ) ); //2014.05.04 Moca
	return *this;
}




/*! @brief キーワードセットの追加

	@date 2005.01.26 Moca 新規作成
	@date 2005.01.29 genta サイズ0で作成→reallocするように
*/
bool CKeyWordSetMgr::AddKeyWordSet(
	const wchar_t*	pszSetName,		//!< [in] セット名
	bool			bKEYWORDCASE,	//!< [in] 大文字小文字の区別．true:あり, false:無し
	int				nSize			//!< [in] 最初に領域を確保するサイズ．
)
{
	if( nSize < 0 ) nSize = nKeyWordSetBlockSize;
	if( MAX_SETNUM <= m_nKeyWordSetNum ){
		return false;
	}
	int nIdx = m_nKeyWordSetNum;	//追加位置
	m_nStartIdx[ ++m_nKeyWordSetNum ] = m_nStartIdx[ nIdx ];// サイズ0でセット追加

	if( !KeyWordReAlloc( nIdx, nSize ) ){
		--m_nKeyWordSetNum;	//	キーワードセットの追加をキャンセルする
		return false;
	}
	wcsncpy( m_szSetNameArr[nIdx], pszSetName, _countof(m_szSetNameArr[nIdx]) - 1 );
	m_szSetNameArr[nIdx][_countof(m_szSetNameArr[nIdx]) - 1] = L'\0';
	m_bKEYWORDCASEArr[nIdx] = bKEYWORDCASE;
	m_nKeyWordNumArr[nIdx] = 0;
	m_IsSorted[nIdx] = 0;	//MIK 2000.12.01 binary search
	return true;
}

/* ｎ番目のセットを削除 */
bool CKeyWordSetMgr::DelKeyWordSet( int nIdx )
{
	int		i;
	if( m_nKeyWordSetNum <= nIdx ||
		0 > nIdx
	){
		return false;
	}
	// キーワード領域を開放
	KeyWordReAlloc( nIdx, 0 );
	
	for( i = nIdx; i < m_nKeyWordSetNum - 1; ++i ){
		//配列まるごとコピー
		memcpy_raw( m_szSetNameArr[i], m_szSetNameArr[i + 1], sizeof( m_szSetNameArr[0] ) );
		m_bKEYWORDCASEArr[i] = m_bKEYWORDCASEArr[i + 1];
		m_nKeyWordNumArr[i] = m_nKeyWordNumArr[i + 1];
		m_nStartIdx[i] = m_nStartIdx[i + 1];	//	2004.07.29 Moca 可変長記憶
		m_IsSorted[i] = m_IsSorted[i+1];	//MIK 2000.12.01 binary search
		m_nKeyWordMaxLenArr[i] = m_nKeyWordMaxLenArr[i+1];	// 2014.05.04 Moca
	}
	m_nStartIdx[m_nKeyWordSetNum - 1] = m_nStartIdx[m_nKeyWordSetNum];	// 2007.07.14 ryoji これが無いと末尾＝最終セットの先頭になってしまう
	m_nKeyWordSetNum--;
	if( m_nKeyWordSetNum <= m_nCurrentKeyWordSetIdx ){
		m_nCurrentKeyWordSetIdx = m_nKeyWordSetNum - 1;
//セットが無くなったとき、m_nCurrentKeyWordSetIdxをわざと-1にするため、コメント化
//		if( 0 > m_nCurrentKeyWordSetIdx ){
//			m_nCurrentKeyWordSetIdx = 0;
//		}
	}
	return true;
}



/*! ｎ番目のセットのセット名を返す

	@param nIdx [in] セット番号 0～キーワードセット数-1
*/
const wchar_t* CKeyWordSetMgr::GetTypeName( int nIdx )
{
	if( nIdx < 0 || m_nKeyWordSetNum <= nIdx ){
		return NULL;
	}
	return m_szSetNameArr[nIdx];
}

/*! ｎ番目のセットのセット名を再設定

	@date 2005.01.26 Moca 新規作成
*/
const wchar_t* CKeyWordSetMgr::SetTypeName( int nIdx, const wchar_t* name )
{
	if( NULL == name || nIdx < 0 || m_nKeyWordSetNum <= nIdx ){
		return NULL;
	}
	wcsncpy( m_szSetNameArr[nIdx], name, MAX_SETNAMELEN );
	m_szSetNameArr[nIdx][MAX_SETNAMELEN] = L'\0';
	return m_szSetNameArr[nIdx];
}

/* ｎ番目のセットのキーワードの数を返す */
int CKeyWordSetMgr::GetKeyWordNum( int nIdx )
{
	if( nIdx < 0 || m_nKeyWordSetNum <= nIdx ){
		return 0;
	}
	return m_nKeyWordNumArr[nIdx];
}

/*! ｎ番目のセットのｍ番目のキーワードを返す

	@param nIdx [in] キーワードセット番号
	@param nIdx2 [in] キーワード番号
*/
const wchar_t* CKeyWordSetMgr::GetKeyWord( int nIdx, int nIdx2 )
{
	if( nIdx < 0 || m_nKeyWordSetNum <= nIdx ){
		return NULL;
	}
	if( nIdx2 < 0 || m_nKeyWordNumArr[nIdx] <= nIdx2 ){
		return NULL;
	}
	return m_szKeyWordArr[m_nStartIdx[nIdx] + nIdx2];
}

//! ｎ番目のセットのｍ番目のキーワードを編集
const wchar_t* CKeyWordSetMgr::UpdateKeyWord(
	int				nIdx,		//!< [in] キーワードセット番号
	int				nIdx2,		//!< [in] キーワード番号
	const WCHAR*	pszKeyWord	//!< [in] 設定するキーワード
)
{
	int i;
	if( nIdx < 0 || m_nKeyWordSetNum <= nIdx ){
		return NULL;
	}
	if( nIdx2 < 0 || m_nKeyWordNumArr[nIdx] <= nIdx2 ){
		return NULL;
	}
	/* 0バイトの長さのキーワードは編集しない */
	if( pszKeyWord[0] == L'\0' ){
		return NULL;
	}
	/* 重複したキーワードは編集しない */
	for( i = m_nStartIdx[nIdx]; i < m_nStartIdx[nIdx] + m_nKeyWordNumArr[nIdx]; ++i ){
		if( 0 == wcscmp( m_szKeyWordArr[i], pszKeyWord ) ){
			return NULL;
		}
	}
	m_IsSorted[nIdx] = 0;	//MIK 2000.12.01 binary search
	wchar_t* p = m_szKeyWordArr[m_nStartIdx[nIdx] + nIdx2];
	wcsncpy( p, pszKeyWord, MAX_KEYWORDLEN );
	p[MAX_KEYWORDLEN] = L'\0';
	return p;
}


/*! ｎ番目のセットにキーワードを追加

	@param nIdx [in] セット番号
	@param pszKeyWord [in] キーワード文字列
	
	@return 0: 成功, 1: セット番号エラー，2: メモリ確保エラー
		3: キーワード不正，4: キーワード重複

*/
int CKeyWordSetMgr::AddKeyWord( int nIdx, const wchar_t* pszKeyWord )
{
	int  i;
	if( m_nKeyWordSetNum <= nIdx ){
		return 1;
	}
// 2004.07.29 Moca
	if( !KeyWordReAlloc( nIdx, m_nKeyWordNumArr[nIdx] + 1 ) ){
		return 2;
	}
//	if( MAX_KEYWORDNUM <= m_nKeyWordNumArr[nIdx] ){
//		return FALSE;
//	}

	/* 0バイトの長さのキーワードは登録しない */
	if( pszKeyWord[0] == L'\0' ){
		return 3;
	}
	/* 重複したキーワードは登録しない */
	for( i = m_nStartIdx[nIdx]; i < m_nStartIdx[nIdx] + m_nKeyWordNumArr[nIdx]; ++i ){
		if( 0 == wcscmp( m_szKeyWordArr[i], pszKeyWord ) ){
			return 4;
		}
	}
	/* MAX_KEYWORDLENより長いキーワードは切り捨てる */
	if( MAX_KEYWORDLEN < wcslen( pszKeyWord ) ){
		wmemcpy( m_szKeyWordArr[m_nStartIdx[nIdx] + m_nKeyWordNumArr[nIdx]], pszKeyWord, MAX_KEYWORDLEN );
		m_szKeyWordArr[m_nStartIdx[nIdx] + m_nKeyWordNumArr[nIdx]][MAX_KEYWORDLEN] = L'\0';
	}else{
		wcscpy( m_szKeyWordArr[m_nStartIdx[nIdx] + m_nKeyWordNumArr[nIdx]], pszKeyWord );
	}
	m_nKeyWordNumArr[nIdx]++;
	m_IsSorted[nIdx] = 0;	//MIK 2000.12.01 binary search
	return 0;
}


/*! ｎ番目のセットのｍ番目のキーワードを削除

	@param nIdx [in] キーワードセット番号
	@param nIdx2 [in] キーワード番号
*/
int CKeyWordSetMgr::DelKeyWord( int nIdx, int nIdx2 )
{
	if( nIdx < 0 || m_nKeyWordSetNum <= nIdx ){
		return 1;
	}
	if( nIdx2 < 0 ||  m_nKeyWordNumArr[nIdx] <= nIdx2 ){
		return 2;
	}
	if( 0 >= m_nKeyWordNumArr[nIdx]	){
		return 3;	//	登録数が0なら上の条件で引っかかるのでここには来ない？
	}
	int nDelKeywordLen = wcslen( m_szKeyWordArr[m_nStartIdx[nIdx] + nIdx2] );
	int  i;
	int  endPos = m_nStartIdx[nIdx] + m_nKeyWordNumArr[nIdx] - 1;
	for( i = m_nStartIdx[nIdx] + nIdx2; i < endPos; ++i ){
		wcscpy( m_szKeyWordArr[i], m_szKeyWordArr[i + 1] );
	}
	m_nKeyWordNumArr[nIdx]--;

	// 2005.01.26 Moca 1つずらすだけなので、ソートの状態は保持される
	// m_IsSorted[nIdx] = 0;	//MIK 2000.12.01 binary search
	KeyWordReAlloc( nIdx, m_nKeyWordNumArr[nIdx] );	// 2004.07.29 Moca

	// 2014.05.04 Moca キーワード長の再計算
	if( nDelKeywordLen == m_nKeyWordMaxLenArr[nIdx] ){
		KeywordMaxLen(nIdx);
	}
	return 0;
}


//MIK START 2000.12.01 binary search
/*!	キーワードのソートとキーワード長の最大値計算

	@param nIdx [in] キーワードセット番号

*/
typedef int (__cdecl *qsort_callback)(const void *, const void *);
void CKeyWordSetMgr::SortKeyWord( int nIdx )
{
	//nIdxのセットをソートする。
	if( m_bKEYWORDCASEArr[nIdx] ) {
		qsort(
			m_szKeyWordArr[m_nStartIdx[nIdx]],
			m_nKeyWordNumArr[nIdx],
			sizeof(m_szKeyWordArr[0]),
			(qsort_callback)wcscmp
		);
	}
	else {
		qsort(
			m_szKeyWordArr[m_nStartIdx[nIdx]],
			m_nKeyWordNumArr[nIdx],
			sizeof(m_szKeyWordArr[0]),
			(qsort_callback)wcsicmp
		);
	}
	KeywordMaxLen(nIdx);
	m_IsSorted[nIdx] = 1;
	return;
}

void CKeyWordSetMgr::KeywordMaxLen(int nIdx)
{
	int i;
	int len;
	int nMaxLen = 0;
	const int nEnd = m_nStartIdx[nIdx] + m_nKeyWordNumArr[nIdx];
	for( i = m_nStartIdx[nIdx]; i < nEnd; i++ ){
		len = wcslen( m_szKeyWordArr[i] );
		if( nMaxLen < len ){
			nMaxLen = len;
		}
	}
	m_nKeyWordMaxLenArr[nIdx] = nMaxLen;
}


/** nIdx番目のキーワードセットから pszKeyWordを探す。
	見つかれば 0以上を、見つからなければ負の数を返す。
	@retval 0以上 見つかった。
	@retval -1     見つからなかった。
	@retval -2     見つからなかったが、pszKeywordから始まるキーワードが存在している。
	@retval intmax 見つかったが、pszKeywordから始まる、より長いキーワードが存在している。
*/
int CKeyWordSetMgr::SearchKeyWord2( int nIdx, const wchar_t* pszKeyWord, int nKeyWordLen )
{
	//sort
	if( m_IsSorted[nIdx] == 0 ) {
		SortKeyWord( nIdx );
	}

	if( m_nKeyWordMaxLenArr[nIdx] < nKeyWordLen ) {
		return -1; // 字数オーバー。
	}

	int result = -1;
	int pl = m_nStartIdx[nIdx];
	int pr = m_nStartIdx[nIdx] + m_nKeyWordNumArr[nIdx] - 1;
	int pc = (pr + 1 - pl) / 2 + pl;
	int (*const cmp)(const wchar_t*, const wchar_t*, size_t) = m_bKEYWORDCASEArr[nIdx] ? wcsncmp : wcsnicmp;
	while( pl <= pr ) {
		const int ret = cmp( pszKeyWord, m_szKeyWordArr[pc], nKeyWordLen );
		if( 0 < ret ) {
			pl = pc + 1;
		} else if( ret < 0 ) {
			pr = pc - 1;
		} else {
			if( wcslen( m_szKeyWordArr[pc] ) > static_cast<size_t>(nKeyWordLen) ) {
				// 始まりは一致したが長さが足りない。
				if( 0 <= result ) {
					result = std::numeric_limits<int>::max();
					break;
				}
				result = -2;
				// ぴったり一致するキーワードを探すために続ける。
				pr = pc - 1;
			} else {
				// 一致するキーワードが見つかった。
				if( result == -2 ) {
					result = std::numeric_limits<int>::max();
					break;
				}
				result = pc - m_nStartIdx[nIdx];
				// より長いキーワードを探すために続ける。
				pl = pc + 1;
			}
		}
		pc = (pr + 1 - pl) / 2 + pl;
	}
	return result;
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
		m_bKEYWORDCASEArr[nIdx] = true;
	} else {
		m_bKEYWORDCASEArr[nIdx] = false;
	}

	m_IsSorted[nIdx] = 0;
	return;
}

bool CKeyWordSetMgr::GetKeyWordCase( int nIdx )
{
	return 	m_bKEYWORDCASEArr[nIdx];
}
//MIK END


// From Here 2004.07.29 Moca 可変長記憶
/*!	@brief \\0またはTABで区切られた文字列からキーワードを設定

	@return 登録に成功したキーワード数
	
	@author Moca
	@date 2004.07.29 Moca CShareData::ShareData_IO_2内のコードを元に移築・作成
*/
int CKeyWordSetMgr::SetKeyWordArr(
	int				nIdx,			//!< [in] キーワードセット番号
	int				nSize,			//!< [in] キーワード数
	const wchar_t*	pszKeyWordArr	//!< [in]「key\\tword\\t\\0」又は「key\\0word\\0\\0」の形式
)
{
	if( !KeyWordReAlloc( nIdx, nSize ) ){
		return 0;
	}
	int cnt, i;
	const wchar_t* ptr = pszKeyWordArr;
	for( cnt = 0, i = m_nStartIdx[nIdx];
		i < m_nStartIdx[nIdx] + nSize && *ptr != L'\0'; cnt++, i++ ){
		//	May 25, 2003 キーワードの区切りとして\0以外にTABを受け付けるようにする
		const wchar_t* pTop = ptr;	// キーワードの先頭位置を保存
		while( *ptr != L'\t' && *ptr != L'\0' )
			++ptr;
		int kwlen = ptr - pTop;
		wmemcpy( m_szKeyWordArr[i], pTop, kwlen );
		m_szKeyWordArr[i][kwlen] = L'\0';
		++ptr;
	}
	m_nKeyWordNumArr[nIdx] = cnt;
	return nSize;
}

/*!
	キーワードリストを設定

	@return 登録したキーワード数．0は失敗．
*/
int CKeyWordSetMgr::SetKeyWordArr(
	int				nIdx,				//!< [in] キーワードセット番号
	int				nSize,				//!< [in] ppszKeyWordArrの要素数
	const wchar_t*	ppszKeyWordArr[]	//!< [in] キーワードの配列(重複・長さ制限等、考慮済みであること)
)
{
	if( !KeyWordReAlloc( nIdx, nSize ) ){
		return 0;
	}
	int cnt, i;
	for( cnt = 0, i = m_nStartIdx[nIdx]; i < m_nStartIdx[nIdx] + nSize; cnt++, i++ ){
		wcscpy( m_szKeyWordArr[i], ppszKeyWordArr[cnt] );
	}
	m_nKeyWordNumArr[nIdx] = nSize;
	return nSize;
}

/*!	@brief キーワードの整理

	重複や使用不可のキーワードを取り除く

	@param nIdx [in] キーワードセット番号
	
	@return 削除したキーワード数
*/
int CKeyWordSetMgr::CleanKeyWords( int nIdx )
{
	// 先にソートしておかないと、後で順番が変わると都合が悪い
	if( m_IsSorted[nIdx] == 0 )	SortKeyWord( nIdx );

	int nDelCount = 0;	//!< 削除キーワード数
	int i = 0;
	while( i < GetKeyWordNum( nIdx ) - 1 ){
		const wchar_t* p = GetKeyWord( nIdx, i );
		bool bDelKey = false;	//!< trueなら削除対象
		// 重複するキーワードか
		const wchar_t* r = GetKeyWord( nIdx, i + 1 );
		unsigned int nKeyWordLen = wcslen( p );
		if( nKeyWordLen == wcslen( r ) ){
			if( m_bKEYWORDCASEArr[nIdx] ){
				if( 0 == auto_memcmp( p, r, nKeyWordLen ) ){
					bDelKey = true;
				}
			}else{
				if( 0 == auto_memicmp( p, r, nKeyWordLen ) ){
					bDelKey = true;
				}
			}
		}
		if( bDelKey ){
			DelKeyWord( nIdx, i );
			nDelCount++;
			//後ろがずれるので、iを増やさない
		}else{
			i++;
		}
	}
	return nDelCount;
}

/*!	@brief キーワード追加余地の問い合わせ

	@param nIdx [in] キーワードセット番号
	@return true: もう1つ追加可能, false: 追加不可能

	@date 2005.01.26 Moca 新規作成
	@date 2005.01.29 genta 割り当て済みの領域に空きがあれば拡張不能でも追加可能
*/
bool CKeyWordSetMgr::CanAddKeyWord( int nIdx )
{
	//	割り当て済みの領域の空きをまず調べる
	int nSizeOld = GetAllocSize( nIdx );
	if( m_nKeyWordNumArr[nIdx] < nSizeOld ){
		return true;
	}

	//	割り当て済み領域がいっぱいならば，割り当て可能領域の有無を確認
	//	一応割り当て最小単位分残っていることを確認．
	if( GetFreeSize() >= nKeyWordSetBlockSize ){
		return true;
	}

	//	それでもだめか
	return false;
}

#if 0
/*!	新しいキーワードセットのキーワード領域を確保する
	m_nKeyWordSetNumは、呼び出し側が、呼び出した後に+1する
*/
bool CKeyWordSetMgr::KeyWordAlloc( int nSize )
{
	// assert( m_nKeyWordSetNum < MAX_SETNUM );
	// assert( 0 <= nSize );

	// ブロックのサイズで整列
	int nAllocSize = GetAlignmentSize( nSize );

	if( GetFreeSize() < nAllocSize ){
		// メモリ不足
		return false;
	}
	m_nStartIdx[m_nKeyWordSetNum + 1] = m_nStartIdx[m_nKeyWordSetNum] + nAllocSize;
	int i;
	for( i = m_nKeyWordSetNum + 1; i < MAX_SETNUM; i++ ){
		m_nStartIdx[i + 1] = m_nStartIdx[i];
	}
	return true;
}
#endif

/*!	初期化済みのキーワードセットのキーワード領域の再割り当て、解放を行う

	@param nIdx [in] キーワードセット番号
	@param nSize [in] 必要なキーワード数 (0～)
*/
bool CKeyWordSetMgr::KeyWordReAlloc( int nIdx, int nSize )
{
	// assert( 0 <= nIdx && nIdx < m_nKeyWordSetNum );

	// ブロックのサイズで整列
	int nAllocSize = GetAlignmentSize( nSize );
	int nSizeOld = GetAllocSize( nIdx );

	if( nSize < 0 ){
		return false;
	}
	if( nAllocSize == nSizeOld ){
		// サイズ変更なし
		return true;
	}

	int nDiffSize = nAllocSize - nSizeOld;
	if( GetFreeSize() < nDiffSize ){
		// メモリ不足
		return false;
	}
	// 後ろのキーワードセットのキーワードをすべて移動する
	int i;
	if( nIdx + 1 < m_nKeyWordSetNum ){
		int nKeyWordIdx = m_nStartIdx[nIdx + 1];
		int nKeyWordNum = m_nStartIdx[m_nKeyWordSetNum] - m_nStartIdx[nIdx + 1];
		memmove( m_szKeyWordArr[nKeyWordIdx + nDiffSize],
			m_szKeyWordArr[nKeyWordIdx],
			nKeyWordNum * sizeof( m_szKeyWordArr[0] ) );
	}
	for( i = nIdx + 1; i <= m_nKeyWordSetNum; i++ ){
		m_nStartIdx[ i ] += nDiffSize;
	}
	return true;
}

/*!	@brief 割り当て済みキーワード数 

	@param nIdx [in] キーワードセット番号
	@return キーワードセットに割り当て済みのキーワード数
*/
int CKeyWordSetMgr::GetAllocSize( int nIdx ) const
{
	return m_nStartIdx[nIdx + 1] - m_nStartIdx[nIdx];
}

/*! 共有空きスペース

	@date 2004.07.29 Moca 新規作成
	
	@return 共有空き領域(キーワード数)
 */
int CKeyWordSetMgr::GetFreeSize( void ) const 
{
	return MAX_KEYWORDNUM - m_nStartIdx[m_nKeyWordSetNum];
}
// To Here 2004.07.29 Moca

// キーワードセット名からセット番号を取得。見つからなければ -1
//	Uchi 2010/4/14
int  CKeyWordSetMgr::SearchKeyWordSet( const wchar_t* pszKeyWord )
{
	int		i;
	int 	nIdx = -1;
	for (i = 0; i < m_nKeyWordSetNum; i++) {
		if ( wcscmp(m_szSetNameArr[i], pszKeyWord) == 0) {
			nIdx = i;
			break;
		}
	}
	return nIdx;
}



