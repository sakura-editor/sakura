//2007.10.02 kobake CEditViewから分離

#pragma once

class CConvertMediator{
public:
	//! 機能種別によるバッファの変換
	static void ConvMemory( CNativeW* pCMemory, int nFuncCode, int nTabWidth );

protected:
	static void Command_TRIM2( CNativeW* pCMemory , BOOL bLeft );
};

class CConvert{
public:
	virtual ~CConvert(){}

	//インターフェース
	void CallConvert( CNativeW* pcData )
	{
		bool bRet=DoConvert(pcData);
		if(!bRet){
			ErrorMessage(NULL,_T("変換でエラーが発生しました"));
		}
	}

	//実装
	virtual bool DoConvert( CNativeW* pcData )=0;
};
