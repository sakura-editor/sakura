/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include "CommonSetting.h"

#include "env/CShareData.h"
#include "env/CShareData_IO.h"
#include "CDataProfile.h"
#include "util/window.h"

#include "CSelectLang.h"
#include "sakura_rc.h"
#include "String_define.h"


//CommonValue管理
struct CommonValueInfo{
	enum EType{
		TYPE_UNKNOWN,
		TYPE_ASTR,    //char文字列 (終端NULL)
		TYPE_WSTR,    //wchar_t文字列 (終端NULL)
	};

	void* m_pValue;     //!< 値へのポインタ
	int   m_nValueSize; //!< 値のサイズ。バイト単位。
	char  m_szEntryKey[32];
	EType m_eType;

	CommonValueInfo(void* pValue, int nValueSize, const char* szEntryKey, EType eType=TYPE_UNKNOWN)
	: m_pValue(pValue), m_nValueSize(nValueSize), m_eType(eType)
	{
		strcpy_s(m_szEntryKey,_countof(m_szEntryKey),szEntryKey);
	}

	void Save()
	{
		printf("%hs=",m_szEntryKey);

		//intと同じサイズならintとして出力
		if(m_nValueSize==sizeof(int)){
			printf("%d\n",*((int*)m_pValue));
		}
		//それ以外ならバイナリ出力
		else{
			for(int i=0;i<m_nValueSize;i++){
				printf("%%%02X",((BYTE*)m_pValue)[i]);
			}
		}
	}
};
std::vector<CommonValueInfo> g_commonvalues;
void CommonValue_AllSave()
{
	int nSize = (int)g_commonvalues.size();
	for(int i=0;i<nSize;i++){
		g_commonvalues[i].Save();
	}
}

//CommonValue ※virtual使うの禁止
template <class T>
class CommonValue{
private:
	typedef CommonValue<T> Me;
public:
	CommonValue() = default;
	void Regist(const char* szEntryKey)
	{
		//CommonValueリストに自分を追加
		g_commonvalues.push_back(CommonValueInfo(&m_value,sizeof(m_value),szEntryKey));
	}
	Me& operator = (const T& rhs){ m_value=rhs; return *this; }
	operator T& () { return m_value; }
	operator const T& () const{ return m_value; }
private:
	T m_value;
};

typedef char mystring[10];

void sample()
{
	CommonValue<int>      intvalue;
	CommonValue<mystring> strvalue;

	intvalue.Regist("intvalue");
	strvalue.Regist("strvalue");

	intvalue=3;
	strcpy(strvalue,"hage");

	CommonValue_AllSave();
}

CommonSetting::CommonSetting(
	const std::filesystem::path& iniFolder
) noexcept
	: m_sMacro(iniFolder)
{
}

CommonSetting_FileName::CommonSetting_FileName() noexcept
{
	const std::array<std::pair<std::wstring, std::wstring>, 7> expectedPairs = {{
		{ LR"(%DeskTop%\)",           LS(STR_TRANSNAME_DESKTOP) },
		{ LR"(%Personal%\)",          LS(STR_TRANSNAME_MYDOC) },
		{ LR"(%Cache%\Content.IE5\)", LS(STR_TRANSNAME_IE) },
		{ LR"(%TEMP%\)",              LS(STR_TRANSNAME_TEMP) },
		{ LR"(%Common DeskTop%\)",    LS(STR_TRANSNAME_COMDESKTOP) },
		{ LR"(%Common Documents%\)",  LS(STR_TRANSNAME_COMDOC) },
		{ LR"(%AppData%\)",           LS(STR_TRANSNAME_APPDATA) }
	}};

	m_nTransformFileNameArrNum = int(std::size(expectedPairs));

	for (size_t i = 0; i < expectedPairs.size(); ++i) {
		wcscpy_s(m_szTransformFileNameFrom[i], expectedPairs[i].first.c_str());
		wcscpy_s(m_szTransformFileNameTo[i], expectedPairs[i].second.c_str());
	}
}

CommonSetting_Format::CommonSetting_Format() noexcept
{
	wcscpy_s(m_szMidashiKigou, LS(STR_ERR_CSHAREDATA14));
	wcscpy_s(m_szInyouKigou, L"> ");
	wcscpy_s(m_szDateFormat, LS(STR_ERR_CSHAREDATA15));
	wcscpy_s(m_szTimeFormat, LS(STR_ERR_CSHAREDATA16));
}

CommonSetting_Helper::CommonSetting_Helper() noexcept
{
	auto& lfIconTitle = m_lf;
	auto& nIconPointSize = m_nPointSize;
	SystemParametersInfoW(
		SPI_GETICONTITLELOGFONT,
		sizeof(LOGFONT),
		&lfIconTitle,
		NULL
	);
	nIconPointSize = lfIconTitle.lfHeight >= 0 ? lfIconTitle.lfHeight : DpiPixelsToPoints(-lfIconTitle.lfHeight, 10);
}

CommonSetting_Macro::CommonSetting_Macro(
	const std::filesystem::path& iniFolder
) noexcept
	: m_szMACROFOLDER(iniFolder)
{
}

CommonSetting_MainMenu::CommonSetting_MainMenu() noexcept
{
	CDataProfile cProfile;
	cProfile.SetReadingMode();

	std::vector<std::wstring> data;
	cProfile.ReadProfileRes(MAKEINTRESOURCE(IDR_MENU1), MAKEINTRESOURCE(ID_RC_TYPE_INI), &data);

	CShareData_IO::IO_MainMenu(cProfile, &data, *this, false);
}

CommonSetting_TabBar::CommonSetting_TabBar() noexcept
{
	auto& lfIconTitle = m_lf;
	auto& nIconPointSize = m_nPointSize;
	SystemParametersInfoW(
		SPI_GETICONTITLELOGFONT,
		sizeof(LOGFONT),
		&lfIconTitle,
		NULL
	);
	nIconPointSize = lfIconTitle.lfHeight >= 0 ? lfIconTitle.lfHeight : DpiPixelsToPoints(-lfIconTitle.lfHeight, 10);

	// L"${w?【Grep】$h$:【アウトプット】$:$f$n$}${U?(更新)$}${R?(ビューモード)$:(上書き禁止)$}${M?【キーマクロの記録中】$}"
	wcscpy_s(m_szTabWndCaption, LS(STR_ERR_CSHAREDATA10));
}

CommonSetting_View::CommonSetting_View() noexcept
{
	auto& lf = m_lf;
	lf.lfHeight			= DpiPointsToPixels(-10);
	lf.lfWidth			= 0;
	lf.lfEscapement		= 0;
	lf.lfOrientation	= 0;
	lf.lfWeight			= 400;
	lf.lfItalic			= 0x0;
	lf.lfUnderline		= 0x0;
	lf.lfStrikeOut		= 0x0;
	lf.lfCharSet		= 0x80;
	lf.lfOutPrecision	= 0x3;
	lf.lfClipPrecision	= 0x2;
	lf.lfQuality		= 0x1;
	lf.lfPitchAndFamily	= 0x31;

	StringBufferW(lf.lfFaceName) = L"ＭＳ ゴシック";
}

CommonSetting_Window::CommonSetting_Window() noexcept
{
	// L"${w?$h$:アウトプット$:${I?$f$n$:$N$n$}$}${U?(更新)$} - $A $V ${R?(ビューモード)$:(上書き禁止)$}${M?  【キーマクロの記録中】$} $<profile>"
	wcscpy_s(m_szWindowCaptionActive, LS(STR_ERR_CSHAREDATA17));
	// L"${w?$h$:アウトプット$:$f$n$}${U?(更新)$} - $A $V ${R?(ビューモード)$:(上書き禁止)$}${M?  【キーマクロの記録中】$} $<profile>"
	wcscpy_s(m_szWindowCaptionInactive, LS(STR_ERR_CSHAREDATA18));
}