#pragma once

#include "doc/CDocListener.h"
class CDlgCancel;
class CEditView;

//	Jun. 26, 2001 genta	���K�\�����C�u�����̍����ւ�
//	Mar. 28, 2004 genta DoGrepFile����s�v�Ȉ������폜
class CGrepAgent : public CDocListenerEx{
public:
	CGrepAgent();

	// �C�x���g
	ECallbackResult OnBeforeClose();

	// Grep���s
	DWORD DoGrep(
		CEditView*				pcViewDst,
		const CNativeW*			pcmGrepKey,
		const CNativeT*			pcmGrepFile,
		const CNativeT*			pcmGrepFolder,
		BOOL					bGrepSubFolder,
		const SSearchOption&	sSearchOption,
		ECodeType				nGrepCharSet,	// 2002/09/21 Moca �����R�[�h�Z�b�g�I��
		BOOL					bGrepOutputLine,
		int						nGrepOutputStyle
	);

	// Grep���s
	int DoGrepTree(
		CEditView*				pcViewDst,
		CDlgCancel*				pcDlgCancel,		//!< [in] Cancel�_�C�A���O�ւ̃|�C���^
		HWND					hwndCancel,			//!< [in] Cancel�_�C�A���O�̃E�B���h�E�n���h��
		const wchar_t*			pszKey,				//!< [in] �����p�^�[��
		int*					pnKey_CharCharsArr,	//!< [in] ������z��(2byte/1byte)�D�P�������񌟍��Ŏg�p�D
		const TCHAR*			pszFile,			//!< [in] �����Ώۃt�@�C���p�^�[��(!�ŏ��O�w��)
		const TCHAR*			pszPath,			//!< [in] �����Ώۃp�X
		BOOL					bGrepSubFolder,		//!< [in] TRUE: �T�u�t�H���_���ċA�I�ɒT������ / FALSE: ���Ȃ�
		const SSearchOption&	sSearchOption,		//!< [in] �����I�v�V����
		ECodeType				nGrepCharSet,		//!< [in] �����R�[�h�Z�b�g (0:�����F��)�`
		BOOL					bGrepOutputLine,	//!< [in] TRUE: �q�b�g�s���o�� / FALSE: �q�b�g�������o��
		int						nGrepOutputStyle,	//!< [in] �o�͌`�� 1: Normal, 2: WZ��(�t�@�C���P��)
		CBregexp*				pRegexp,			//!< [in] ���K�\���R���p�C���f�[�^�B���ɃR���p�C������Ă���K�v������
		int						nNest,				//!< [in] �l�X�g���x��
		int*					pnHitCount			//!< [i/o] �q�b�g���̍��v
	);

	// Grep���s
	int DoGrepFile(
		CEditView*				pcViewDst,
		CDlgCancel*				pcDlgCancel,
		HWND					hwndCancel,
		const wchar_t*			pszKey,
		int*					pnKey_CharCharsArr,
		const TCHAR*			pszFile,
		const SSearchOption&	sSearchOption,
		ECodeType				nGrepCharSet,
		BOOL					bGrepOutputLine,
		int						nGrepOutputStyle,
		CBregexp*				pRegexp,		//	Jun. 27, 2001 genta	���K�\�����C�u�����̍����ւ�
		int*					pnHitCount,
		const TCHAR*			pszFullPath,
		CNativeW&				cmemMessage
	);

	// Grep���ʂ�pszWork�Ɋi�[
	void SetGrepResult(
		// �f�[�^�i�[��
		wchar_t*		pWork,
		int*			pnWorkLen,
		// �}�b�`�����t�@�C���̏��
		const TCHAR*	pszFullPath,	//	�t���p�X
		const TCHAR*	pszCodeName,	//	�����R�[�h���"[SJIS]"�Ƃ�
		// �}�b�`�����s�̏��
		int				nLine,			//	�}�b�`�����s�ԍ�
		int				nColm,			//	�}�b�`�������ԍ�
		const wchar_t*	pCompareData,	//	�s�̕�����
		int				nLineLen,		//	�s�̕�����̒���
		int				nEolCodeLen,	//	EOL�̒���
		// �}�b�`����������̏��
		const wchar_t*	pMatchData,		//	�}�b�`����������
		int				nMatchLen,		//	�}�b�`����������̒���
		// �I�v�V����
		BOOL			bGrepOutputLine,
		int				nGrepOutputStyle
	);

public: //$$ ��
	bool	m_bGrepMode;		//!< Grep���[�h��
	bool	m_bGrepRunning;		//!< Grep������
};
