/*
	Copyright (C) 2008, kobake

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
#ifndef SAKURA_WINDOW_E7B899CD_2106_4A3B_BBA1_EB29FD9640F39_H_
#define SAKURA_WINDOW_E7B899CD_2106_4A3B_BBA1_EB29FD9640F39_H_

/*!
	@brief ��� DPI �X�P�[�����O
	@note 96 DPI �s�N�Z����z�肵�Ă���f�U�C�����ǂꂾ���X�P�[�����O���邩

	@date 2009.10.01 ryoji ��DPI�Ή��p�ɍ쐬
*/
class CDPI{
	static void Init()
	{
		if( !bInitialized )
		{
			HDC hDC = GetDC(NULL);
			nDpiX = GetDeviceCaps(hDC, LOGPIXELSX);
			nDpiY = GetDeviceCaps(hDC, LOGPIXELSY);
			ReleaseDC(NULL, hDC);
			bInitialized = true;
		}
	}
	static int nDpiX;
	static int nDpiY;
	static bool bInitialized;
public:
	static int ScaleX(int x){Init(); return ::MulDiv(x, nDpiX, 96);}
	static int ScaleY(int y){Init(); return ::MulDiv(y, nDpiY, 96);}
	static int UnscaleX(int x){Init(); return ::MulDiv(x, 96, nDpiX);}
	static int UnscaleY(int y){Init(); return ::MulDiv(y, 96, nDpiY);}
	static void ScaleRect(LPRECT lprc)
	{
		lprc->left = ScaleX(lprc->left);
		lprc->right = ScaleX(lprc->right);
		lprc->top = ScaleY(lprc->top);
		lprc->bottom = ScaleY(lprc->bottom);
	}
	static void UnscaleRect(LPRECT lprc)
	{
		lprc->left = UnscaleX(lprc->left);
		lprc->right = UnscaleX(lprc->right);
		lprc->top = UnscaleY(lprc->top);
		lprc->bottom = UnscaleY(lprc->bottom);
	}
	static int PointsToPixels(int pt, int ptMag = 1){Init(); return ::MulDiv(pt, nDpiY, 72 * ptMag);}	// ptMag: �����̃|�C���g���ɂ������Ă���{��
	static int PixelsToPoints(int px, int ptMag = 1){Init(); return ::MulDiv(px * ptMag, 72, nDpiY);}	// ptMag: �߂�l�̃|�C���g���ɂ�����{��
};

inline int DpiScaleX(int x){return CDPI::ScaleX(x);}
inline int DpiScaleY(int y){return CDPI::ScaleY(y);}
inline int DpiUnscaleX(int x){return CDPI::UnscaleX(x);}
inline int DpiUnscaleY(int y){return CDPI::UnscaleY(y);}
inline void DpiScaleRect(LPRECT lprc){CDPI::ScaleRect(lprc);}
inline void DpiUnscaleRect(LPRECT lprc){CDPI::UnscaleRect(lprc);}
inline int DpiPointsToPixels(int pt, int ptMag = 1){return CDPI::PointsToPixels(pt, ptMag);}
inline int DpiPixelsToPoints(int px, int ptMag = 1){return CDPI::PixelsToPoints(px, ptMag);}

SAKURA_CORE_API void ActivateFrameWindow( HWND );	/* �A�N�e�B�u�ɂ��� */

/*
||	�������̃��[�U�[������\�ɂ���
||	�u���b�L���O�t�b�N(?)(���b�Z�[�W�z��)
*/
SAKURA_CORE_API BOOL BlockingHook( HWND hwndDlgCancel );


#ifndef GA_PARENT
#define GA_PARENT		1
#define GA_ROOT			2
#define GA_ROOTOWNER	3
#endif
#define GA_ROOTOWNER2	100


HWND MyGetAncestor( HWND hWnd, UINT gaFlags );	// �w�肵���E�B���h�E�̑c��̃n���h�����擾����	// 2007.07.01 ryoji


//�`�F�b�N�{�b�N�X
inline void CheckDlgButtonBool(HWND hDlg, int nIDButton, bool bCheck)
{
	CheckDlgButton(hDlg,nIDButton,bCheck?BST_CHECKED:BST_UNCHECKED);
}
inline bool IsDlgButtonCheckedBool(HWND hDlg, int nIDButton)
{
	return (IsDlgButtonChecked(hDlg,nIDButton) & BST_CHECKED) != 0;
}

//�_�C�A���O�A�C�e���̗L����
inline bool DlgItem_Enable(HWND hwndDlg, int nIDDlgItem, bool nEnable)
{
	return FALSE != ::EnableWindow( ::GetDlgItem(hwndDlg, nIDDlgItem), nEnable?TRUE:FALSE);
}

// ���v�Z�⏕�N���X
// �ő�̕���񍐂��܂�
class CTextWidthCalc
{
public:
	CTextWidthCalc(HWND hParentDlg, int nID);
	CTextWidthCalc(HWND hwndThis);
	CTextWidthCalc(HFONT font);
	virtual ~CTextWidthCalc();
	void Reset(){ nCx = 0; nExt = 0; }
	void SetCx(int cx = 0){ nCx = cx; }
	void SetDefaultExtend(int extCx = 0){ nExt = 0; }
	bool SetWidthIfMax(int width);
	bool SetWidthIfMax(int width, int extCx);
	bool SetTextWidthIfMax(LPCTSTR pszText);
	bool SetTextWidthIfMax(LPCTSTR pszText, int extCx);
	int GetCx(){ return nCx; }
	// �Z�o���@���悭������Ȃ��̂Œ萔�ɂ��Ă���
	// ����s�v�Ȃ� ListView��LVSCW_AUTOSIZE������
	enum StaticMagicNambers{
		//! �X�N���[���o�[�ƃA�C�e���̊Ԃ̌���
		WIDTH_MARGIN_SCROLLBER = 8,
		//! ���X�g�r���[�w�b�_ �}�[�W��
		WIDTH_LV_HEADER = 17,
		//! ���X�g�r���[�̃}�[�W��
		WIDTH_LV_ITEM_NORMAL  = 14,
		//! ���X�g�r���[�̃`�F�b�N�{�b�N�X�ƃ}�[�W���̕�
		WIDTH_LV_ITEM_CHECKBOX = 30,
	};
private:
	HWND  hwnd;
	HDC   hDC;
	HFONT hFont;
	HFONT hFontOld;
	int nCx;
	int nExt;
};

#endif /* SAKURA_WINDOW_E7B899CD_2106_4A3B_BBA1_EB29FD9640F39_H_ */
/*[EOF]*/
