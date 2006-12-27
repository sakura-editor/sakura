/*!	@file
	@brief �����񋤒ʒ�`

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, MIK, Stonee, jepro
	Copyright (C) 2002, KK
	Copyright (C) 2003, MIK
	Copyright (C) 2005, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "stdafx.h"
#include "global.h"


#ifdef _DEBUG
	const char* GSTR_APPNAME = "sakura(�f�o�b�O��)";
#else
	const char* GSTR_APPNAME = "sakura";
#endif


const char* gm_pszCodeNameArr_1[] = {
	"SJIS",				/* SJIS */
	"JIS",				/* JIS */
	"EUC",				/* EUC */
	"Unicode",			/* Unicode */
	"UTF-8",			/* UTF-8 */
	"UTF-7",			/* UTF-7 */
	"UniBE"				/* Unicode BigEndian */
};

const char* gm_pszCodeNameArr_2[] = {
	"SJIS",				/* SJIS */
	"JIS",				/* JIS */
	"EUC",				/* EUC */
	"Uni",				/* Unicode */
	"UTF-8",			/* UTF-8 */
	"UTF-7",			/* UTF-7 */
	"UniBE"				/* Unicode BigEndian */
};

const char* gm_pszCodeNameArr_3[] = {
	"  [SJIS]",			/* SJIS */
	"  [JIS]",			/* JIS */
	"  [EUC]",			/* EUC */
	"  [Unicode]",		/* Unicode */
	"  [UTF-8]",		/* UTF-8 */
	"  [UTF-7]",		/* UTF-7 */
	"  [UniBE]"			/* Unicode BigEndian */
};

const int gm_nCodeComboValueArr[] = {
	CODE_AUTODETECT,	/* �����R�[�h�������� */
	CODE_SJIS,
	CODE_JIS,
	CODE_EUC,
	CODE_UNICODE,
	CODE_UNICODEBE,
	CODE_UTF8,
	CODE_UTF7
};
const char* const	gm_pszCodeComboNameArr[] = {
	"�����I��",
	"SJIS",
	"JIS",
	"EUC",
	"Unicode",
	"UnicodeBE",
	"UTF-8",
	"UTF-7"
};

const int gm_nCodeComboNameArrNum = sizeof( gm_nCodeComboValueArr ) / sizeof( gm_nCodeComboValueArr[0] );


/*! �I��̈�`��p�p�����[�^ */
const COLORREF	SELECTEDAREA_RGB = RGB( 255, 255, 255 );
const int		SELECTEDAREA_ROP2 = R2_XORPEN;

/*! �s�I�[�q�̔z�� */
const enumEOLType gm_pnEolTypeArr[EOL_TYPE_NUM] = {
	EOL_NONE			,	// == 0
	EOL_CRLF			,	// == 2
	EOL_LFCR			,	// == 2
	EOL_LF				,	// == 1
	EOL_CR					// == 1
};

/*! �L�[���[�h�L�����N�^ */
const unsigned char gm_keyword_char[256] = {
 /* 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F      : 0123456789ABCDEF */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* 0: ................ */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* 1: ................ */
	0, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* 2:  !"#$%&'()*+,-./ */
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,	/* 3: 0123456789:;<=>? */
	2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,	/* 4: @ABCDEFGHIJKLMNO */
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 2, 0, 0, 1,	/* 5: PQRSTUVWXYZ[\]^_ */
	0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,	/* 6: `abcdefghijklmno */
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,	/* 7: pqrstuvwxyz{|}~. */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* 8: ................ */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* 9: ................ */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* A: .��������������� */	//setlocal( LC_ALL, "C" )
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* B: ���������������� */	//setlocal( LC_ALL, "C" )
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* C: ���������������� */	//setlocal( LC_ALL, "C" )
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* D: ���������������� */	//setlocal( LC_ALL, "C" )
//	0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,	/* A: .��������������� */	//setlocal( LC_ALL, "Japanese" )
//	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,	/* B: ���������������� */	//setlocal( LC_ALL, "Japanese" )
//	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,	/* C: ���������������� */	//setlocal( LC_ALL, "Japanese" )
//	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,	/* D: ���������������� */	//setlocal( LC_ALL, "Japanese" )
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* E: ................ */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* F: ................ */
	/* 0: not-keyword, 1:__iscsym(), 2:user-define */
};

/*!
  ini�̐F�ݒ��ԍ��łȂ�������ŏ����o���B(added by Stonee, 2001/01/12, 2001/01/15)
  �z��̏��Ԃ͋��L���������̃f�[�^�̏��Ԃƈ�v���Ă���B

  @note ���l�ɂ������I�Ή���global.h�ōs���Ă���̂ŎQ�Ƃ̂��ƁB(Mar. 7, 2001 jepro)
  CShareData����global�Ɉړ�
*/
const ColorAttributeData g_ColorAttributeArr[] =
{
	{"TXT", COLOR_ATTRIB_FORCE_DISP | COLOR_ATTRIB_NO_EFFECTS},
	{"RUL", COLOR_ATTRIB_NO_EFFECTS},
	{"CAR", COLOR_ATTRIB_FORCE_DISP | COLOR_ATTRIB_NO_BACK | COLOR_ATTRIB_NO_EFFECTS},	// �L�����b�g		// 2006.12.07 ryoji
	{"IME", COLOR_ATTRIB_NO_BACK | COLOR_ATTRIB_NO_EFFECTS},	// IME�L�����b�g	// 2006.12.07 ryoji
	{"UND", COLOR_ATTRIB_NO_BACK | COLOR_ATTRIB_NO_EFFECTS},
	{"LNO", 0},
	{"MOD", 0},
	{"TAB", 0},
	{"SPC", 0},	//2002.04.28 Add By KK
	{"ZEN", 0},
	{"CTL", 0},
	{"EOL", 0},
	{"RAP", 0},
	{"VER", 0},  // 2005.11.08 Moca �w�茅�c��
	{"EOF", 0},
	{"NUM", 0},	//@@@ 2001.02.17 by MIK ���p���l�̋���
	{"FND", 0},
	{"KW1", 0},
	{"KW2", 0},
	{"KW3", 0},	//@@@ 2003.01.13 by MIK �����L�[���[�h3-10
	{"KW4", 0},
	{"KW5", 0},
	{"KW6", 0},
	{"KW7", 0},
	{"KW8", 0},
	{"KW9", 0},
	{"KWA", 0},
	{"CMT", 0},
	{"SQT", 0},
	{"WQT", 0},
	{"URL", 0},
	{"RK1", 0},	//@@@ 2001.11.17 add MIK
	{"RK2", 0},	//@@@ 2001.11.17 add MIK
	{"RK3", 0},	//@@@ 2001.11.17 add MIK
	{"RK4", 0},	//@@@ 2001.11.17 add MIK
	{"RK5", 0},	//@@@ 2001.11.17 add MIK
	{"RK6", 0},	//@@@ 2001.11.17 add MIK
	{"RK7", 0},	//@@@ 2001.11.17 add MIK
	{"RK8", 0},	//@@@ 2001.11.17 add MIK
	{"RK9", 0},	//@@@ 2001.11.17 add MIK
	{"RKA", 0},	//@@@ 2001.11.17 add MIK
	{"DFA", 0},	//DIFF�ǉ�	//@@@ 2002.06.01 MIK
	{"DFC", 0},	//DIFF�ύX	//@@@ 2002.06.01 MIK
	{"DFD", 0},	//DIFF�폜	//@@@ 2002.06.01 MIK
	{"BRC", 0},	//�Ί���	// 02/09/18 ai Add
	{"MRK", 0},	//�u�b�N�}�[�N	// 02/10/16 ai Add
	{"LAST", 0}	// Not Used
};


/*[EOF]*/
