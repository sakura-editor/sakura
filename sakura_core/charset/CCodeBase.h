#pragma once

//定数
enum EConvertResult{
	RESULT_COMPLETE, //!< データを失うことなく変換が完了した。
	RESULT_LOSESOME, //!< 変換が完了したが、一部のデータが失われた。
	RESULT_FAILURE,  //!< 何らかの原因により失敗した。
};

/*!
	文字コード基底クラス。
	
	ここで言う「特定コード」とは、
	CCodeBaseを継承した子クラスが定める、一意の文字コードのことです。
*/
class CCodeBase{
public:
	virtual ~CCodeBase(){}
//	virtual bool IsCode(const CMemory* pMem){return false;}  //!< 特定コードであればtrue
	virtual EConvertResult CodeToUnicode(const CMemory* pSrc, CNativeW* pDst)=0; //!< 特定コード → UNICODE    変換
	virtual EConvertResult UnicodeToCode(const CNativeW* pSrc, CMemory* pDst)=0; //!< UNICODE    → 特定コード 変換
};

