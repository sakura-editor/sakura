#pragma once

#include "CRecentImp.h"
#include "util/StaticType.h"

typedef StaticString<TCHAR, _MAX_PATH> CMetaPath;

//! �t�H���_�̗������Ǘ� (RECENT_FOR_FOLDER)
class CRecentExceptMRU : public CRecentImp<CMetaPath, LPCTSTR>{
public:
	//����
	CRecentExceptMRU();

	//�I�[�o�[���C�h
	int				CompareItem( const CMetaPath* p1, LPCTSTR p2 ) const;
	void			CopyItem( CMetaPath* dst, LPCTSTR src ) const;
	const TCHAR*	GetItemText( int nIndex ) const;
	bool			DataToReceiveType( LPCTSTR* dst, const CMetaPath* src ) const;
	bool			TextToDataType( CMetaPath* dst, LPCTSTR pszText ) const;
};
