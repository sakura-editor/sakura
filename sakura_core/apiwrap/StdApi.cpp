#include "StdAfx.h"
#include <vector>
#include "StdApi.h"
#include "charset/charcode.h"
#include "_os/COsVersionInfo.h"

using namespace std;

#ifndef _UNICODE
/*!
	ワイド文字列からマルチバイト文字列を生成する。
	マルチバイト文字列のために新しいメモリ領域が確保されるので、
	使い終わったらDestroyMbStringを呼ぶこと！

	@retval 変換されたACHAR文字列
*/
static ACHAR* CreateMbString(
	const WCHAR*	pWideString,	//!< [in]  元のWCHAR文字列
	int				nWideLength,	//!< [in]  元のWCHAR文字列の長さ。文字単位。
	int*			pnMbLength		//!< [out] 変換されたACHAR文字列の長さの受け取り先。文字単位。
)
{
	//必要な領域サイズを取得
	int nNewLen=WideCharToMultiByte(
		CP_SJIS,				// 2008/5/12 Uchi
		0,
		pWideString,
		nWideLength,
		NULL,
		0,
		NULL,
		NULL
	);

	//領域を確保
	ACHAR* buf=new ACHAR[nNewLen+1];

	//変換
	nNewLen = WideCharToMultiByte(
		CP_SJIS,				// 2008/5/12 Uchi
		0,
		pWideString,
		nWideLength,
		buf,
		nNewLen,
		NULL,
		NULL
	);
	buf[nNewLen]='\0';

	//結果
	if(pnMbLength)*pnMbLength=nNewLen;
	return buf;
}

/*!
	CreateMbString で確保したマルチバイト文字列を解放する
*/
static void DestroyMbString(ACHAR* pMbString)
{
	delete[] pMbString;
}
#endif	// ndef _UNICODE



namespace ApiWrap{



	/*!
		MakeSureDirectoryPathExists の UNICODE 版。
		szDirPath で指定されたすべてのディレクトリを作成します。
		ディレクトリの記述は、ルートから開始します。

		@param DirPath
			有効なパス名を指定する、null で終わる文字列へのポインタを指定します。
			パスの最後のコンポーネントがファイル名ではなくディレクトリである場合、
			文字列の最後に円記号（\）を記述しなければなりません。 

		@returns
			関数が成功すると、TRUE が返ります。
			関数が失敗すると、FALSE が返ります。

		@note
			指定された各ディレクトリがまだ存在しない場合、それらのディレクトリを順に作成します。
			一部のディレクトリのみを作成した場合、この関数は FALSE を返します。

		@author
			kobake

		@date
			2007.10.15
	*/
	BOOL MakeSureDirectoryPathExistsW(LPCWSTR szDirPath)
	{
		const wchar_t* p=szDirPath-1;
		for (;;) {
			p=wcschr(p+1,L'\\');
			if(!p)break; //'\\'を走査し終わったので終了

			//先頭からpまでの部分文字列 -> szBuf
			wchar_t szBuf[_MAX_PATH];
			wcsncpy_s(szBuf,_countof(szBuf),szDirPath,p-szDirPath);

			//存在するか
			int nAcc = _waccess(szBuf,0);
			if(nAcc==0)continue; //存在するなら、次へ

			//ディレクトリ作成
			int nDir = _wmkdir(szBuf);
			if(nDir==-1)return FALSE; //エラーが発生したので、FALSEを返す
		}
		return TRUE;
	}




	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//              W系描画API (ANSI版でも利用可能)                //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	/*!
		ANSI版でも使えるExtTextOutW_AnyBuild。
		文字数制限1024半角文字。(文字間隔配列を1024半角文字分しか用意していないため)
	*/
#ifdef _UNICODE
#else
	BOOL ExtTextOutW_AnyBuild(
		HDC				hdc,
		int				x,
		int				y,
		UINT			fuOptions,
		const RECT*		lprc,
		LPCWSTR			lpwString,
		UINT			cbCount,
		const int*		lpDx
	)
	{
		if(lpwString==NULL || *lpwString==L'\0')return FALSE;
		if(cbCount>1024)return FALSE;

		int nNewLength=0;
		//ANSI文字列を生成
		ACHAR* pNewString = CreateMbString(
			lpwString,
			cbCount==-1?wcslen(lpwString):cbCount,
			&nNewLength
		);

		//文字間隔配列を生成
		int nHankakuDx;
		const int* lpDxNew=NULL;
		if(lpDx){
			if(WCODE::IsHankaku(lpwString[0]))nHankakuDx=lpDx[0];
			else nHankakuDx=lpDx[0]/2;
			static int aDx[1024]={0}; //1024半角文字まで
			if(aDx[0]!=nHankakuDx){
				for(int i=0;i<_countof(aDx);i++){
					aDx[i]=nHankakuDx;
				}
			}
			lpDxNew=aDx;
		}

		//APIコール
		BOOL ret=::ExtTextOut(hdc,x,y,fuOptions,lprc,pNewString,nNewLength,lpDxNew);

		//後始末
		DestroyMbString(pNewString);
		DEBUG_SETPIXEL(hdc);
		return ret;
	}
#endif

#ifdef _UNICODE
#else
	BOOL TextOutW_AnyBuild(
		HDC		hdc,
		int		nXStart,
		int		nYStart,
		LPCWSTR	lpwString,
		int		cbString
	)
	{
		int nNewLength=0;
		ACHAR* pNewString = CreateMbString(
			lpwString,
			cbString==-1?wcslen(lpwString):cbString,
			&nNewLength
		);
		BOOL ret=::TextOut(hdc,nXStart,nYStart,pNewString,nNewLength);
		DestroyMbString(pNewString);
		DEBUG_SETPIXEL(hdc);
		return ret;
	}
#endif


	LPWSTR CharNextW_AnyBuild(
		LPCWSTR lpsz
	)
	{
		//$$ サロゲートペア無視
		if(*lpsz)return const_cast<LPWSTR>(lpsz+1);
		else return const_cast<LPWSTR>(lpsz);
	}

	LPWSTR CharPrevW_AnyBuild(
		LPCWSTR lpszStart,
		LPCWSTR lpszCurrent
	)
	{
		//$$ サロゲートペア無視
		if(lpszCurrent>lpszStart)return const_cast<LPWSTR>(lpszCurrent-1);
		else return const_cast<LPWSTR>(lpszStart);
	}

#if 1
	BOOL GetTextExtentPoint32W_AnyBuild(
		HDC		hdc, 
		LPCWSTR	lpString, 
		int		cbString, 
		LPSIZE	lpSize
	)
	{
		vector<char> buf;
		wcstombs_vector(lpString,cbString,&buf);
		return GetTextExtentPoint32A(
			hdc,
			&buf[0],
			buf.size()-1,
			lpSize
		);
	}
#endif

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//             その他W系API (ANSI版でも利用可能)               //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

#ifdef _UNICODE
#else
	int LoadStringW_AnyBuild(
		HINSTANCE	hInstance,
		UINT		uID,
		LPWSTR		lpBuffer,
		int			nBufferCount	//!< バッファのサイズ。文字単位。
	)
	{
		//まずはACHARでロード
		int nTmpCnt = nBufferCount*2+2;
		ACHAR* pTmp = new ACHAR[nTmpCnt];
		int ret=LoadStringA(hInstance, uID, pTmp, nTmpCnt);

		//WCHARに変換
		mbstowcs2(lpBuffer, pTmp, nBufferCount);
		int ret2=wcslen(lpBuffer);

		//後始末
		delete[] pTmp;

		//結果
		return ret2;
	}
#endif


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                    描画API 不具合ラップ                     //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	/*
		VistaでSetPixelが動かないため、代替関数を用意。

		参考：http://forums.microsoft.com/MSDN-JA/ShowPost.aspx?PostID=3228018&SiteID=7
		> Vista で Aero を OFF にすると SetPixel がうまく動かないそうです。
		> しかも、SP1 でも修正されていないとか。
	*/
	void SetPixelSurely(HDC hdc,int x,int y,COLORREF c)
	{
		if (!IsWinVista_or_later()) {
		//Vistaより前：SetPixel直呼び出し
			::SetPixel(hdc,x,y,c);
		}
		else {
		//Vista以降：SetPixelエミュレート
			static HPEN hPen = NULL;
			static COLORREF clrPen = 0;
			if(hPen && c!=clrPen){
				DeleteObject(hPen);
				hPen = NULL;
			}
			//ペン生成
			if(!hPen){
				hPen = CreatePen(PS_SOLID,1,clrPen = c);
			}
			//描画
			HPEN hpnOld = (HPEN)SelectObject(hdc,hPen);
			::MoveToEx(hdc,x,y,NULL);
			::LineTo(hdc,x+1,y+1);
			SelectObject(hdc,hpnOld);
		}
	}
}
