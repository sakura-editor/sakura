/*!	@file
	@brief CRegexKeyword Library

	���K�\���L�[���[�h�������B
	BREGEXP.DLL�𗘗p����B

	@author MIK
	@date Nov. 17, 2001
*/
/*
	Copyright (C) 2001, MIK

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

//@@@ 2001.11.17 add start MIK

//class CRegexKeyword;

#ifndef	_REGEX_KEYWORD_H_
#define	_REGEX_KEYWORD_H_

#include "global.h"
#include "CBregexp.h"
#include "config/maxdata.h" //MAX_REGEX_KEYWORD

struct STypeConfig;

#define USE_PARENT	//�e���g���ăL�[���[�h�i�[�̈���팸����B

/*
 * �p�����[�^�錾
 */
#define RK_EMPTY          0      //�������
#define RK_CLOSE          1      //BREGEXP�N���[�Y
#define RK_OPEN           2      //BREGEXP�I�[�v��
#define RK_ACTIVE         3      //�R���p�C���ς�
#define RK_ERROR          9      //�R���p�C���G���[

#define RK_MATCH          4      //�}�b�`����
#define RK_NOMATCH        5      //���̍s�ł̓}�b�`���Ȃ�

#define RK_SIZE           100    //�ő�o�^�\��

//#define RK_HEAD_CHAR      '^'    //�s�擪�̐��K�\��
#define RK_HEAD_STR1      L"/^"   //BREGEXP
#define RK_HEAD_STR1_LEN  2
#define RK_HEAD_STR2      L"m#^"  //BREGEXP
#define RK_HEAD_STR2_LEN  3
#define RK_HEAD_STR3      L"m/^"  //BREGEXP
#define RK_HEAD_STR3_LEN  3
//#define RK_HEAD_STR4      "#^"   //BREGEXP
//#define RK_HEAD_STR4_LEN  2

#define RK_KAKOMI_1_START "/"
#define RK_KAKOMI_1_END   "/k"
#define RK_KAKOMI_2_START "m#"
#define RK_KAKOMI_2_END   "#k"
#define RK_KAKOMI_3_START "m/"
#define RK_KAKOMI_3_END   "/k"
//#define RK_KAKOMI_4_START "#"
//#define RK_KAKOMI_4_END   "#k"



//@@@ 2001.11.17 add start MIK
struct RegexKeywordInfo {
	wchar_t	m_szKeyword[100];	//���K�\���L�[���[�h
	int	m_nColorIndex;		//�F�w��ԍ�
};
//@@@ 2001.11.17 add end MIK


//!	���K�\���L�[���[�h�������\����
typedef struct RegexInfo_t {
	BREGEXP_W	*pBregexp;	//BREGEXP_W�\����
#ifdef USE_PARENT
#else
	struct RegexKeywordInfo	sRegexKey;	//�R���p�C���p�^�[����ێ�
#endif
	int    nStatus;		//���(EMPTY,CLOSE,OPEN,ACTIVE,ERROR)
	int    nMatch;		//���̃L�[���[�h�̃}�b�`���(EMPTY,MATCH,NOMATCH)
	int    nOffset;		//�}�b�`�����ʒu
	int    nLength;		//�}�b�`��������
	int    nHead;		//�擪�̂݃`�F�b�N���邩�H
	int    nFlag;           //�F�w��̃`�F�b�N�������Ă��邩�H YES=RK_EMPTY, NO=RK_NOMATCH
} REGEX_INFO;



//!	���K�\���L�[���[�h�N���X
/*!
	���K�\���L�[���[�h�������B
*/
class SAKURA_CORE_API CRegexKeyword : public CBregexp {
public:
	CRegexKeyword(LPCTSTR);
	~CRegexKeyword();

	//! �s�����J�n
	BOOL RegexKeyLineStart( void );
	//! �s����
	BOOL RegexIsKeyword( const CStringRef& cStr, int nPos, int *nMatchLen, int *nMatchColor );
	//! �^�C�v�ݒ�
	BOOL RegexKeySetTypes( STypeConfig *pTypesPtr );
	//! ����(�͂�)�`�F�b�N
	BOOL RegexKeyCheckSyntax( const wchar_t *s );



protected:
	//! �R���p�C��
	BOOL RegexKeyCompile(void);
	//! �ϐ�������
	BOOL RegexKeyInit( void );


public:
	int				m_nTypeIndex;				//���݂̃^�C�v�ݒ�ԍ�
	bool			m_bUseRegexKeyword;			//���K�\���L�[���[�h���g�p����E���Ȃ�

private:
	STypeConfig*	m_pTypes;					//�^�C�v�ݒ�ւ̃|�C���^(�Ăяo�����������Ă������)
	int				m_nCompiledMagicNumber;		//�R���p�C���ς݂��H
	int				m_nRegexKeyCount;			//���݂̃L�[���[�h��
	REGEX_INFO		m_sInfo[MAX_REGEX_KEYWORD];	//�L�[���[�h�ꗗ(BREGEXP�R���p�C���Ώ�)
	wchar_t			m_szMsg[256];				//!< BREGEXP_W����̃��b�Z�[�W��ێ�����
};

#endif	//_REGEX_KEYWORD_H_

//@@@ 2001.11.17 add end MIK


