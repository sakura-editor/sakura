#pragma once

#include "doc/CDocListener.h"
#include "COpeBuf.h"

class CEditDoc;
class CDocLineMgr;

class CDocEditor : public CDocListenerEx{
public:
	CDocEditor(CEditDoc* pcDoc);

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         �C�x���g                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//���[�h�O��
	void OnBeforeLoad(SLoadInfo* sLoadInfo);
	void OnAfterLoad(const SLoadInfo& sLoadInfo);

	//�Z�[�u�O��
	void OnAfterSave(const SSaveInfo& sSaveInfo);



	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           ���                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//	Jan. 22, 2002 genta Modified Flag�̐ݒ�
	void SetModified( bool flag, bool redraw);
	//! �t�@�C�����C�������ǂ���
	bool IsModified() const { return m_bIsDocModified; }

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           �ݒ�                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//	Nov. 20, 2000 genta
	void SetImeMode(int mode);	//	IME��Ԃ̐ݒ�

	//	May 15, 2000 genta
	CEol  GetNewLineCode() const { return m_cNewLineCode; }
	void  SetNewLineCode(const CEol& t){ m_cNewLineCode = t; }

	//	Oct. 2, 2005 genta �}�����[�h�̐ݒ�
	bool IsInsMode() const { return m_bInsMode; }
	void SetInsMode(bool mode) { m_bInsMode = mode; }

	bool IsEnableUndo( void );				/* Undo(���ɖ߂�)�\�ȏ�Ԃ��H */
	bool IsEnableRedo( void );				/* Redo(��蒼��)�\�ȏ�Ԃ��H */
	bool IsEnablePaste( void );				/* �N���b�v�{�[�h����\��t���\���H */

public:
	CEditDoc*		m_pcDocRef;
	CEol 			m_cNewLineCode;				//!< Enter�������ɑ}��������s�R�[�h���
	COpeBuf			m_cOpeBuf;					//!< �A���h�D�o�b�t�@
	bool			m_bInsMode;					//!< �}���E�㏑�����[�h Oct. 2, 2005 genta
	bool			m_bIsDocModified;
};


class CDocEditAgent{
public:
	CDocEditAgent(CDocLineMgr* pcDocLineMgr) : m_pcDocLineMgr(pcDocLineMgr) { }

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           ����                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//	May 15, 2000 genta
	void AddLineStrX( const wchar_t*, int );	/* �����ɍs��ǉ� Ver1.5 */

	void DeleteData_CDocLineMgr(
		CLogicInt	nLine,
		CLogicInt	nDelPos,
		CLogicInt	nDelLen,
		CLogicInt*	pnModLineOldFrom,	/* �e���̂������ύX�O�̍s(from) */
		CLogicInt*	pnModLineOldTo,		/* �e���̂������ύX�O�̍s(to) */
		CLogicInt*	pnDelLineOldFrom,	/* �폜���ꂽ�ύX�O�_���s(from) */
		CLogicInt*	pnDelLineOldNum,	/* �폜���ꂽ�s�� */
		CNativeW*	cmemDeleted			/* �폜���ꂽ�f�[�^ */
	);

	/* �f�[�^�̑}�� */
	void InsertData_CDocLineMgr(
		CLogicInt		nLine,
		CLogicInt		nInsPos,
		const wchar_t*	pInsData,
		CLogicInt		nInsDataLen,
		CLogicInt*		pnInsLineNum,	// �}���ɂ���đ������s�̐�
		CLogicPoint*	pptNewPos		// �}�����ꂽ�����̎��̈ʒu
	);
private:
	CDocLineMgr* m_pcDocLineMgr;
};
