#pragma once

//共有メモリ内構造体
struct SShare_History{
	//@@@ 2001.12.26 YAZAKI	以下の2つは、直接アクセスしないでください。CMRUを経由してください。
	int					m_nMRUArrNum;
	EditInfo			m_fiMRUArr[MAX_MRU];
	bool				m_bMRUArrFavorite[MAX_MRU];	//お気に入り	//@@@ 2003.04.08 MIK

	//@@@ 2001.12.26 YAZAKI	以下の2つは、直接アクセスしないでください。CMRUFolderを経由してください。
	int								m_nOPENFOLDERArrNum;
	StaticString<TCHAR,_MAX_PATH>	m_szOPENFOLDERArr[MAX_OPENFOLDER];
	bool							m_bOPENFOLDERArrFavorite[MAX_OPENFOLDER];	//お気に入り	//@@@ 2003.04.08 MIK

	//MRU以外の情報
	SFilePath													m_szIMPORTFOLDER;	// インポートディレクトリの履歴
	StaticVector< StaticString<TCHAR, MAX_CMDLEN>, MAX_CMDARR > m_aCommands;		// 外部コマンド実行履歴
};
