/*
	X�l�̒P�ʕϊ��֐��Q�B
*/

#pragma once

class CLayout;
class CDocLine;
class CEditView;

class CViewCalc{
protected:
	//�O���ˑ�
	CLayoutInt GetTabSpace() const;

public:
	CViewCalc(const CEditView* pOwner) : m_pOwner(pOwner) { }
	virtual ~CViewCalc(){}

	//�P�ʕϊ�: ���C�A�E�g�����W�b�N
	CLogicInt  LineColmnToIndex ( const CLayout*  pcLayout,  CLayoutInt nColumn ) const;		/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� Ver1 */		// @@@ 2002.09.28 YAZAKI
	CLogicInt  LineColmnToIndex ( const CDocLine* pcDocLine, CLayoutInt nColumn ) const;		/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� Ver1 */		// @@@ 2002.09.28 YAZAKI
	CLogicInt  LineColmnToIndex2( const CLayout*  pcLayout,  CLayoutInt nColumn, CLayoutInt* pnLineAllColLen ) const;	/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� Ver0 */		// @@@ 2002.09.28 YAZAKI

	//�P�ʕϊ�: ���W�b�N�����C�A�E�g
	CLayoutInt LineIndexToColmn ( const CLayout*  pcLayout,  CLogicInt nIndex ) const;		// �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ�	// @@@ 2002.09.28 YAZAKI
	CLayoutInt LineIndexToColmn ( const CDocLine* pcLayout,  CLogicInt nIndex ) const;		// �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ�	// @@@ 2002.09.28 YAZAKI

private:
	const CEditView* m_pOwner;
};
