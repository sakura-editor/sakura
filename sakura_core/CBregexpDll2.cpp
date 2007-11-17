#include "stdafx.h"
#include "CBregexpDll2.h"

//	2007.07.22 genta : DLL判別用
static const TCHAR P_BREG[] = _T("BREGEXP.DLL");
static const TCHAR P_ONIG[] = _T("bregonig.dll");


CBregexpDll2::CBregexpDll2()
{
}

CBregexpDll2::~CBregexpDll2()
{
}

/*!
	@date 2001.07.05 genta 引数追加。ただし、ここでは使わない。
	@date 2007.06.25 genta 複数のDLL名に対応
	@date 2007.09.13 genta サーチルールを変更
		@li 指定有りの場合はそれのみを返す
		@li 指定無し(NULLまたは空文字列)の場合はBREGONIG, BREGEXPの順で試みる
*/
LPCTSTR CBregexpDll2::GetDllNameInOrder( LPCTSTR str, int index )
{

	switch( index ){
	case 0:
		//	NULLはリストの終わりを意味するので，
		//	str == NULLの場合にそのまま返してはいけない．
		return str == NULL || str[0] == _T('\0') ? P_ONIG : str ;

// 2007.11.04 kobake 文字列管理がUNICODEになり、BREGEXP は使えなくなったので、コメントアウト。
//	case 1:
//		return str == NULL || str[0] == _T('\0') ? P_BREG : NULL;
	}
	return NULL;
}


/*!
	DLLの初期化

	関数のアドレスを取得してメンバに保管する．

	@retval 0 成功
	@retval 1 アドレス取得に失敗
*/
int CBregexpDll2::InitDll(void)
{
	//DLL内関数名リスト
	const ImportTable table[] = {
		{ &m_BMatch,			"BMatchW" },
		{ &m_BSubst,			"BSubstW" },
		{ &m_BTrans,			"BTransW" },
		{ &m_BSplit,			"BSplitW" },
		{ &m_BRegfree,			"BRegfreeW" },
		{ &m_BRegexpVersion,	"BRegexpVersionW" },
		{ &m_BMatchEx,			"BMatchExW" },
		{ &m_BSubstEx,			"BSubstExW" },
		{ NULL, 0 }
	};
	
	if( ! RegisterEntries( table )){
		return 1;
	}
	
	return 0;
}

/*!
	BREGEXP_W構造体の解放
*/
int CBregexpDll2::DeinitDll( void )
{
	return 0;
}
