#pragma once

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ���O                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// �A�v�����B2007.09.21 kobake ����
#ifdef _UNICODE
	#define _APP_NAME_(TYPE) TYPE("sakuraW")
#else
	#define _APP_NAME_(TYPE) TYPE("sakuraA")
#endif

#ifdef _DEBUG
	#define _APP_NAME_2_(TYPE) TYPE("(�f�o�b�O�� ") TYPE(__DATE__) TYPE(")")
#else
	#define _APP_NAME_2_(TYPE) TYPE("(") TYPE(__DATE__) TYPE(")")
#endif

#define _GSTR_APPNAME_(TYPE)  _APP_NAME_(TYPE) _APP_NAME_2_(TYPE) //��:UNICODE�f�o�b�O��_T("sakuraW2(�f�o�b�O��)")

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


