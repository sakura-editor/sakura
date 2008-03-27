#pragma once

class CDocLineMgr;

//as decorator
class CDocReader{
public:
	CDocReader(const CDocLineMgr& pcDocLineMgr) : m_pcDocLineMgr(&pcDocLineMgr) { }

	wchar_t* GetAllData(int* pnDataLen);	/* �S�s�f�[�^��Ԃ� */
	const wchar_t* GetLineStr( CLogicInt , CLogicInt* );
	const wchar_t* GetLineStrWithoutEOL( CLogicInt , int* ); // 2003.06.22 Moca
	const wchar_t* GetFirstLinrStr( int* );	/* ���A�N�Z�X���[�h�F�擪�s�𓾂� */
	const wchar_t* GetNextLinrStr( int* );	/* ���A�N�Z�X���[�h�F���̍s�𓾂� */

private:
	const CDocLineMgr* m_pcDocLineMgr;
};
