#include "stdafx.h"
#include "CType.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        CTypeConfig                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
STypeConfig* CTypeConfig::GetTypeConfig()
{
	return &GetDllShareData().GetTypeSetting(*this);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          CType                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
void CType::InitTypeConfig(int nIdx)
{
	DLLSHAREDATA* pShareData = &GetDllShareData();

	//規定値をコピー
	pShareData->GetTypeSetting(CTypeConfig(nIdx)) = pShareData->GetTypeSetting(CTypeConfig(0));

	//インデックスを設定
	CTypeConfig(nIdx)->m_nIdx = nIdx;

	//個別設定
	InitTypeConfigImp(CTypeConfig(nIdx).GetTypeConfig());
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        CShareData                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!	@brief 共有メモリ初期化/タイプ別設定

	タイプ別設定の初期化処理

	@date 2005.01.30 genta CShareData::Init()から分離．
*/
void CShareData::InitTypeConfigs(DLLSHAREDATA* pShareData)
{
	CType* table[] = {
		new CType_Basis(),	//基本
		new CType_Text(),	//テキスト
		new CType_Cpp(),	//C/C++
		new CType_Html(),	//HTML
		new CType_Sql(),	//PL/SQL
		new CType_Cobol(),	//COBOL
		new CType_Java(),	//Java
		new CType_Asm(),	//アセンブラ
		new CType_Awk(),	//awk
		new CType_Dos(),	//MS-DOSバッチファイル
		new CType_Pascal(),	//Pascal
		new CType_Tex(),	//TeX
		new CType_Perl(),	//Perl
		new CType_Vb(),		//Visual Basic
		new CType_Rich(),	//リッチテキスト
		new CType_Ini(),	//設定ファイル
		new CType_Other1(),	//設定17
		new CType_Other2(),	//設定18
		new CType_Other3(),	//設定19
		new CType_Other4(),	//設定20
	};
	for(int i=0;i<_countof(table);i++){
		table[i]->InitTypeConfig(i);
		SAFE_DELETE(table[i]);
	}
}
