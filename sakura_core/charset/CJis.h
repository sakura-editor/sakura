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
#ifndef SAKURA_CJIS_3DD8E095_FA6C_46BA_BDD8_00F658818D72_H_
#define SAKURA_CJIS_3DD8E095_FA6C_46BA_BDD8_00F658818D72_H_

#include "CCodeBase.h"

class CJis : public CCodeBase{
public:
	CJis(bool base64decode = true) : m_base64decode(base64decode) { }
public:
	//CCodeBase�C���^�[�t�F�[�X
	EConvertResult CodeToUnicode(const CMemory& cSrc, CNativeW* pDst){ *pDst->_GetMemory()=cSrc; return JISToUnicode(pDst->_GetMemory(),m_base64decode); }	//!< ����R�[�h �� UNICODE    �ϊ�
	EConvertResult UnicodeToCode(const CNativeW& cSrc, CMemory* pDst){ *pDst=*cSrc._GetMemory(); return UnicodeToJIS(pDst); }	//!< UNICODE    �� ����R�[�h �ϊ�
// GetEol��CCodeBase�Ɉړ�	2010/6/13 Uchi
	EConvertResult UnicodeToHex(const wchar_t* cSrc, const int iSLen, TCHAR* pDst, const CommonSetting_Statusbar* psStatusbar);			//!< UNICODE �� Hex �ϊ�

public:
	//����
	static EConvertResult JISToUnicode(CMemory* pMem, bool base64decode = true);	// E-Mail(JIS��Unicode)�R�[�h�ϊ�	//2007.08.13 kobake �ǉ�
	static EConvertResult UnicodeToJIS(CMemory* pMem);		// Unicode   �� JIS�R�[�h�ϊ�

protected:
	// 2008.11.10  �ϊ����W�b�N����������
	static int _JisToUni_block( const unsigned char*, const int, unsigned short*, const EMyJisEscseq, bool* pbError );
	static int JisToUni( const char*, const int, wchar_t*, bool* pbError );
	static int _SjisToJis_char( const unsigned char*, unsigned char*, const ECharSet, bool* pbError );
	static int UniToJis( const wchar_t*, const int, char*, bool* pbError );

private:
	//�ϊ����j
	bool m_base64decode;

public:
	//�e�픻��萔
	// JIS �R�[�h�̃G�X�P�[�v�V�[�P���X������f�[�^
	static const char JISESCDATA_ASCII7[];
	static const char JISESCDATA_JISX0201Latin[];
	static const char JISESCDATA_JISX0201Latin_OLD[];
	static const char JISESCDATA_JISX0201Katakana[];
	static const char JISESCDATA_JISX0208_1978[];
	static const char JISESCDATA_JISX0208_1983[];
	static const char JISESCDATA_JISX0208_1990[];
//	static const int TABLE_JISESCLEN[];
//	static const char* TABLE_JISESCDATA[];
};


#if 0 // codechecker.h �ɒ�`����Ă���
/*! JIS �R�[�h�̃G�X�P�[�v�V�[�P���X���� */
/*
	�����������W��       16�i�\��            ������\��[*1]
	------------------------------------------------------------
	JIS C 6226-1978      1b 24 40            ESC $ @
	JIS X 0208-1983      1b 24 42            ESC $ B
	JIS X 0208-1990      1b 26 40 1b 24 42   ESC & @ ESC $ B
	JIS X 0212-1990      1b 24 28 44         ESC $ ( D
	JIS X 0213:2000 1��  1b 24 28 4f         ESC $ ( O
	JIS X 0213:2004 1��  1b 24 28 51         ESC $ ( Q
	JIS X 0213:2000 2��  1b 24 28 50         ESC $ ( P
	JIS X 0201 ���e��    1b 28 4a            ESC ( J
	JIS X 0201 ���e��    1b 28 48            ESC ( H         (���j�I[*2])
	JIS X 0201 �Љ���    1b 28 49            ESC ( I
	ISO/IEC 646 IRV      1b 28 42            ESC ( B
	
	
	  [*1] �e�o�C�g��֋X�I��ISO/IEC 646 IRV�̕����ŕ\�������́B
	       ������ESC�̓o�C�g�l1b��\���B
	
	  [*2] JIS X 0201�̎w���Ƃ��Ă͎g�p���ׂ��łȂ����A�Â��f�[�^�ł�
	       �g���Ă���\��������B
	
	�o�W�Fhttp://www.asahi-net.or.jp/~wq6k-yn/code/
	�Q�l�Fhttp://homepage2.nifty.com/zaco/code/
*/
enum EJisESCSeqType {
	JISESC_UNKNOWN,
	JISESC_ASCII,
	JISESC_JISX0201Latin,
	JISESC_JISX0201Latin_OLD,
	JISESC_JISX0201Katakana,
	JISESC_JISX0208_1978,
	JISESC_JISX0208_1983,
	JISESC_JISX0208_1990,
};

#endif

#endif /* SAKURA_CJIS_3DD8E095_FA6C_46BA_BDD8_00F658818D72_H_ */
/*[EOF]*/
