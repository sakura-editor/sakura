#pragma once

//���L���������\����
struct SShare_History{
	//@@@ 2001.12.26 YAZAKI	�ȉ���2�́A���ڃA�N�Z�X���Ȃ��ł��������BCMRU���o�R���Ă��������B
	int					m_nMRUArrNum;
	EditInfo			m_fiMRUArr[MAX_MRU];
	bool				m_bMRUArrFavorite[MAX_MRU];	//���C�ɓ���	//@@@ 2003.04.08 MIK

	//@@@ 2001.12.26 YAZAKI	�ȉ���2�́A���ڃA�N�Z�X���Ȃ��ł��������BCMRUFolder���o�R���Ă��������B
	int								m_nOPENFOLDERArrNum;
	StaticString<TCHAR,_MAX_PATH>	m_szOPENFOLDERArr[MAX_OPENFOLDER];
	bool							m_bOPENFOLDERArrFavorite[MAX_OPENFOLDER];	//���C�ɓ���	//@@@ 2003.04.08 MIK

	//MRU�ȊO�̏��
	SFilePath													m_szIMPORTFOLDER;	// �C���|�[�g�f�B���N�g���̗���
	StaticVector< StaticString<TCHAR, MAX_CMDLEN>, MAX_CMDARR > m_aCommands;		// �O���R�}���h���s����
};
