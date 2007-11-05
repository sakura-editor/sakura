//2007.10.02 kobake CEditView���番��

#pragma once

class CConvertMediator{
public:
	//! �@�\��ʂɂ��o�b�t�@�̕ϊ�
	static void ConvMemory( CNativeW2* pCMemory, int nFuncCode, int nTabWidth );

protected:
	static void Command_TRIM2( CNativeW2* pCMemory , BOOL bLeft );
};

class CConvert{
public:
	virtual ~CConvert(){}

	//�C���^�[�t�F�[�X
	void CallConvert( CNativeW2* pcData )
	{
		bool bRet=DoConvert(pcData);
		if(!bRet){
			ErrorMessage(NULL,_T("�ϊ��ŃG���[���������܂���"));
		}
	}

	//����
	virtual bool DoConvert( CNativeW2* pcData )=0;
};
