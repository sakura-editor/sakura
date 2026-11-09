/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include "view/CEditView.h" // SColorStrategyInfo
#include "CColor_Numeric.h"
#include "parse/CWordParse.h"
#include "util/string_ex2.h"
#include "doc/layout/CLayout.h"
#include "types/CTypeSupport.h"

static int IsNumber( const CStringRef& cStr, int offset );/* 数値ならその長さを返す */	//@@@ 2001.02.17 by MIK

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         半角数値                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CColor_Numeric::BeginColor(const CStringRef& cStr, int nPos)
{
	if(!cStr.IsValid())return false;

	int	nnn;

	if( _IsPosKeywordHead(cStr,nPos)
		&& (nnn = IsNumber(cStr, nPos)) > 0 )		/* 半角数字を表示する */
	{
		/* キーワード文字列の終端をセットする */
		this->m_nCOMMENTEND = nPos + nnn;
		return true;	/* 半角数値である */ // 2002/03/13 novice
	}
	return false;
}

bool CColor_Numeric::EndColor([[maybe_unused]] const CStringRef& cStr, int nPos)
{
	if( nPos == this->m_nCOMMENTEND ){
		return true;
	}
	return false;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         実装補助                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//@@@ 2001.11.07 Start by MIK
/*
 * 数値なら長さを返す。
 * 10進数の整数または小数。16進数(正数)。
 * 文字列   数値(色分け)
 * ---------------------
 * 123      123
 * 0123     0123
 * 0xfedc   0xfedc
 * -123     -123
 * &H9a     &H9a     (ただしソース中の#ifを有効にしたとき)
 * -0x89a   0x89a
 * 0.5      0.5
 * 0.56.1   0.56 , 1 (ただしソース中の#ifを有効にしたら"0.56.1"になる)
 * .5       5        (ただしソース中の#ifを有効にしたら".5"になる)
 * -.5      5        (ただしソース中の#ifを有効にしたら"-.5"になる)
 * 123.     123
 * 0x567.8  0x567 , 8
 */
/*
 * 半角数値
 *   1, 1.2, 1.2.3, .1, 0xabc, 1L, 1F, 1.2f, 0x1L, 0x2F, -.1, -1, 1e2, 1.2e+3, 1.2e-3, -1e0
 *   10進数, 16進数, LF接尾語, 浮動小数点数, 負符号
 *   IPアドレスのドット連結(本当は数値じゃないんだよね)
 */
static int IsNumber(const CStringRef& cStr,/*const wchar_t *buf,*/ int offset/*, int length*/)
{
	const WCHAR* p;
	const WCHAR* q;
	int i = 0;
	int d = 0;
	int f = 0;

	p = cStr.GetPtr() + offset;
	q = cStr.GetPtr() + cStr.GetLength();

	if( *p == LCHAR('0') )  /* 10進数,Cの16進数 */
	{
		p++; i++;
		if( ( p < q ) && ( *p == LCHAR('x') ) )  /* Cの16進数 */
		{
			p++; i++;
			while( p < q )
			{
				if( ( *p >= LCHAR('0') && *p <= LCHAR('9') )
				 || ( *p >= LCHAR('A') && *p <= LCHAR('F') )
				 || ( *p >= LCHAR('a') && *p <= LCHAR('f') ) )
				{
					p++; i++;
				}
				else
				{
					break;
				}
			}
			/* "0x" なら "0" だけが数値 */
			if( i == 2 ) return 1;
			
			/* 接尾語 */
			if( p < q )
			{
				if( *p == LCHAR('L') || *p == LCHAR('l') || *p == LCHAR('F') || *p == LCHAR('f') )
				{
					p++; i++;
				}
			}
			return i;
		}
		else if( *p >= LCHAR('0') && *p <= LCHAR('9') )
		{
			p++; i++;
			while( p < q )
			{
				if( *p < LCHAR('0') || *p > LCHAR('9') )
				{
					if( *p == LCHAR('.') )
					{
						if( f == 1 ) break;  /* 指数部に入っている */
						d++;
						if( d > 1 )
						{
							if( *(p - 1) == LCHAR('.') ) break;  /* "." が連続なら中断 */
						}
					}
					else if( *p == LCHAR('E') || *p == LCHAR('e') )
					{
						if( f == 1 ) break;  /* 指数部に入っている */
						if( p + 2 < q )
						{
							if( ( *(p + 1) == LCHAR('+') || *(p + 1) == LCHAR('-') )
							 && ( *(p + 2) >= LCHAR('0') && *(p + 2) <= LCHAR('9') ) )
							{
								p++; i++;
								p++; i++;
								f = 1;
							}
							else if( *(p + 1) >= LCHAR('0') && *(p + 1) <= LCHAR('9') )
							{
								p++; i++;
								f = 1;
							}
							else
							{
								break;
							}
						}
						else if( p + 1 < q )
						{
							if( *(p + 1) >= LCHAR('0') && *(p + 1) <= LCHAR('9') )
							{
								p++; i++;
								f = 1;
							}
							else
							{
								break;
							}
						}
						else
						{
							break;
						}
					}
					else
					{
						break;
					}
				}
				p++; i++;
			}
			if( *(p - 1)  == LCHAR('.') ) return i - 1;  /* 最後が "." なら含めない */
			/* 接尾語 */
			if( p < q )
			{
				if( (( d == 0 ) && ( *p == LCHAR('L') || *p == LCHAR('l') ))
				 || *p == LCHAR('F') || *p == LCHAR('f') )
				{
					p++; i++;
				}
			}
			return i;
		}
		else if( *p == LCHAR('.') )
		{
			while( p < q )
			{
				if( *p < LCHAR('0') || *p > LCHAR('9') )
				{
					if( *p == LCHAR('.') )
					{
						if( f == 1 ) break;  /* 指数部に入っている */
						d++;
						if( d > 1 )
						{
							if( *(p - 1) == LCHAR('.') ) break;  /* "." が連続なら中断 */
						}
					}
					else if( *p == LCHAR('E') || *p == LCHAR('e') )
					{
						if( f == 1 ) break;  /* 指数部に入っている */
						if( p + 2 < q )
						{
							if( ( *(p + 1) == LCHAR('+') || *(p + 1) == LCHAR('-') )
							 && ( *(p + 2) >= LCHAR('0') && *(p + 2) <= LCHAR('9') ) )
							{
								p++; i++;
								p++; i++;
								f = 1;
							}
							else if( *(p + 1) >= LCHAR('0') && *(p + 1) <= LCHAR('9') )
							{
								p++; i++;
								f = 1;
							}
							else
							{
								break;
							}
						}
						else if( p + 1 < q )
						{
							if( *(p + 1) >= LCHAR('0') && *(p + 1) <= LCHAR('9') )
							{
								p++; i++;
								f = 1;
							}
							else
							{
								break;
							}
						}
						else
						{
							break;
						}
					}
					else
					{
						break;
					}
				}
				p++; i++;
			}
			if( *(p - 1)  == LCHAR('.') ) return i - 1;  /* 最後が "." なら含めない */
			/* 接尾語 */
			if( p < q )
			{
				if( *p == LCHAR('F') || *p == LCHAR('f') )
				{
					p++; i++;
				}
			}
			return i;
		}
		else if( *p == LCHAR('E') || *p == LCHAR('e') )
		{
			p++; i++;
			while( p < q )
			{
				if( *p < LCHAR('0') || *p > LCHAR('9') )
				{
					if( ( *p == LCHAR('+') || *p == LCHAR('-') ) && ( *(p - 1) == LCHAR('E') || *(p - 1) == LCHAR('e') ) )
					{
						if( p + 1 < q )
						{
							if( *(p + 1) < LCHAR('0') || *(p + 1) > LCHAR('9') )
							{
								/* "0E+", "0E-" */
								break;
							}
						}
						else
						{
							/* "0E-", "0E+" */
							break;
						}
					}
					else
					{
						break;
					}
				}
				p++; i++;
			}
			if( i == 2 ) return 1;  /* "0E", 0e" なら "0" が数値 */
			/* 接尾語 */
			if( p < q )
			{
				if( (( d == 0 ) && ( *p == LCHAR('L') || *p == LCHAR('l') ))
				 || *p == LCHAR('F') || *p == LCHAR('f') )
				{
					p++; i++;
				}
			}
			return i;
		}
		else
		{
			/* "0" だけが数値 */
			/*if( *p == L'.' ) return i - 1;*/  /* 最後が "." なら含めない */
			if( p < q )
			{
				if( (( d == 0 ) && ( *p == LCHAR('L') || *p == LCHAR('l') ))
				 || *p == LCHAR('F') || *p == LCHAR('f') )
				{
					p++; i++;
				}
			}
			return i;
		}
	}

	else if( *p >= LCHAR('1') && *p <= LCHAR('9') )  /* 10進数 */
	{
		p++; i++;
		while( p < q )
		{
			if( *p < LCHAR('0') || *p > LCHAR('9') )
			{
				if( *p == LCHAR('.') )
				{
					if( f == 1 ) break;  /* 指数部に入っている */
					d++;
					if( d > 1 )
					{
						if( *(p - 1) == LCHAR('.') ) break;  /* "." が連続なら中断 */
					}
				}
				else if( *p == LCHAR('E') || *p == LCHAR('e') )
				{
					if( f == 1 ) break;  /* 指数部に入っている */
					if( p + 2 < q )
					{
						if( ( *(p + 1) == LCHAR('+') || *(p + 1) == LCHAR('-') )
						 && ( *(p + 2) >= LCHAR('0') && *(p + 2) <= LCHAR('9') ) )
						{
							p++; i++;
							p++; i++;
							f = 1;
						}
						else if( *(p + 1) >= LCHAR('0') && *(p + 1) <= LCHAR('9') )
						{
							p++; i++;
							f = 1;
						}
						else
						{
							break;
						}
					}
					else if( p + 1 < q )
					{
						if( *(p + 1) >= LCHAR('0') && *(p + 1) <= LCHAR('9') )
						{
							p++; i++;
							f = 1;
						}
						else
						{
							break;
						}
					}
					else
					{
						break;
					}
				}
				else
				{
					break;
				}
			}
			p++; i++;
		}
		if( *(p - 1) == LCHAR('.') ) return i - 1;  /* 最後が "." なら含めない */
		/* 接尾語 */
		if( p < q )
		{
			if( (( d == 0 ) && ( *p == LCHAR('L') || *p == LCHAR('l') ))
			 || *p == LCHAR('F') || *p == LCHAR('f') )
			{
				p++; i++;
			}
		}
		return i;
	}

	else if( *p == LCHAR('-') )  /* マイナス */
	{
		p++; i++;
		while( p < q )
		{
			if( *p < LCHAR('0') || *p > LCHAR('9') )
			{
				if( *p == LCHAR('.') )
				{
					if( f == 1 ) break;  /* 指数部に入っている */
					d++;
					if( d > 1 )
					{
						if( *(p - 1) == LCHAR('.') ) break;  /* "." が連続なら中断 */
					}
				}
				else if( *p == LCHAR('E') || *p == LCHAR('e') )
				{
					if( f == 1 ) break;  /* 指数部に入っている */
					if( p + 2 < q )
					{
						if( ( *(p + 1) == LCHAR('+') || *(p + 1) == LCHAR('-') )
						 && ( *(p + 2) >= LCHAR('0') && *(p + 2) <= LCHAR('9') ) )
						{
							p++; i++;
							p++; i++;
							f = 1;
						}
						else if( *(p + 1) >= LCHAR('0') && *(p + 1) <= LCHAR('9') )
						{
							p++; i++;
							f = 1;
						}
						else
						{
							break;
						}
					}
					else if( p + 1 < q )
					{
						if( *(p + 1) >= LCHAR('0') && *(p + 1) <= LCHAR('9') )
						{
							p++; i++;
							f = 1;
						}
						else
						{
							break;
						}
					}
					else
					{
						break;
					}
				}
				else
				{
					break;
				}
			}
			p++; i++;
		}
		/* "-", "-." だけなら数値でない */
		//@@@ 2001.11.09 start MIK
		//if( i <= 2 ) return 0;
		//if( *(p - 1)  == L'.' ) return i - 1;  /* 最後が "." なら含めない */
		if( i == 1 ) return 0;
		if( *(p - 1) == LCHAR('.') )
		{
			i--;
			if( i == 1 ) return 0;
			return i;
		}  //@@@ 2001.11.09 end MIK
		/* 接尾語 */
		if( p < q )
		{
			if( (( d == 0 ) && ( *p == LCHAR('L') || *p == LCHAR('l') ))
			 || *p == LCHAR('F') || *p == LCHAR('f') )
			{
				p++; i++;
			}
		}
		return i;
	}

	else if( *p == LCHAR('.') )  /* 小数点 */
	{
		d++;
		p++; i++;
		while( p < q )
		{
			if( *p < LCHAR('0') || *p > LCHAR('9') )
			{
				if( *p == LCHAR('.') )
				{
					if( f == 1 ) break;  /* 指数部に入っている */
					d++;
					if( d > 1 )
					{
						if( *(p - 1) == LCHAR('.') ) break;  /* "." が連続なら中断 */
					}
				}
				else if( *p == LCHAR('E') || *p == LCHAR('e') )
				{
					if( f == 1 ) break;  /* 指数部に入っている */
					if( p + 2 < q )
					{
						if( ( *(p + 1) == LCHAR('+') || *(p + 1) == LCHAR('-') )
						 && ( *(p + 2) >= LCHAR('0') && *(p + 2) <= LCHAR('9') ) )
						{
							p++; i++;
							p++; i++;
							f = 1;
						}
						else if( *(p + 1) >= LCHAR('0') && *(p + 1) <= LCHAR('9') )
						{
							p++; i++;
							f = 1;
						}
						else
						{
							break;
						}
					}
					else if( p + 1 < q )
					{
						if( *(p + 1) >= LCHAR('0') && *(p + 1) <= LCHAR('9') )
						{
							p++; i++;
							f = 1;
						}
						else
						{
							break;
						}
					}
					else
					{
						break;
					}
				}
				else
				{
					break;
				}
			}
			p++; i++;
		}
		/* "." だけなら数値でない */
		if( i == 1 ) return 0;
		if( *(p - 1)  == LCHAR('.') ) return i - 1;  /* 最後が "." なら含めない */
		/* 接尾語 */
		if( p < q )
		{
			if( *p == LCHAR('F') || *p == LCHAR('f') )
			{
				p++; i++;
			}
		}
		return i;
	}

#if 0
	else if( *p == LCHAR('&') )  /* VBの16進数 */
	{
		p++; i++;
		if( ( p < q ) && ( *p == LCHAR('H') ) )
		{
			p++; i++;
			while( p < q )
			{
				if( ( *p >= LCHAR('0') && *p <= LCHAR('9') )
				 || ( *p >= LCHAR('A') && *p <= LCHAR('F') )
				 || ( *p >= LCHAR('a') && *p <= LCHAR('f') ) )
				{
					p++; i++;
				}
				else
				{
					break;
				}
			}
			/* "&H" だけなら数値でない */
			if( i == 2 ) i = 0;
			return i;
		}

		/* "&" だけなら数値でない */
		return 0;
	}
#endif

	/* 数値ではない */
	return 0;
}
//@@@ 2001.11.07 End by MIK
