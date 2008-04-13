#pragma once

class CEditView;


//! �N���b�s���O�̈���v�Z����ۂ̃t���O
enum EPaintArea{
	PAINT_LINENUMBER = (1<<0), //!< �s�ԍ�
	PAINT_RULER      = (1<<1), //!< ���[���[
	PAINT_BODY       = (1<<2), //!< �{��

	//����
	PAINT_ALL        = PAINT_LINENUMBER | PAINT_RULER | PAINT_BODY, //!< �����
};

class CEditView_Paint{
public:
	virtual CEditView* GetEditView()=0;

public:
	virtual ~CEditView_Paint(){}
	void Call_OnPaint(
		int nPaintFlag,   //!< �`�悷��̈��I������
		bool bUseMemoryDC //!< ������DC���g�p����
	);
};
