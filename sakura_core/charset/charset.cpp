#include "StdAfx.h"
#include "charset.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ���O                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

LPCTSTR CCodeTypeName::Normal() const
{
	LPCTSTR table[] = {
		_T("SJIS"),			/* SJIS */
		_T("JIS"),			/* JIS */
		_T("EUC"),			/* EUC */
		_T("Unicode"),		/* Unicode */
		_T("UTF-8"),		/* UTF-8 */
		_T("UTF-7"),		/* UTF-7 */
		_T("UniBE"),		/* Unicode BigEndian */
		_T("CESU-8")		/* CESU-8 */
	};
	if(!IsValidCodeType(m_eCodeType))return NULL;
	return table[m_eCodeType];
}

LPCTSTR CCodeTypeName::Short() const
{
	LPCTSTR table[] = {
		_T("SJIS"),			/* SJIS */
		_T("JIS"),			/* JIS */
		_T("EUC"),			/* EUC */
		_T("Uni"),			/* Unicode */
		_T("UTF-8"),		/* UTF-8 */
		_T("UTF-7"),		/* UTF-7 */
		_T("UniBE"),		/* Unicode BigEndian */
		_T("CESU-8")		/* CESU-8 */
	};
	if(!IsValidCodeType(m_eCodeType))return NULL;
	return table[m_eCodeType];
}

LPCTSTR CCodeTypeName::Bracket() const
{
	LPCTSTR table[] = {
		_T("  [SJIS]"),		/* SJIS */
		_T("  [JIS]"),		/* JIS */
		_T("  [EUC]"),		/* EUC */
		_T("  [Unicode]"),	/* Unicode */
		_T("  [UTF-8]"),	/* UTF-8 */
		_T("  [UTF-7]"),	/* UTF-7 */
		_T("  [UniBE]"),	/* Unicode BigEndian */
		_T("  [CESU-8]")	/* CESU-8 */
	};
	if(!IsValidCodeType(m_eCodeType))return NULL;
	return table[m_eCodeType];
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      �R���{�{�b�N�X                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

ECodeType gm_nCodeComboValueArr[] = {
	CODE_AUTODETECT,	/* �����R�[�h�������� */
	CODE_SJIS,
	CODE_JIS,
	CODE_EUC,
	CODE_UNICODE,
	CODE_UNICODEBE,
	CODE_UTF8,
	CODE_CESU8,
	CODE_UTF7
};

LPCTSTR	gm_pszCodeComboNameArr[] = {
	_T("�����I��"),
	_T("SJIS"),
	_T("JIS"),
	_T("EUC"),
	_T("Unicode"),
	_T("UnicodeBE"),
	_T("UTF-8"),
	_T("CESU-8"),
	_T("UTF-7")
};

int CCodeTypesForCombobox::GetCount() const
{
	return _countof(gm_nCodeComboValueArr);
}

ECodeType CCodeTypesForCombobox::GetCode(int nIndex) const
{
	return gm_nCodeComboValueArr[nIndex];
}

LPCTSTR CCodeTypesForCombobox::GetName(int nIndex) const
{
	return gm_pszCodeComboNameArr[nIndex];
}


