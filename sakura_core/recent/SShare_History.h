/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_SSHARE_HISTORY_9F7E6200_FEE2_4CAC_A5D3_32EEC4130CFC_H_
#define SAKURA_SSHARE_HISTORY_9F7E6200_FEE2_4CAC_A5D3_32EEC4130CFC_H_

#include "EditInfo.h"
#include "config/maxdata.h"

//共有メモリ内構造体
struct SShare_History{
	//@@@ 2001.12.26 YAZAKI	以下の2つは、直接アクセスしないでください。CMRUを経由してください。
	int					m_nMRUArrNum;
	EditInfo			m_fiMRUArr[MAX_MRU];
	bool				m_bMRUArrFavorite[MAX_MRU];	//お気に入り	//@@@ 2003.04.08 MIK

	//@@@ 2001.12.26 YAZAKI	以下の2つは、直接アクセスしないでください。CMRUFolderを経由してください。
	int								m_nOPENFOLDERArrNum;
	StaticString<_MAX_PATH>			m_szOPENFOLDERArr[MAX_OPENFOLDER];
	bool							m_bOPENFOLDERArrFavorite[MAX_OPENFOLDER];	//お気に入り	//@@@ 2003.04.08 MIK

	//MRU除外リスト一覧
	StaticVector< StaticString<_MAX_PATH>, MAX_MRU,  const WCHAR* >	m_aExceptMRU;

	//MRU以外の情報
	SFilePath													m_szIMPORTFOLDER;	// インポートディレクトリの履歴
	StaticVector< StaticString<MAX_CMDLEN>, MAX_CMDARR > m_aCommands;		// 外部コマンド実行履歴
	StaticVector< StaticString<_MAX_PATH>, MAX_CMDARR > m_aCurDirs;		// カレントディレクトリ履歴

	explicit SShare_History(
		const std::filesystem::path& iniFolder
	) noexcept
		: m_szIMPORTFOLDER(iniFolder)
	{
	}
};

#endif /* SAKURA_SSHARE_HISTORY_9F7E6200_FEE2_4CAC_A5D3_32EEC4130CFC_H_ */
