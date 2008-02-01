//2007.10.02 kobake CEditView���番��

#pragma once

enum EFunctionCode;

class CConvertMediator{
public:
	//! �@�\��ʂɂ��o�b�t�@�̕ϊ�
	static void ConvMemory( CNativeW* pCMemory, EFunctionCode nFuncCode, int nTabWidth );

protected:
	static void Command_TRIM2( CNativeW* pCMemory , BOOL bLeft );
};

class CConvert{
public:
	virtual ~CConvert(){}

	//�C���^�[�t�F�[�X
	void CallConvert( CNativeW* pcData )
	{
		bool bRet=DoConvert(pcData);
		if(!bRet){
			ErrorMessage(NULL,_T("�ϊ��ŃG���[���������܂���"));
		}
	}

	//����
	virtual bool DoConvert( CNativeW* pcData )=0;
};
