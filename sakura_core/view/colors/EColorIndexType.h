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
#ifndef SAKURA_ECOLORINDEXTYPE_H_
#define SAKURA_ECOLORINDEXTYPE_H_

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          �F�萔                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// �F�萔��F�ԍ��ɕϊ����邽�߂̎���bit
#define COLORIDX_BLOCK_BIT (2 << 9)		//!< �u���b�N�R�����g����bit
#define COLORIDX_REGEX_BIT (2 << 10)	//!< ���K�\���L�[���[�h����bit

/*! �F�萔
	@date 2000.01.12 Stonee ������ύX�����Ƃ��́ACColorStrategy.cpp ��g_ColorAttributeArr�̒�`���ύX���ĉ������B
	@date 2000.09.18 JEPRO ���Ԃ�啝�ɓ���ւ���
	@date 2007.09.09 Moca  ���Ԃ̒�`�͂��C���ɕύX
	@date 2013.04.26 novice �F�萔��F�ԍ���ϊ����邽�߂̎���bit����
*/
enum EColorIndexType {
	COLORIDX_TEXT = 0,		//!< �e�L�X�g
	COLORIDX_RULER,			//!< ���[���[
	COLORIDX_CARET,			//!< �L�����b�g	// 2006.12.07 ryoji
	COLORIDX_CARET_IME,		//!< IME�L�����b�g // 2006.12.07 ryoji
	COLORIDX_CARETLINEBG,	//!< �J�[�\���s�w�i�F
	COLORIDX_UNDERLINE,		//!< �J�[�\���s�A���_�[���C��
	COLORIDX_CURSORVLINE,	//!< �J�[�\���ʒu�c�� // 2006.05.13 Moca
	COLORIDX_GYOU,			//!< �s�ԍ�
	COLORIDX_GYOU_MOD,		//!< �s�ԍ�(�ύX�s)
	COLORIDX_TAB,			//!< TAB�L��
	COLORIDX_SPACE,			//!< ���p�� //2002.04.28 Add by KK �ȍ~�S��+1
	COLORIDX_ZENSPACE,		//!< ���{���
	COLORIDX_CTRLCODE,		//!< �R���g���[���R�[�h
	COLORIDX_EOL,			//!< ���s�L��
	COLORIDX_WRAP,			//!< �܂�Ԃ��L��
	COLORIDX_VERTLINE,		//!< �w�茅�c��	// 2005.11.08 Moca
	COLORIDX_EOF,			//!< EOF�L��
	COLORIDX_DIGIT,			//!< ���p���l	 //@@@ 2001.02.17 by MIK //�F�ݒ�Ver.3���烆�[�U�t�@�C���ɑ΂��Ă͕�����ŏ������Ă���̂Ń��i���o�����O���Ă��悢. Mar. 7, 2001 JEPRO noted
	COLORIDX_BRACKET_PAIR,	//!< �Ί���	  // 02/09/18 ai Add
	COLORIDX_SELECT,		//!< �I��͈�
	COLORIDX_SEARCH,		//!< ����������
	COLORIDX_SEARCH2,		//!< ����������2
	COLORIDX_SEARCH3,		//!< ����������3
	COLORIDX_SEARCH4,		//!< ����������4
	COLORIDX_SEARCH5,		//!< ����������5
	COLORIDX_COMMENT,		//!< �s�R�����g						//Dec. 4, 2000 shifted by MIK
	COLORIDX_SSTRING,		//!< �V���O���N�H�[�e�[�V����������	//Dec. 4, 2000 shifted by MIK
	COLORIDX_WSTRING,		//!< �_�u���N�H�[�e�[�V����������		//Dec. 4, 2000 shifted by MIK
	COLORIDX_URL,			//!< URL								//Dec. 4, 2000 shifted by MIK
	COLORIDX_KEYWORD1,		//!< �����L�[���[�h1 // 2002/03/13 novice
	COLORIDX_KEYWORD2,		//!< �����L�[���[�h2 // 2002/03/13 novice  //MIK ADDED
	COLORIDX_KEYWORD3,		//!< �����L�[���[�h3 // 2005.01.13 MIK 3-10 added
	COLORIDX_KEYWORD4,		//!< �����L�[���[�h4
	COLORIDX_KEYWORD5,		//!< �����L�[���[�h5
	COLORIDX_KEYWORD6,		//!< �����L�[���[�h6
	COLORIDX_KEYWORD7,		//!< �����L�[���[�h7
	COLORIDX_KEYWORD8,		//!< �����L�[���[�h8
	COLORIDX_KEYWORD9,		//!< �����L�[���[�h9
	COLORIDX_KEYWORD10,		//!< �����L�[���[�h10
	COLORIDX_REGEX1,		//!< ���K�\���L�[���[�h1  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX2,		//!< ���K�\���L�[���[�h2  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX3,		//!< ���K�\���L�[���[�h3  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX4,		//!< ���K�\���L�[���[�h4  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX5,		//!< ���K�\���L�[���[�h5  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX6,		//!< ���K�\���L�[���[�h6  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX7,		//!< ���K�\���L�[���[�h7  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX8,		//!< ���K�\���L�[���[�h8  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX9,		//!< ���K�\���L�[���[�h9  //@@@ 2001.11.17 add MIK
	COLORIDX_REGEX10,		//!< ���K�\���L�[���[�h10	//@@@ 2001.11.17 add MIK
	COLORIDX_DIFF_APPEND,	//!< DIFF�ǉ�  //@@@ 2002.06.01 MIK
	COLORIDX_DIFF_CHANGE,	//!< DIFF�ǉ�  //@@@ 2002.06.01 MIK
	COLORIDX_DIFF_DELETE,	//!< DIFF�ǉ�  //@@@ 2002.06.01 MIK
	COLORIDX_MARK,			//!< �u�b�N�}�[�N  // 02/10/16 ai Add

	//�J���[�̍Ō�
	COLORIDX_LAST,			//!< �J���[�̍Ō�

	//�J���[�\������p(�u���b�N�R�����g)
	COLORIDX_BLOCK1			= COLORIDX_BLOCK_BIT,			//!< �u���b�N�R�����g1(�����F�Ɣw�i�F�͍s�R�����g�Ɠ���)
	COLORIDX_BLOCK2,										//!< �u���b�N�R�����g2(�����F�Ɣw�i�F�͍s�R�����g�Ɠ���)

	//�J���[�\������p(���K�\���L�[���[�h)
	COLORIDX_REGEX_FIRST	= COLORIDX_REGEX_BIT,						//!< ���K�\���L�[���[�h(�ŏ�)
	COLORIDX_REGEX_LAST		= COLORIDX_REGEX_FIRST + COLORIDX_LAST - 1,	//!< ���K�\���L�[���[�h(�Ō�)

	// -- -- �ʖ� -- -- //
	COLORIDX_DEFAULT		= COLORIDX_TEXT,							//!< �f�t�H���g
	COLORIDX_SEARCHTAIL		= COLORIDX_SEARCH5,
};

#endif
