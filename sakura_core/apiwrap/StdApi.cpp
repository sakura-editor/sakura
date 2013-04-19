#include "StdAfx.h"
#include <vector>
#include "StdApi.h"
#include "charset/charcode.h"
#include "_os/COsVersionInfo.h"

using namespace std;

#ifndef _UNICODE
/*!
	���C�h�����񂩂�}���`�o�C�g������𐶐�����B
	�}���`�o�C�g������̂��߂ɐV�����������̈悪�m�ۂ����̂ŁA
	�g���I�������DestroyMbString���ĂԂ��ƁI

	@retval �ϊ����ꂽACHAR������
*/
static ACHAR* CreateMbString(
	const WCHAR*	pWideString,	//!< [in]  ����WCHAR������
	int				nWideLength,	//!< [in]  ����WCHAR������̒����B�����P�ʁB
	int*			pnMbLength		//!< [out] �ϊ����ꂽACHAR������̒����̎󂯎���B�����P�ʁB
)
{
	//�K�v�ȗ̈�T�C�Y���擾
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

	//�̈���m��
	ACHAR* buf=new ACHAR[nNewLen+1];

	//�ϊ�
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

	//����
	if(pnMbLength)*pnMbLength=nNewLen;
	return buf;
}

/*!
	CreateMbString �Ŋm�ۂ����}���`�o�C�g��������������
*/
static void DestroyMbString(ACHAR* pMbString)
{
	delete[] pMbString;
}
#endif	// ndef _UNICODE



namespace ApiWrap{



	/*!
		MakeSureDirectoryPathExists �� UNICODE �ŁB
		szDirPath �Ŏw�肳�ꂽ���ׂẴf�B���N�g�����쐬���܂��B
		�f�B���N�g���̋L�q�́A���[�g����J�n���܂��B

		@param DirPath
			�L���ȃp�X�����w�肷��Anull �ŏI��镶����ւ̃|�C���^���w�肵�܂��B
			�p�X�̍Ō�̃R���|�[�l���g���t�@�C�����ł͂Ȃ��f�B���N�g���ł���ꍇ�A
			������̍Ō�ɉ~�L���i\�j���L�q���Ȃ���΂Ȃ�܂���B 

		@returns
			�֐�����������ƁATRUE ���Ԃ�܂��B
			�֐������s����ƁAFALSE ���Ԃ�܂��B

		@note
			�w�肳�ꂽ�e�f�B���N�g�����܂����݂��Ȃ��ꍇ�A�����̃f�B���N�g�������ɍ쐬���܂��B
			�ꕔ�̃f�B���N�g���݂̂��쐬�����ꍇ�A���̊֐��� FALSE ��Ԃ��܂��B

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
			if(!p)break; //'\\'�𑖍����I������̂ŏI��

			//�擪����p�܂ł̕��������� -> szBuf
			wchar_t szBuf[_MAX_PATH];
			wcsncpy_s(szBuf,_countof(szBuf),szDirPath,p-szDirPath);

			//���݂��邩
			int nAcc = _waccess(szBuf,0);
			if(nAcc==0)continue; //���݂���Ȃ�A����

			//�f�B���N�g���쐬
			int nDir = _wmkdir(szBuf);
			if(nDir==-1)return FALSE; //�G���[�����������̂ŁAFALSE��Ԃ�
		}
		return TRUE;
	}




	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//              W�n�`��API (ANSI�łł����p�\)                //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	/*!
		ANSI�łł��g����ExtTextOutW_AnyBuild�B
		����������1024���p�����B(�����Ԋu�z���1024���p�����������p�ӂ��Ă��Ȃ�����)
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
		//ANSI������𐶐�
		ACHAR* pNewString = CreateMbString(
			lpwString,
			cbCount==-1?wcslen(lpwString):cbCount,
			&nNewLength
		);

		//�����Ԋu�z��𐶐�
		int nHankakuDx;
		const int* lpDxNew=NULL;
		if(lpDx){
			if(WCODE::IsHankaku(lpwString[0]))nHankakuDx=lpDx[0];
			else nHankakuDx=lpDx[0]/2;
			static int aDx[1024]={0}; //1024���p�����܂�
			if(aDx[0]!=nHankakuDx){
				for(int i=0;i<_countof(aDx);i++){
					aDx[i]=nHankakuDx;
				}
			}
			lpDxNew=aDx;
		}

		//API�R�[��
		BOOL ret=::ExtTextOut(hdc,x,y,fuOptions,lprc,pNewString,nNewLength,lpDxNew);

		//��n��
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
		//$$ �T���Q�[�g�y�A����
		if(*lpsz)return const_cast<LPWSTR>(lpsz+1);
		else return const_cast<LPWSTR>(lpsz);
	}

	LPWSTR CharPrevW_AnyBuild(
		LPCWSTR lpszStart,
		LPCWSTR lpszCurrent
	)
	{
		//$$ �T���Q�[�g�y�A����
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
	//             ���̑�W�nAPI (ANSI�łł����p�\)               //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

#ifdef _UNICODE
#else
	int LoadStringW_AnyBuild(
		HINSTANCE	hInstance,
		UINT		uID,
		LPWSTR		lpBuffer,
		int			nBufferCount	//!< �o�b�t�@�̃T�C�Y�B�����P�ʁB
	)
	{
		//�܂���ACHAR�Ń��[�h
		int nTmpCnt = nBufferCount*2+2;
		ACHAR* pTmp = new ACHAR[nTmpCnt];
		int ret=LoadStringA(hInstance, uID, pTmp, nTmpCnt);

		//WCHAR�ɕϊ�
		mbstowcs2(lpBuffer, pTmp, nBufferCount);
		int ret2=wcslen(lpBuffer);

		//��n��
		delete[] pTmp;

		//����
		return ret2;
	}
#endif


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                    �`��API �s����b�v                     //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	/*
		Vista��SetPixel�������Ȃ����߁A��֊֐���p�ӁB

		�Q�l�Fhttp://forums.microsoft.com/MSDN-JA/ShowPost.aspx?PostID=3228018&SiteID=7
		> Vista �� Aero �� OFF �ɂ���� SetPixel �����܂������Ȃ������ł��B
		> �������ASP1 �ł��C������Ă��Ȃ��Ƃ��B
	*/
	void SetPixelSurely(HDC hdc,int x,int y,COLORREF c)
	{
		if (!IsWinVista_or_later()) {
		//Vista���O�FSetPixel���Ăяo��
			::SetPixel(hdc,x,y,c);
		}
		else {
		//Vista�ȍ~�FSetPixel�G�~�����[�g
			static HPEN hPen = NULL;
			static COLORREF clrPen = 0;
			if(hPen && c!=clrPen){
				DeleteObject(hPen);
				hPen = NULL;
			}
			//�y������
			if(!hPen){
				hPen = CreatePen(PS_SOLID,1,clrPen = c);
			}
			//�`��
			HPEN hpnOld = (HPEN)SelectObject(hdc,hPen);
			::MoveToEx(hdc,x,y,NULL);
			::LineTo(hdc,x+1,y+1);
			SelectObject(hdc,hpnOld);
		}
	}
}
