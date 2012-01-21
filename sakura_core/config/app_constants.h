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
#ifndef SAKURA_APP_CONSTANTS_AD36E2CE_B62E_497D_806F_6B9738310127_H_
#define SAKURA_APP_CONSTANTS_AD36E2CE_B62E_497D_806F_6B9738310127_H_

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ���O                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// �A�v�����B2007.09.21 kobake ����
#ifdef _UNICODE
	#define _APP_NAME_(TYPE) TYPE("sakura")
#else
	#define _APP_NAME_(TYPE) TYPE("sakura")
#endif

#ifdef _DEBUG
	#define _APP_NAME_2_(TYPE) TYPE("(�f�o�b�O��)")
#else
	#define _APP_NAME_2_(TYPE) TYPE("")
#endif

#define _GSTR_APPNAME_(TYPE)  _APP_NAME_(TYPE) _APP_NAME_2_(TYPE) //��:UNICODE�f�o�b�O��_T("sakura(�f�o�b�O��)")

#define GSTR_APPNAME    (_GSTR_APPNAME_(_T)   )
#define GSTR_APPNAME_A  (_GSTR_APPNAME_(ATEXT))
#define GSTR_APPNAME_W  (_GSTR_APPNAME_(LTEXT))


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      �e�L�X�g�G���A                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// Feb. 18, 2003 genta �ő�l�̒萔���ƒl�ύX
const int LINESPACE_MAX = 128;
const int COLUMNSPACE_MAX = 64;

//	Aug. 14, 2005 genta �萔��`�ǉ�
// 2007.09.07 kobake �萔���ύX: MAXLINESIZE��MAXLINEKETAS
// 2007.09.07 kobake �萔���ύX: MINLINESIZE��MINLINEKETAS
const int MAXLINEKETAS		= 10240;	//!< 1�s�̌����̍ő�l
const int MINLINEKETAS		= 10;		//!< 1�s�̌����̍ŏ��l

#endif /* SAKURA_APP_CONSTANTS_AD36E2CE_B62E_497D_806F_6B9738310127_H_ */
/*[EOF]*/
