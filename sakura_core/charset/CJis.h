#pragma once

#include "CCodeBase.h"
#include "CShiftJis.h"

class CJis : public CCodeBase{
public:
	CJis(bool base64decode = true) : m_base64decode(base64decode) { }
public:
	//CCodeBase�C���^�[�t�F�[�X
	EConvertResult CodeToUnicode(const CMemory& cSrc, CNativeW* pDst){ *pDst->_GetMemory()=cSrc; return JISToUnicode(pDst->_GetMemory(),m_base64decode); }	//!< ����R�[�h �� UNICODE    �ϊ�
	EConvertResult UnicodeToCode(const CNativeW& cSrc, CMemory* pDst){ *pDst=*cSrc._GetMemory(); return UnicodeToJIS(pDst); }	//!< UNICODE    �� ����R�[�h �ϊ�
	void GetEol(CMemory* pcmemEol, EEolType eEolType){ CShiftJis::S_GetEol(pcmemEol,eEolType); }	//!< ���s�f�[�^�擾
	EConvertResult UnicodeToHex(const wchar_t* cSrc, const int iSLen, TCHAR* pDst);			//!< UNICODE �� Hex �ϊ�

public:
	//����
	static EConvertResult JISToUnicode(CMemory* pMem, bool base64decode = true);	// E-Mail(JIS��Unicode)�R�[�h�ϊ�	//2007.08.13 kobake �ǉ�
	static EConvertResult UnicodeToJIS(CMemory* pMem);		// Unicode   �� JIS�R�[�h�ϊ�

	static void SJIStoJIS(CMemory* pMem);		// SJIS��JIS�R�[�h�ϊ�
	static void JIStoSJIS( CMemory* pMem, bool base64decode = true);		// E-Mail(JIS��SJIS)�R�[�h�ϊ�	//Jul. 15, 2001 JEPRO

protected:
	static long MemJIStoSJIS(unsigned char*, long );	/* JIS��SJIS�ϊ� */
	static long MemSJIStoJIS( unsigned char*, long );	/* SJIS��JIS�ϊ� */
	static int StrSJIStoJIS( CMemory*, unsigned char*, int );	/* SJIS��JIS�ŐV�������m�� */
	static long QuotedPrintable_Decode(char*, long );	/* Quoted-Printable�f�R�[�h */

private:
	//�ϊ����j
	bool m_base64decode;

public:
	//�e�픻��萔
	// JIS �R�[�h�̃G�X�P�[�v�V�[�P���X������f�[�^
	static const char JISESCDATA_ASCII[];
	static const char JISESCDATA_JISX0201Latin[];
	static const char JISESCDATA_JISX0201Latin_OLD[];
	static const char JISESCDATA_JISX0201Katakana[];
	static const char JISESCDATA_JISX0208_1978[];
	static const char JISESCDATA_JISX0208_1983[];
	static const char JISESCDATA_JISX0208_1990[];
	static const int TABLE_JISESCLEN[];
	static const char* TABLE_JISESCDATA[];
};



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
