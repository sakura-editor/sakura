//	$Id$
/*!	@file
	CImageListMgr ImageList�������N���X

	@author genta
	@date Oct. 11, 2000 genta
	$Revision$

*/
/*
	Copyright (C) 2000-2001, genta

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/
#include "CImageListMgr.h"
#include "sakura_rc.h"

//	Destructor
CImageListMgr::~CImageListMgr()
{
	if( m_hList != NULL ){
		ImageList_Destroy( m_hList );
	}
}

bool CImageListMgr::Create(HINSTANCE hInstance, HWND hWnd)
{
	if( m_hList != NULL ){	//	���ɍ\�z�ς݂Ȃ疳������
		return true;
	}

	HBITMAP	hRscbmp;	//	���\�[�X����ǂݍ��񂾂ЂƂ����܂��Bitmap
	HBITMAP	hTmpbmp;	//	1�A�C�R��Bitmap�p�̗̈�
	HBITMAP	hFOldbmp;	//	SetObject�œ���ꂽ1�O�̃n���h����ێ�����
	HBITMAP	hTOldbmp;	//	SetObject�œ���ꂽ1�O�̃n���h����ێ�����
	HDC		dcFrom;		//	�`��p
	HDC		dcTo;		//	�`��p
	int		nRetPos;	//	�㏈���p
	m_cx = m_cy  = 16;

//	Oct. 21, 2000 JEPRO �ݒ�
	const int MAX_X = 32;
	const int MAX_Y = 11;

	//	�b��
//	const int MAX_X = 128;
//	const int MAX_Y = 1;

	COLORREF cTransparent;	//	�擾�����摜��(0,0)�̐F��w�i�F�Ƃ��Ďg��

	nRetPos = 0;
	do {
		//	From Here 2001.7.1 GAE
		//	2001.7.1 GAE ���\�[�X�����[�J���t�@�C��(sakura�f�B���N�g��) my_icons.bmp ����ǂ߂�悤��
		char	szPath[_MAX_PATH], szExeDrive[_MAX_DRIVE], szExeDir[_MAX_DIR];

		// sakura.exe �̃p�X���擾
		::GetModuleFileName( ::GetModuleHandle(NULL), szPath, sizeof(szPath) );

		// (sakura�f�B���N�g��) my_icons.bmp �̕�������쐬
		_splitpath( szPath, szExeDrive, szExeDir, NULL, NULL );
		wsprintf( szPath, "%s%s%s", szExeDrive, szExeDir, "my_icons.bmp" );	//Jul.04, 2001 JEPRO toolbar.bmp��my_icons.bmp �ɖ��O�ύX
		hRscbmp = (HBITMAP)::LoadImage( NULL, szPath, IMAGE_BITMAP, 0, 0,
			LR_LOADFROMFILE | LR_CREATEDIBSECTION | LR_LOADMAP3DCOLORS );

		if( hRscbmp == NULL ) {	// ���[�J���t�@�C���̓ǂݍ��ݎ��s���̓��\�[�X����擾
			//	���̃u���b�N���͏]���̏���
			//	���\�[�X����Bitmap��ǂݍ���
			hRscbmp = ::LoadBitmap( hInstance, MAKEINTRESOURCE( IDB_MYTOOL ) );
			if( hRscbmp == NULL ){
				nRetPos = 0;
				break;
			}
		}
		//	To Here 2001.7.1 GAE

		//	BitBlt���g�����߂�MemoryDC�Ƀ}�b�s���O����
		//	MAP������MemoryDC�ɑ΂��ĕ`����s����Bitmap�������������Ă���Ƃ����Z�i�D
		dcFrom = CreateCompatibleDC(0);	//	�]�����p
		if( dcFrom == NULL ){
			nRetPos = 1;
			break;
		}

		//	�܂�bitmap��dc��map����
		//	�������邱�Ƃ�CreateCompatibleBitmap��
		//	hRscbmp�Ɠ����`����bitmap������D
		//	�P��CreateCompatibleDC(0)�Ŏ擾����dc��
		//	�X�N���[����DC�ɑ΂���CreateCompatibleBitmap��
		//	�g���ƃ��m�N��Bitmap�ɂȂ�D
		hFOldbmp = (HBITMAP)SelectObject( dcFrom, hRscbmp );
		if( hFOldbmp == NULL ){
			nRetPos = 4;
			break;
		}

		dcTo = CreateCompatibleDC(dcFrom);	//	�]����p
		if( dcTo == NULL ){
			nRetPos = 2;
			break;
		}
		//	Oct, 23, 2000 genta
		//	������Bitmap�ɂ�����A�C�R�������������Ȃ����̂ŁC
		//	1�s�����������ǉ�������@�ɕύX
		//	1�s����Bitmap���\�z
		hTmpbmp = ::CreateCompatibleBitmap(dcFrom, m_cx * MAX_X, m_cy);
		if( hTmpbmp == NULL ){
			nRetPos = 3;
			break;
		}

		hTOldbmp = (HBITMAP)SelectObject( dcTo, hTmpbmp );
		if( hTOldbmp == NULL ){
			nRetPos = 5;
			break;
		}

		//	�����F�̐ݒ�
		cTransparent = GetPixel( dcFrom, 0, 0 );

		//	Image List�{�̂��쐬
		m_hList = ImageList_Create( m_cx, m_cy, ILC_COLOR |ILC_MASK, MAX_X * MAX_Y, 32 );
		if( m_hList == NULL ){
			nRetPos = 6;
			break;
		}

		int y, py, px;
		for( y = py = 0, px = 0; y < MAX_Y;
			y++, py += m_cy, px += m_cx * MAX_X ){
			::BitBlt( dcTo,	//	Destination
				0, 0,		//	�]����
				m_cx * MAX_X, m_cy,		//	�]���T�C�Y
				dcFrom,		//	Source
				0, py,		//	�]����
				SRCCOPY		//	�P���R�s�[
			);
			SelectObject( dcTo, hTOldbmp );	//	���d�v�I
			//	Bitmap��MemoryDC��Assign����Ă���Ԃ�bitmap�n���h����
			//	�g���Ă�������bitmap���擾�ł��Ȃ��D
			//	�܂�CDC�ւ̕`�施�߂𔭍s���Ă����̏��Bitmap��
			//	���f�����킯�ł͂Ȃ��D
			//	Bitmap��DC������O���ď��߂ē��e�̕ۏ؂��ł���

			//	Oct, 23, 2000 genta
			//	������Bitmap�ɂ�����A�C�R�������������Ȃ����̂ŁC
			//	1�s�����������ǉ�������@�ɕύX
			ImageList_AddMasked( m_hList, hTmpbmp, cTransparent);

			//	MAP������
			SelectObject( dcTo, hTmpbmp );
		}
	}while(0);	//	1�񂵂��ʂ�Ȃ�

	//	�㏈��
	switch( nRetPos ){

	case 6:
		SelectObject( dcTo, hTOldbmp );
	case 0:
	case 5:
		DeleteObject( hTmpbmp );
	case 3:
		//	�E�B���h�E�n���h���Ɋ֘A�Â���ꂽDC(GetDC�Ŏ擾�������́j��ReleaseDC�ŉ�����邪�C
		//	�����łȂ����̂�DeleteDC���g���D
		DeleteDC( dcTo );
	case 2:
		SelectObject( dcFrom, hFOldbmp );
	case 4:
		DeleteDC( dcFrom );
	case 1:
	case 50:
		DeleteObject( hRscbmp );
	}

	return nRetPos == 0;
}


/*[EOF]*/
