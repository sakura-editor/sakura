/*
	Observer�p�^�[����CEditDoc�����ŁB
	CDocSubject�͊ώ@����ACDocListner�͊ώ@���s���B
	�ώ@�̊J�n�� CDocListener::Listen �ōs���B

	$Note:
		Listener (Observer) �� Subject �̃����[�V�����Ǘ���
		�W�F�l���b�N�Ȕėp���W���[���ɕ����ł���B
*/
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
#ifndef SAKURA_CDOCLISTENER_797F65F5_C90A_4055_944C_AB9256AD8B90_H_
#define SAKURA_CDOCLISTENER_797F65F5_C90A_4055_944C_AB9256AD8B90_H_

class CDocListener;
#include "basis/CMyString.h"
#include "charset/charset.h"
#include "CEol.h"
#include "types/CType.h"
#include "util/relation_tool.h"

//###
enum ESaveResult{
	SAVED_OK,
	SAVED_FAILURE,
	SAVED_INTERRUPT,//!< ���f���ꂽ
	SAVED_LOSESOME,	//!< �����̈ꕔ������ꂽ
};

//###
enum ELoadResult{
	LOADED_OK,
	LOADED_FAILURE,
	LOADED_INTERRUPT,	//!< ���f���ꂽ
	LOADED_LOSESOME,	//!< �����̈ꕔ������ꂽ

	//����
	LOADED_NOIMPLEMENT,	//!< ��������
};

//###
enum ECallbackResult{
	CALLBACK_CONTINUE,			//!< ������
	CALLBACK_INTERRUPT,			//!< ���f
};

//###
struct SLoadInfo
{
	//����
	CFilePath	cFilePath;
	ECodeType	eCharCode;
	bool		bViewMode;
	CTypeConfig	nType;

	//���[�h
	bool		bRequestReload;	//�����[�h�v��

	//�o��
	bool		bOpened;

	SLoadInfo()
	: cFilePath(_T(""))
	, eCharCode(CODE_AUTODETECT)
	, bViewMode(false)
	, nType(-1)
	, bRequestReload(false)
	, bOpened(false)
	{
	}
	SLoadInfo(const CFilePath& _cFilePath, ECodeType _eCodeType, bool _bReadOnly, CTypeConfig _nType = CTypeConfig(-1))
	: cFilePath(_cFilePath)
	, eCharCode(_eCodeType)
	, bViewMode(_bReadOnly)
	, nType(_nType)
	, bRequestReload(false)
	, bOpened(false)
	{
	}

	//! �t�@�C���p�X�̔�r
	bool IsSamePath(LPCTSTR pszPath) const;
};

struct SSaveInfo{
	CFilePath	cFilePath;	//!< �ۑ��t�@�C����
	ECodeType	eCharCode;	//!< �ۑ������R�[�h�Z�b�g
	CEol		cEol;		//!< �ۑ����s�R�[�h
	bool		bBomExist;	//!< �ۑ���BOM�t��

	//���[�h
	bool		bOverwriteMode;	//!< �㏑���v��

	SSaveInfo() : cFilePath(_T("")), eCharCode(CODE_AUTODETECT), cEol(EOL_NONE), bBomExist(false), bOverwriteMode(false) { }
	SSaveInfo(const CFilePath& _cFilePath, ECodeType _eCodeType, const CEol& _cEol, bool _bBomExist)
		: cFilePath(_cFilePath), eCharCode(_eCodeType), cEol(_cEol), bBomExist(_bBomExist), bOverwriteMode(false) { }

	//! �t�@�C���p�X�̔�r
	bool IsSamePath(LPCTSTR pszPath) const;
};


class CProgressListener;

//! ������CProgressSubject����E�H�b�`�����
class CProgressSubject : public CSubjectT<CProgressListener>{
public:
	virtual ~CProgressSubject(){}
	void NotifyProgress(int nPer);
};

//! 1��CProgressSubject���E�H�b�`����
class CProgressListener : public CListenerT<CProgressSubject>{
public:
	virtual ~CProgressListener(){}
	virtual void OnProgress(int nPer)=0;
};

//Subject�͕�����Listener����ώ@�����
class CDocSubject : public CSubjectT<CDocListener>{
public:
	virtual ~CDocSubject();

	//���[�h�O��
	ECallbackResult NotifyCheckLoad	(SLoadInfo* pLoadInfo);
	void NotifyBeforeLoad			(SLoadInfo* sLoadInfo);
	ELoadResult NotifyLoad			(const SLoadInfo& sLoadInfo);
	void NotifyLoading				(int nPer);
	void NotifyAfterLoad			(const SLoadInfo& sLoadInfo);
	void NotifyFinalLoad			(ELoadResult eLoadResult);

	//�Z�[�u�O��
	ECallbackResult NotifyCheckSave	(SSaveInfo* pSaveInfo);
	ECallbackResult NotifyPreBeforeSave(SSaveInfo* pSaveInfo);
	void NotifyBeforeSave			(const SSaveInfo& sSaveInfo);
	void NotifySave					(const SSaveInfo& sSaveInfo);
	void NotifySaving				(int nPer);
	void NotifyAfterSave			(const SSaveInfo& sSaveInfo);
	void NotifyFinalSave			(ESaveResult eSaveResult);

	//�N���[�Y�O��
	ECallbackResult NotifyBeforeClose();
};

//Listener��1��Subject���ώ@����
class CDocListener : public CListenerT<CDocSubject>{
public:
	CDocListener(CDocSubject* pcDoc = NULL);
	virtual ~CDocListener();

	// -- -- ���� -- -- //
	CDocSubject* GetListeningDoc() const{ return GetListeningSubject(); }

	// -- -- �e��C�x���g -- -- //
	//���[�h�O��
	virtual ECallbackResult	OnCheckLoad	(SLoadInfo* pLoadInfo)		{ return CALLBACK_CONTINUE; }	//!< �{���Ƀ��[�h���s�����̔�����s��
	virtual void			OnBeforeLoad(SLoadInfo* sLoadInfo){ return ; }	//!< ���[�h���O����
	virtual ELoadResult		OnLoad		(const SLoadInfo& sLoadInfo){ return LOADED_NOIMPLEMENT; }	//!< ���[�h����
	virtual void			OnLoading	(int nPer)					{ return ; }	//!< ���[�h�����̌o�ߏ�����M
	virtual void			OnAfterLoad	(const SLoadInfo& sLoadInfo){ return ; }	//!< ���[�h���㏈��
	virtual void			OnFinalLoad	(ELoadResult eLoadResult)	{ return ; }	//!< ���[�h�t���[�̍Ō�ɕK���Ă΂��

	//�Z�[�u�O��
	virtual ECallbackResult OnCheckSave	(SSaveInfo* pSaveInfo)		{ return CALLBACK_CONTINUE; }	//!< �{���ɃZ�[�u���s�����̔�����s��
	virtual ECallbackResult OnPreBeforeSave	(SSaveInfo* pSaveInfo)	{ return CALLBACK_CONTINUE; }	//!< �Z�[�u���O���܂����� ($$ ��)
	virtual void			OnBeforeSave(const SSaveInfo& sSaveInfo){ return ; }	//!< �Z�[�u���O����
	virtual void			OnSave		(const SSaveInfo& sSaveInfo){ return ; }	//!< �Z�[�u����
	virtual void			OnSaving	(int nPer)					{ return ; }	//!< �Z�[�u�����̌o�ߏ�����M
	virtual void			OnAfterSave	(const SSaveInfo& sSaveInfo){ return ; }	//!< �Z�[�u���㏈��
	virtual void			OnFinalSave	(ESaveResult eSaveResult)	{ return ; }	//!< �Z�[�u�t���[�̍Ō�ɕK���Ă΂��

	//�N���[�Y�O��
	virtual ECallbackResult OnBeforeClose()							{ return CALLBACK_CONTINUE; }
};

//GetListeningDoc�̗��֐����A�b�v
class CEditDoc;
class CDocListenerEx : public CDocListener{
public:
	CDocListenerEx(CDocSubject* pcDoc = NULL) : CDocListener(pcDoc) { }
	CEditDoc* GetListeningDoc() const;
};


#include <exception>
class CFlowInterruption : public std::exception{
public:
	const char* what() const throw(){ return "CFlowInterruption"; }
};

#endif /* SAKURA_CDOCLISTENER_797F65F5_C90A_4055_944C_AB9256AD8B90_H_ */
/*[EOF]*/
