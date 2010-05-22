#pragma once

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �A�N�Z�T                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! �ǂ�����ł��A�N�Z�X�ł���A���L�f�[�^�A�N�Z�T�B2007.10.30 kobake
struct DLLSHAREDATA;
DLLSHAREDATA& GetDllShareData();

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                    ���L�������\���v�f                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// 2010.04.19 Moca CShareData����DLLSHAREDATA�����o��include��DLLSHAREDATA.h�Ɉړ�

#include "config/maxdata.h"


#include "EditInfo.h"
#include "env/CAppNodeManager.h"	//SShare_Nodes

//2007.09.28 kobake Common�\���̂�CShareData.h���番��
#include "env/CommonSetting.h"

#include "types/CType.h" // STypeConfig
#include "CPrint.h" //PRINTSETTING
#include "env/CSearchKeywordManager.h"	//SShare_SearchKeywords
#include "env/CTagJumpManager.h"		//SShare_TagJump
#include "env/CFileNameManager.h"		//SShare_FileNameManagement
#include "recent/SShare_History.h"	//SShare_History

#ifndef SAKURA_EXCLUDE_OPTIONAL_INCLUDE
//@@@ 2001.12.26 YAZAKI CMRU, CMRUFolder
#include "recent/CMRU.h"
#include "recent/CMRUFolder.h"

#include "env/CFormatManager.h"
#include "env/CHelpManager.h"
#endif





//���L�t���O
struct SShare_Flags{
	BOOL				m_bEditWndChanging;				// �ҏW�E�B���h�E�ؑ֒�	// 2007.04.03 ryoji
	/*	@@@ 2002.1.24 YAZAKI
		�L�[�{�[�h�}�N���́A�L�^�I���������_�Ńt�@�C���um_szKeyMacroFileName�v�ɏ����o�����Ƃɂ���B
		m_bRecordingKeyMacro��TRUE�̂Ƃ��́A�L�[�{�[�h�}�N���̋L�^���Ȃ̂ŁAm_szKeyMacroFileName�ɃA�N�Z�X���Ă͂Ȃ�Ȃ��B
	*/
	BOOL				m_bRecordingKeyMacro;		/* �L�[�{�[�h�}�N���̋L�^�� */
	HWND				m_hwndRecordingKeyMacro;	/* �L�[�{�[�h�}�N�����L�^���̃E�B���h�E */
};

//���L���[�N�o�b�t�@
struct SShare_WorkBuffer{
	//2007.09.16 kobake char�^���ƁA��ɕ�����ł���Ƃ�������������̂ŁABYTE�^�ɕύX�B�ϐ������ύX�B
	//           UNICODE�łł́A�]���ɗ̈���g�����Ƃ��\�z����邽�߁AANSI�ł�2�{�m�ہB
private:
	BYTE				m_pWork[32000*sizeof(TCHAR)];
public:
	template <class T>
	T* GetWorkBuffer(){ return reinterpret_cast<T*>(m_pWork); }

	template <class T>
	size_t GetWorkBufferCount(){ return sizeof(m_pWork)/sizeof(T); }

public:
	EditInfo			m_EditInfo_MYWM_GETFILEINFO;	//MYWM_GETFILEINFO�f�[�^�󂯓n���p	####�������Ȃ�
};

//���L�n���h��
struct SShare_Handles{
	HWND				m_hwndTray;
	HWND				m_hwndDebug;
	HACCEL				m_hAccel;
};

//EXE���
struct SShare_Version{
	DWORD				m_dwProductVersionMS;
	DWORD				m_dwProductVersionLS;
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                   ���L�������\���̖{��                      //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

struct DLLSHAREDATA{
	void OnInit();
	
	// -- -- �o�[�W���� -- -- //
	/*!
		�f�[�^�\�� Version	//	Oct. 27, 2000 genta
		�f�[�^�\���̈قȂ�o�[�W�����̓����N����h������
		�K���擪�ɂȂ��Ă͂Ȃ�Ȃ��D
	*/
	unsigned int				m_vStructureVersion;

	// -- -- ��ۑ��Ώ� -- -- //
	SShare_Version				m_sVersion;	//���Ǎ��͍s��Ȃ����A�����͍s��
	SShare_WorkBuffer			m_sWorkBuffer;
	SShare_Flags				m_sFlags;
	SShare_Nodes				m_sNodes;
	SShare_Handles				m_sHandles;
	// �������p�S�p�L���b�V��
	BYTE				m_bCharWidthCache[0x10000/4];		//16KB �������p�S�p�L���b�V�� 2008/5/16 Uchi
	int					m_nCharWidthCacheTest;				//cache��ꌟ�o

	// -- -- �ۑ��Ώ� -- -- //
	//�ݒ�
	CommonSetting				m_Common;								// ���ʐݒ�
	STypeConfig					m_Types[MAX_TYPES];						// �^�C�v�ʐݒ�
	PRINTSETTING				m_PrintSettingArr[MAX_PRINTSETTINGARR];	// ����y�[�W�ݒ�
	//���̑�
	SShare_SearchKeywords		m_sSearchKeywords;
	SShare_TagJump				m_sTagJump;
	SShare_FileNameManagement	m_sFileNameManagement;
	SShare_History				m_sHistory;

	//�O���R�}���h���s�_�C�A���O�̃I�v�V����
	int							m_nExecFlgOpt;				/* �O���R�}���h���s�I�v�V���� */	//	2006.12.03 maru �I�v�V�����̊g���̂���
	//DIFF�����\���_�C�A���O�̃I�v�V����
	int							m_nDiffFlgOpt;				/* DIFF�����\�� */	//@@@ 2002.05.27 MIK
	//�^�O�t�@�C���̍쐬�_�C�A���O�̃I�v�V����
	TCHAR						m_szTagsCmdLine[_MAX_PATH];	/* TAGS�R�}���h���C���I�v�V���� */	//@@@ 2003.05.12 MIK
	int							m_nTagsOpt;					/* TAGS�I�v�V����(�`�F�b�N) */	//@@@ 2003.05.12 MIK


	// -- -- �e���|���� -- -- //
	//�w��s�փW�����v�_�C�A���O�̃I�v�V����
	bool						m_bLineNumIsCRLF_ForJump;			/* �w��s�փW�����v�́u���s�P�ʂ̍s�ԍ��v���u�܂�Ԃ��P�ʂ̍s�ԍ��v�� */
};


