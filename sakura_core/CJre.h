//	$Id$
//	Copyright (C) 1998-2000, Norio Nakatani

#ifndef __JRE_H
#define __JRE_H

//class CMemory;
#include "CMemory.h"
#include "jreusr.h"

//
// JRE���ȒP�Ɏg�����߂̃N���X
//
class SAKURA_CORE_API CJre
{
public:
	CJre();
	~CJre();

	// �������A���̃����o�֐����ĂԑO�ɕK�����s����悤�ɁB
	BOOL Init();

	// JreGetVersion
	BOOL GetVersion();

	// GetJreMessage
//	CString GetJreMessage(int nLanguage);
	void GetJreMessage(int nLanguage, char* pszMsgBuf );

	// Jre2Compile
	BOOL Compile(LPCTSTR lpszRe);

	// Jre2GetMatchInfo
	LPCTSTR GetMatchInfo(LPCTSTR lpszSt, int, UINT nStart/*=0*/);

	int GetErrorCode() { return m_jreData.nError; }
	int GetLength() { return m_jreData.nLength; }

	// JRE2�\���́ipublic�ɂ���Ȃ�Ă݂��Ƃ��Ȃ��j
	JRE2 m_jreData;
//	const JRE2 *GetJreData();

protected:
	BOOL m_bOpen;
	CMemory m_cmemText;

// static data
private:
	static int m_nUseCount;				// Object�̃J�E���g
	static HINSTANCE m_hDll;			// DLL��Instance
	static LPJRE2OPEN m_lpJre2Open;
	static LPJRE2COMPILE m_lpJre2Compile;
	static LPJRE2GETMATCHINFO m_lpJre2MatchInfo;
	static LPJRE2CLOSE m_lpJre2Close;
	static LPJREGETVERSION m_lpJreGetVersion;
	static LPGETJREMESSAGE m_lpGetJreMessage;

// static function
public:
	static BOOL IsExist();	// jre.dll�����邩�ǂ����𔻒�
};

#endif // __JRE_H

/*[EOF]*/
