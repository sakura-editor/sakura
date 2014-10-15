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
#ifndef SAKURA_CGREPAGENT_89E8C8B7_433B_47F3_A389_75C91E00A4BA9_H_
#define SAKURA_CGREPAGENT_89E8C8B7_433B_47F3_A389_75C91E00A4BA9_H_

#include "doc/CDocListener.h"
class CDlgCancel;
class CEditView;
class CSearchStringPattern;
class CGrepEnumKeys;
class CGrepEnumFiles;
class CGrepEnumFolders;

struct SGrepOption{
	bool		bGrepSubFolder;			//!< �T�u�t�H���_�������������
	bool		bGrepStdout;			//!< �W���o�̓��[�h
	bool		bGrepHeader;			//!< �w�b�_�E�t�b�_�\��
	ECodeType	nGrepCharSet;			//!< �����R�[�h�Z�b�g�I��
	bool		bGrepOutputLine;		//!< true: �q�b�g�s���o�� / false: �q�b�g�������o��
	int			nGrepOutputStyle;		//!< �o�͌`�� 1: Normal, 2: WZ��(�t�@�C���P��) 3: ���ʂ̂�
	bool		bGrepOutputFileOnly;	//!< �t�@�C�����ŏ��̂݌���
	bool		bGrepOutputBaseFolder;	//!< �x�[�X�t�H���_�\��
	bool		bGrepSeparateFolder;	//!< �t�H���_���ɕ\��

	SGrepOption() : 
		 bGrepSubFolder(true)
		,bGrepStdout(false)
		,bGrepHeader(true)
		,nGrepCharSet(CODE_AUTODETECT)
		,bGrepOutputLine(true)
		,nGrepOutputStyle(true)
		,bGrepOutputFileOnly(false)
		,bGrepOutputBaseFolder(false)
		,bGrepSeparateFolder(false)
	{}
};

//	Jun. 26, 2001 genta	���K�\�����C�u�����̍����ւ�
//	Mar. 28, 2004 genta DoGrepFile����s�v�Ȉ������폜
class CGrepAgent : public CDocListenerEx{
public:
	CGrepAgent();

	// �C�x���g
	ECallbackResult OnBeforeClose();
	void OnAfterSave(const SSaveInfo& sSaveInfo);

	static void CreateFolders( const TCHAR* pszPath, std::vector<std::tstring>& vPaths );
	static std::tstring ChopYen( const std::tstring& str );
	static void AddTail( CEditView* pcEditView, const CNativeW& cmem, bool bAddStdout );

	// Grep���s
	DWORD DoGrep(
		CEditView*				pcViewDst,
		const CNativeW*			pcmGrepKey,
		const CNativeT*			pcmGrepFile,
		const CNativeT*			pcmGrepFolder,
		bool					bGrepCurFolder,
		BOOL					bGrepSubFolder,
		bool					bGrepStdout,
		bool					bGrepHeader,
		const SSearchOption&	sSearchOption,
		ECodeType				nGrepCharSet,	// 2002/09/21 Moca �����R�[�h�Z�b�g�I��
		BOOL					bGrepOutputLine,
		int						nGrepOutputStyle,
		bool					bGrepOutputFileOnly,	//!< [in] �t�@�C�����ŏ��̂ݏo��
		bool					bGrepOutputBaseFolder,	//!< [in] �x�[�X�t�H���_�\��
		bool					bGrepSeparateFolder	//!< [in] �t�H���_���ɕ\��
	);

private:
	// Grep���s
	int DoGrepTree(
		CEditView*				pcViewDst,
		CDlgCancel*				pcDlgCancel,		//!< [in] Cancel�_�C�A���O�ւ̃|�C���^
		const wchar_t*			pszKey,				//!< [in] �����p�^�[��
		CGrepEnumKeys&			cGrepEnumKeys,		//!< [in] �����Ώۃt�@�C���p�^�[��(!�ŏ��O�w��)
		CGrepEnumFiles&			cGrepExceptAbsFiles,
		CGrepEnumFolders&		cGrepExceptAbsFolders,
		const TCHAR*			pszPath,			//!< [in] �����Ώۃp�X
		const TCHAR*			pszBasePath,		//!< [in] �����Ώۃp�X(�x�[�X)
		const SSearchOption&	sSearchOption,		//!< [in] �����I�v�V����
		const SGrepOption&		sGrepOption,		//!< [in] Grep�I�v�V����
		const CSearchStringPattern& pattern,		//!< [in] �����p�^�[��
		CBregexp*				pRegexp,			//!< [in] ���K�\���R���p�C���f�[�^�B���ɃR���p�C������Ă���K�v������
		int						nNest,				//!< [in] �l�X�g���x��
		bool&					bOutputBaseFolder,
		int*					pnHitCount			//!< [i/o] �q�b�g���̍��v
	);

	// Grep���s
	int DoGrepFile(
		CEditView*				pcViewDst,
		CDlgCancel*				pcDlgCancel,
		const wchar_t*			pszKey,
		const TCHAR*			pszFile,
		const SSearchOption&	sSearchOption,
		const SGrepOption&		sGrepOption,
		const CSearchStringPattern& pattern,
		CBregexp*				pRegexp,		//	Jun. 27, 2001 genta	���K�\�����C�u�����̍����ւ�
		int*					pnHitCount,
		const TCHAR*			pszFullPath,
		const TCHAR*			pszBaseFolder,
		const TCHAR*			pszFolder,
		const TCHAR*			pszRelPath,
		bool&					bOutputBaseFolder,
		bool&					bOutputFolderName,
		CNativeW&				cmemMessage
	);

	// Grep���ʂ�pszWork�Ɋi�[
	void SetGrepResult(
		// �f�[�^�i�[��
		CNativeW&		cmemMessage,
		// �}�b�`�����t�@�C���̏��
		const TCHAR*	pszFilePath,	//	�t���p�X or ���΃p�X
		const TCHAR*	pszCodeName,	//	�����R�[�h���"[SJIS]"�Ƃ�
		// �}�b�`�����s�̏��
		int				nLine,			//	�}�b�`�����s�ԍ�
		int				nColumn,		//	�}�b�`�������ԍ�
		const wchar_t*	pCompareData,	//	�s�̕�����
		int				nLineLen,		//	�s�̕�����̒���
		int				nEolCodeLen,	//	EOL�̒���
		// �}�b�`����������̏��
		const wchar_t*	pMatchData,		//	�}�b�`����������
		int				nMatchLen,		//	�}�b�`����������̒���
		// �I�v�V����
		const SGrepOption&	sGrepOption
	);

public: //$$ ��
	bool	m_bGrepMode;		//!< Grep���[�h��
	bool	m_bGrepRunning;		//!< Grep������
};

#endif /* SAKURA_CGREPAGENT_89E8C8B7_433B_47F3_A389_75C91E00A4BA9_H_ */
/*[EOF]*/
