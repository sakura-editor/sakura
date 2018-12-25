/*! @file */
/*
	Copyright (C) 2008, kobake

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/

#include "StdAfx.h"
#include <wchar.h>
#include <algorithm>
#include "types/CType.h"
#include "doc/CEditDoc.h"
#include "doc/CDocOutline.h"
#include "doc/logic/CDocLine.h"
#include "outline/CFuncInfoArr.h"
#include "view/Colors/EColorIndexType.h"

/* TeX */
//Oct. 31, 2000 JEPRO TeX  ユーザに贈る
//Oct. 31, 2000 JEPRO TeX ユーザに贈る	//Mar. 10, 2001 JEPRO 追加
void CType_Tex::InitTypeConfigImp(STypeConfig* pType)
{
	//名前と拡張子
	_tcscpy( pType->m_szTypeName, _T("TeX") );
	_tcscpy( pType->m_szTypeExts, _T("tex,ltx,sty,bib,log,blg,aux,bbl,toc,lof,lot,idx,ind,glo") );

	//設定
	pType->m_cLineComment.CopyTo( 0, L"%", -1 );				/* 行コメントデリミタ */
	pType->m_eDefaultOutline = OUTLINE_TEX;						/* アウトライン解析方法 */
	pType->m_nKeyWordSetIdx[0] = 9;								/* キーワードセット */
	pType->m_nKeyWordSetIdx[1] = 10;							/* キーワードセット2 */	//Jan. 19, 2001 JEPRO
	pType->m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp = false;	//シングルクォーテーション文字列を色分け表示しない
	pType->m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp = false;	//ダブルクォーテーション文字列を色分け表示しない
}

/*! TeX アウトライン解析

	@author naoh
	@date 2003.07.21 naoh 新規作成
	@date 2005.01.03 naoh 「マ」などの"}"を含む文字に対する修正、prosperのslideに対応
*/

/** アウトライン解析の補助クラス */
template<int HierarchyCount>
class TagProcessor
{
	// 環境
	CFuncInfoArr &refFuncInfoArr;
	CLayoutMgr   &refLayoutMgr;
	// 定数
	const wchar_t* (&TagHierarchy)[HierarchyCount]; // 大きい構造から順に並べたタグの配列。\ マークは抜き。
	// 状態
	int tagDepth;      // 直前のタグの深さ。TagHierarchy[tagDepth] == 直前のタグ;
	int treeDepth;     // 直前のタグの「ツリーにおける」深さ。
	int serials[HierarchyCount]; // タグの各深さで割り振ったトピック番号の最大値を記憶しておく。
	// 作業場
	wchar_t szTopic[256];   // トピック番号 + トピックタイトル; (タグが * で終わっている場合はトピック番号を省略する)

public:
	TagProcessor(CFuncInfoArr& fia, CLayoutMgr& lmgr, const wchar_t* (&tagHierarchy)[HierarchyCount])
	: refFuncInfoArr(fia)
	, refLayoutMgr(lmgr)
	, TagHierarchy(tagHierarchy)
	, tagDepth(0)
	, treeDepth(-1)
	, serials()
	{
	}

	/** \tag{title} を受け取って、タグに対応したアウトライントピックを追加する。 */
	const wchar_t* operator()(
		CLogicInt nLineNumber,    // 行番号
		const wchar_t* pLine,     // 行文字列へのポインタ
		const wchar_t* pTag,      // \section{dddd} または \section*{dddd} の、s を指すポインタ。 
		const wchar_t* pTagEnd,   // \section{dddd} または \section*{dddd} の、{ を指すポインタ。
		const wchar_t* pTitle,    // \section{dddd} または \section*{dddd} の、先頭の d を指すポインタ。
		const wchar_t* pTitleEnd, // \section{dddd} または \section*{dddd} の、} を指すポインタ。
		const wchar_t* pLineEnd   // (改行を含む)行文字列末尾の文字の次を指すポインタ
	) {
		const bool bAddNumber = pTag < pTagEnd && pTagEnd[-1] != L'*'; // タグが * 付きかどうか = トピックに番号を付けるかどうか
		if (! bAddNumber) {
			pTagEnd -= 1;
		}

		// 現在のタグの深さ(depth)を求める。
		int depth; // Tag Depth
		for (depth = HierarchyCount - 1; 0 <= depth; --depth) {
			if (wcslen(TagHierarchy[depth]) == pTagEnd - pTag
			    && 0 == wcsncmp(TagHierarchy[depth], pTag, pTagEnd - pTag)
			) {
				break;
			}
		}
		if (depth < 0) { // 例外対応
			/*
			タグが begin なら prosper の slide の可能性も考慮して
			さらに {slide}{} まで読みとっておく。

			\begin{slide}{slide_title} なら、slide_title をタイトルにするということ。
			*/
			if (0 == wcsncmp(L"begin", pTag, pTagEnd - pTag)) {
				if (const wchar_t* pSlide = wcsstr(pTagEnd, L"{slide}")) {
					pTitle = wmemchr(pSlide + 7, L'{', pLineEnd - pSlide);
					pTitle = pTitle ? pTitle + 1 : pLineEnd;
					pTitleEnd = wmemchr(pTitle, L'}', pLineEnd - pTitle);
					pTitleEnd = pTitleEnd ? pTitleEnd : pLineEnd;
					if (pTitle < pTitleEnd) {
						depth = 0;
					}
				}
			}
		}
		if (depth < 0) {
			return pTitleEnd; // トピックタグではなかった。
		}
		assert(depth < HierarchyCount);

		/* 状態変数の更新
			現在のタグの深さ(depth)と直前のタグの深さ(tagDepth)の比較から
			1. ツリーアイテムの深さ(treeDepth)を増減する。
			2. トピック番号(serials[])を更新する。
			3. tagDepth を現在のタグの深さ(depth)で更新する。
		*/

		// 1. treeDepth を増減する。
		//    トピックツリーの仕様から treeDepth の増加幅は１に抑えたい。
		treeDepth += depth <= tagDepth ? (depth - tagDepth) : 1;
		for (int i = depth; i < tagDepth; ++i) {
		/*
			treeDepth の増加幅を１に抑えた結果としてトピックアイテムが
			本来の位置(tagDepth)より浅い位置に置かれていることがある。その補正。
		*/
			if (0 == serials[i]) {
				treeDepth += 1;
			}
		}
		if (treeDepth < 0) {
			treeDepth = 0; // 最初のトピックの場合や、最初のトピックが深い階層(section や subsection など)だったあとに、chapter が現れた場合など。
		}
		assert(treeDepth < HierarchyCount);

		// 2. トピック番号を更新する。
		serials[depth] += 1; // インクリメント
		for (int i = depth + 1; i <= tagDepth; ++i) {
			serials[i] = 0; // リセット
		}

		// 3. tagDepth を現在のタグの深さ(depth)で更新する。
		tagDepth = depth;

		// トピック文字列
		szTopic[0] = L'\0';
		wchar_t* pTopicEnd = szTopic; // 書き込みポインタ。

		// トピック文字列を作成する(1)。トビック番号をバッファに埋め込む。
		if (bAddNumber) {
			assert(4 * HierarchyCount + 2 <= _countof(szTopic)); // 4 はトピック番号「ddd.」のドットを含む最大桁数。+2 はヌル文字を含む " " の分。
			int i = 0;
			while (i <= tagDepth && serials[i] == 0) {
				i += 1; // "0." プリフィックスを表示しないようにスキップする。
			}
			for (; i <= tagDepth; ++i) {
				// "1.", "2.", "3.",..., "10.",..., "100.",...,"999.", "000.", "001.",...
				pTopicEnd += auto_sprintf(pTopicEnd, serials[i]/1000 ? L"%03d." : L"%d.", serials[i]%1000);
			}
			*pTopicEnd++ = L' ';
			*pTopicEnd   = L'\0';
		}
		assert(pTopicEnd < szTopic + _countof(szTopic));

		// トピック文字列を作成する(2)。タイトルをバッファに埋め込む。
		const ptrdiff_t copyLen = t_min(szTopic + _countof(szTopic) - 1 - pTopicEnd, pTitleEnd - pTitle);
		wmemcpy(pTopicEnd, pTitle, copyLen);
		pTopicEnd += copyLen;
		*pTopicEnd = L'\0';

		// トピックツリーにトピックを追加する。
		CLayoutPoint ptPos;
		refLayoutMgr.LogicToLayout(
			CLogicPoint(pTag - pLine, nLineNumber),
			&ptPos
		);
		refFuncInfoArr.AppendData(nLineNumber + CLogicInt(1), ptPos.GetY2() + CLayoutInt(1), szTopic, 0, treeDepth);

		// ループ継続
		return pTitleEnd;
	}
};
template<int HierarchyCount> inline
TagProcessor<HierarchyCount>
MakeTagProcessor(CFuncInfoArr& fia, CLayoutMgr& lmgr, const wchar_t* (&tagHierarchy)[HierarchyCount])
{
	return TagProcessor<HierarchyCount>(fia, lmgr, tagHierarchy);
}

/** アウトライン解析の補助クラス */
class TagIterator
{
	const CDocLineMgr& refDocLineMgr;

public:
	TagIterator(const CDocLineMgr& dlmgr)
	: refDocLineMgr(dlmgr)
	{}

	/** ドキュメント全体を先頭からスキャンして、見つけた \tag{title} を TagProcessor に渡す。 */
	template<int HierarchyCount>
	void each(TagProcessor<HierarchyCount>&& process)
	{
		const CLogicInt nLineCount = refDocLineMgr.GetLineCount();
		for (CLogicInt nLineLen, nLineNumber = CLogicInt(0); nLineNumber < nLineCount; ++nLineNumber) {
			const wchar_t* const pLine = refDocLineMgr.GetLine(nLineNumber)->GetDocLineStrWithEOL(&nLineLen);
			const wchar_t* const pLineEnd = pLine + nLineLen;
			if (! pLine) { // [EOF] のみの行。
				break;
			}

			const wchar_t *pTag      = 0, // \section{dddd} または \section*{dddd} の、s を指すポインタ。
			              *pTagEnd   = 0, // \section{dddd} または \section*{dddd} の、{ を指すポインタ。
			              *pTitle    = 0, // \section{dddd} または \section*{dddd} の、先頭の d を指すポインタ。
			              *pTitleEnd = 0; // \section{dddd} または \section*{dddd} の、} を指すポインタ。

			const wchar_t Meta[] = { L'\\', L'%' };
			const wchar_t* p = pLine;
			while (pLineEnd != (p = std::find_first_of(p, pLineEnd, Meta, Meta + _countof(Meta)))) {
				if (*p == L'%') {
					break; // コメントなので以降はいらない。
				}
				assert(*p == L'\\');

				// '\' の後ろから、'{' を目印にタグとタイトルを見つける。
				pTag      = p + 1;
				pTagEnd   = std::find(pTag, pLineEnd, L'{');
				pTitle    = pTagEnd < pLineEnd ? pTagEnd + 1 : pLineEnd;
				pTitleEnd = std::find(pTitle, pLineEnd, L'}');

				// タグの処理は任せる。
				if (pTag < pTagEnd && pTitle < pTitleEnd && pTitleEnd < pLineEnd) {
					p = process(nLineNumber, pLine, pTag, pTagEnd, pTitle, pTitleEnd, pLineEnd);
					if (p < pTag || pLineEnd < p) {
						return; // 無効な値であるか、無限ループのおそれがあるため中断。
					}
				} else {
					p += 1;
				}
			}
		}
	}
};

void CDocOutline::MakeTopicList_tex(CFuncInfoArr* pcFuncInfoArr)
{
	const wchar_t* TagHierarchy[] = {
		L"chapter",
		L"section",
		L"subsection",
		L"subsubsection"
	};
	TagIterator(m_pcDocRef->m_cDocLineMgr).each(
		MakeTagProcessor(*pcFuncInfoArr, m_pcDocRef->m_cLayoutMgr, TagHierarchy)
	);
}

const wchar_t* g_ppszKeywordsTEX[] = {
//Nov. 20, 2000 JEPRO	大幅追加 & 若干修正・削除 --ほとんどコマンドのみ
	L"error",
	L"Warning",
//			"center",
//			"document",
//			"enumerate",
//			"eqnarray",
//			"figure",
//			"itemize",
//			"minipage",
//			"tabbing",
//			"table",
//			"tabular",
//			"\\!",
//			"\\#",
//			"\\%",
//			"\\&",
//			"\\(",
//			"\\)",
//			"\\+",
//			"\\,",
//			"\\-",
//			"\\:",
//			"\\;",
//			"\\<",
//			"\\=",
//			"\\>",
	L"\\aa",
	L"\\AA",
	L"\\acute",
	L"\\addcontentsline",
	L"\\addtocounter",
	L"\\addtolength",
	L"\\ae",
	L"\\AE",
	L"\\aleph",
	L"\\alpha",
	L"\\alph",
	L"\\Alph",
	L"\\and",
	L"\\angle",
	L"\\appendix",
	L"\\approx",
	L"\\arabic",
	L"\\arccos",
	L"\\arctan",
	L"\\arg",
	L"\\arrayrulewidth",
	L"\\arraystretch",
	L"\\ast",
	L"\\atop",
	L"\\author",
	L"\\b",
	L"\\backslash",
	L"\\bar",
	L"\\baselineskip",
	L"\\baselinestretch",
	L"\\begin",
	L"\\beta",
	L"\\bf",
	L"\\bibitem",
	L"\\bibliography",
	L"\\bibliographystyle",
	L"\\big",
	L"\\Big",
	L"\\bigcap",
	L"\\bigcirc",
	L"\\bigcup",
	L"\\bigg",
	L"\\Bigg",
	L"\\Biggl",
	L"\\Biggm",
	L"\\biggl",
	L"\\biggm",
	L"\\biggr",
	L"\\Biggr",
	L"\\bigl",
	L"\\bigm",
	L"\\Bigm",
	L"\\Bigl",
	L"\\bigodot",
	L"\\bigoplus",
	L"\\bigotimes",
	L"\\bigr",
	L"\\Bigr",
	L"\\bigskip",
	L"\\bigtriangledown",
	L"\\bigtriangleup",
	L"\\boldmath",
	L"\\bot",
	L"\\Box",
	L"\\brace",
	L"\\breve",
	L"\\bullet",
	L"\\bye",
	L"\\c",
	L"\\cal",
	L"\\cap",
	L"\\caption",
	L"\\cc",
	L"\\cdot",
	L"\\cdots",
	L"\\centering",
	L"\\chapter",
	L"\\check",
	L"\\chi",
	L"\\choose",
	L"\\circ",
	L"\\circle",
	L"\\cite",
	L"\\clearpage",
	L"\\cline",
	L"\\closing",
	L"\\clubsuit",
	L"\\colon",
	L"\\columnsep",
	L"\\columnseprule",
	L"\\cong",
	L"\\cot",
	L"\\coth",
	L"\\cr",
	L"\\cup",
	L"\\d",
	L"\\dag",
	L"\\dagger",
	L"\\date",
	L"\\dashbox",
	L"\\ddag",
	L"\\ddot",
	L"\\ddots",
	L"\\def",
	L"\\deg",
	L"\\delta",
	L"\\Delta",
	L"\\det",
	L"\\diamond",
	L"\\diamondsuit",
	L"\\dim",
	L"\\displaystyle",
	L"\\documentclass",
	L"\\documentstyle",
	L"\\dot",
	L"\\doteq",
	L"\\dotfill",
	L"\\Downarrow",
	L"\\downarrow",
	L"\\ell",
	L"\\em",
	L"\\emptyset",
	L"\\encl",
	L"\\end",
	L"\\enspace",
	L"\\enskip",
	L"\\epsilon",
	L"\\eqno",
	L"\\equiv",
	L"\\evensidemargin",
	L"\\eta",
	L"\\exists",
	L"\\exp",
	L"\\fbox",
	L"\\fboxrule",
	L"\\flat",
	L"\\footnote",
	L"\\footnotesize",
	L"\\forall",
	L"\\frac",
	L"\\frame",
	L"\\framebox",
	L"\\gamma",
	L"\\Gamma",
	L"\\gcd",
	L"\\ge",
	L"\\geq",
	L"\\gets",
	L"\\gg",
	L"\\grave",
	L"\\gt",
	L"\\H",
	L"\\hat",
	L"\\hbar",
	L"\\hbox",
	L"\\headsep",
	L"\\heartsuit",
	L"\\hfil",
	L"\\hfill",
	L"\\hline",
	L"\\hom",
	L"\\hrulefill",
	L"\\hskip",
	L"\\hspace",
	L"\\hspace*",
	L"\\huge",
	L"\\Huge",
	L"\\i",
	L"\\Im",
	L"\\imath",
	L"\\in",
	L"\\include",
	L"\\includegraphics",
	L"\\includeonly",
	L"\\indent",
	L"\\index",
	L"\\inf",
	L"\\infty",
	L"\\input",
	L"\\int",
	L"\\iota",
	L"\\it",
	L"\\item",
	L"\\itemsep",
	L"\\j",
	L"\\jmath",
	L"\\kappa",
	L"\\ker",
	L"\\kern",
	L"\\kill",
	L"\\l",
	L"\\L",
	L"\\label",
	L"\\lambda",
	L"\\Lambda",
	L"\\land",
	L"\\langle",
	L"\\large",
	L"\\Large",
	L"\\LARGE",
	L"\\LaTeX",
	L"\\LaTeXe",
	L"\\lceil",
	L"\\ldots",
	L"\\le",
	L"\\leftarrow",
	L"\\Leftarrow",
	L"\\lefteqn",
	L"\\leftharpoondown",
	L"\\leftharpoonup",
	L"\\leftmargin",
	L"\\leftrightarrow",
	L"\\Leftrightarrow",
	L"\\leq",
	L"\\leqno",
	L"\\lfloor",
	L"\\lg",
	L"\\lim",
	L"\\liminf",
	L"\\limsup",
	L"\\line",
	L"\\linebreak",
	L"\\linewidth",
	L"\\listoffigures",
	L"\\listoftables",
	L"\\ll",
	L"\\llap",
	L"\\ln",
	L"\\lnot",
	L"\\log",
	L"\\longleftarrow",
	L"\\Longleftarrow",
	L"\\longleftrightarrow",
	L"\\Longleftrightarrow",
	L"\\longrightarrow",
	L"\\Longrightarrow",
	L"\\lor",
	L"\\lower",
	L"\\magstep",
	L"\\makeatletter",
	L"\\makeatother",
	L"\\makebox",
	L"\\makeindex",
	L"\\maketitle",
	L"\\makelabels",
	L"\\mathop",
	L"\\mapsto",
	L"\\markboth",
	L"\\markright",
	L"\\mathstrut",
	L"\\max",
	L"\\mbox",
	L"\\mc",
	L"\\medskip",
	L"\\mid",
	L"\\min",
	L"\\mit",
	L"\\mp",
	L"\\mu",
	L"\\multicolumn",
	L"\\multispan",
	L"\\multiput",
	L"\\nabla",
	L"\\natural",
	L"\\ne",
	L"\\neg",
	L"\\nearrow",
	L"\\nwarrow",
	L"\\neq",
	L"\\newblock",
	L"\\newcommand",
	L"\\newenvironment",
	L"\\newfont",
	L"\\newlength",
	L"\\newline",
	L"\\newpage",
	L"\\newtheorem",
	L"\\ni",
	L"\\noalign",
	L"\\noindent",
	L"\\nolimits",
	L"\\nolinebreak",
	L"\\nonumber",
	L"\\nopagebreak",
	L"\\normalsize",
	L"\\not",
	L"\\notice",
	L"\\notin",
	L"\\nu",
	L"\\o",
	L"\\O",
	L"\\oddsidemargin",
	L"\\odot",
	L"\\oe",
	L"\\OE",
	L"\\oint",
	L"\\Omega",
	L"\\omega",
	L"\\ominus",
	L"\\oplus",
	L"\\opening",
	L"\\otimes",
	L"\\owns",
	L"\\overleftarrow",
	L"\\overline",
	L"\\overrightarrow",
	L"\\overvrace",
	L"\\oval",
	L"\\P",
	L"\\pagebreak",
	L"\\pagenumbering",
	L"\\pageref",
	L"\\pagestyle",
	L"\\par",
	L"\\parallel",
	L"\\paragraph",
	L"\\parbox",
	L"\\parindent",
	L"\\parskip",
	L"\\partial",
	L"\\perp",
	L"\\phi",
	L"\\Phi",
	L"\\pi",
	L"\\Pi",
	L"\\pm",
	L"\\Pr",
	L"\\prime",
	L"\\printindex",
	L"\\prod",
	L"\\propto",
	L"\\ps",
	L"\\psi",
	L"\\Psi",
	L"\\put",
	L"\\qquad",
	L"\\quad",
	L"\\raisebox",
	L"\\rangle",
	L"\\rceil",
	L"\\Re",
	L"\\ref",
	L"\\renewcommand",
	L"\\renewenvironment",
	L"\\rfloor",
	L"\\rho",
	L"\\right",
	L"\\rightarrow",
	L"\\Rightarrow",
	L"\\rightharpoondown",
	L"\\rightharpoonup",
	L"\\rightleftharpoonup",
	L"\\rightmargin",
	L"\\rm",
	L"\\rule",
	L"\\roman",
	L"\\Roman",
	L"\\S",
	L"\\samepage",
	L"\\sb",
	L"\\sc",
	L"\\scriptsize",
	L"\\scriptscriptstyle",
	L"\\scriptstyle",
	L"\\searrow",
	L"\\sec",
	L"\\section",
	L"\\setcounter",
	L"\\setlength",
	L"\\settowidth",
	L"\\setminus",
	L"\\sf",
	L"\\sharp",
	L"\\sigma",
	L"\\Sigma",
	L"\\signature",
	L"\\sim",
	L"\\simeq",
	L"\\sin",
	L"\\sinh",
	L"\\sl",
	L"\\sloppy",
	L"\\small",
	L"\\smash",
	L"\\smallskip",
	L"\\sp",
	L"\\spadesuit",
	L"\\special",
	L"\\sqrt",
	L"\\ss",
	L"\\star",
	L"\\stackrel",
	L"\\strut",
	L"\\subparagraph",
	L"\\subsection",
	L"\\subset",
	L"\\subseteq",
	L"\\subsubsection",
	L"\\sum",
	L"\\sup",
	L"\\supset",
	L"\\supseteq",
	L"\\swarrow",
	L"\\t",
	L"\\tableofcontents",
	L"\\tan",
	L"\\tanh",
	L"\\tau",
	L"\\TeX",
	L"\\textbf",
	L"\\textgreater",
	L"\\textgt",
	L"\\textheight",
	L"\\textit",
	L"\\textless",
	L"\\textmc",
	L"\\textrm",
	L"\\textsc",
	L"\\textsf",
	L"\\textsl",
	L"\\textstyle",
	L"\\texttt",
	L"\\textwidth",
	L"\\thanks",
	L"\\thebibliography",
	L"\\theequation",
	L"\\thepage",
	L"\\thesection",
	L"\\theta",
	L"\\Theta",
	L"\\thicklines",
	L"\\thinlines",
	L"\\thinspace",
	L"\\thisepage",
	L"\\thisepagestyle",
	L"\\tie",
	L"\\tilde",
	L"\\times",
	L"\\tiny",
	L"\\title",
	L"\\titlepage",
	L"\\to",
	L"\\toaddress",
	L"\\topmargin",
	L"\\triangle",
	L"\\tt",
	L"\\twocolumn",
	L"\\u",
	L"\\underline",
	L"\\undervrace",
	L"\\unitlength",
	L"\\Uparrow",
	L"\\uparrow",
	L"\\updownarrow",
	L"\\Updownarrow",
	L"\\uplus",
	L"\\upsilon",
	L"\\Upsilon",
	L"\\usepackage",
	L"\\v",
	L"\\varepsilon",
	L"\\varphi",
	L"\\varpi",
	L"\\varrho",
	L"\\varsigma",
	L"\\vartheta",
	L"\\vbox",
	L"\\vcenter",
	L"\\vec",
	L"\\vector",
	L"\\vee",
	L"\\verb",
	L"\\verb*",
	L"\\verbatim",
	L"\\vert",
	L"\\Vert",
	L"\\vfil",
	L"\\vfill",
	L"\\vrule",
	L"\\vskip",
	L"\\vspace",
	L"\\vspace*",
	L"\\wedge",
	L"\\widehat",
	L"\\widetilde",
	L"\\wp",
	L"\\wr",
	L"\\wrapfigure",
	L"\\xi",
	L"\\Xi",
	L"\\zeta"//,
//			"\\[",
//			"\\\"",
//			"\\\'",
//			"\\\\",
//			"\\]",
//			"\\^",
//			"\\_",
//			"\\`",
//			"\\{",
//			"\\|",
//			"\\}",
//			"\\~",
};
int g_nKeywordsTEX = _countof(g_ppszKeywordsTEX);

//Jan. 19, 2001 JEPRO	TeX のキーワード2として新規追加 & 一部復活 --環境コマンドとオプション名が中心
const wchar_t* g_ppszKeywordsTEX2[] = {
	//	環境コマンド
	//Jan. 19, 2001 JEPRO 本当は{}付きでキーワードにしたかったが単語として認識してくれないので止めた
	L"abstract",
	L"array",
	L"center",
	L"description",
	L"document",
	L"displaymath",
	L"em",
	L"enumerate",
	L"eqnarray",
	L"eqnarray*",
	L"equation",
	L"figure",
	L"figure*",
	L"floatingfigure",
	L"flushleft",
	L"flushright",
	L"itemize",
	L"letter",
	L"list",
	L"math",
	L"minipage",
	L"multicols",
	L"namelist",
	L"picture",
	L"quotation",
	L"quote",
	L"sloppypar",
	L"subeqnarray",
	L"subeqnarray*",
	L"subequations",
	L"subfigure",
	L"tabbing",
	L"table",
	L"table*",
	L"tabular",
	L"tabular*",
	L"tatepage",
	L"thebibliography",
	L"theindex",
	L"titlepage",
	L"trivlist",
	L"verbatim",
	L"verbatim*",
	L"verse",
	L"wrapfigure",
	//	スタイルオプション
	L"a4",
	L"a4j",
	L"a5",
	L"a5j",
	L"Alph",
	L"alph",
	L"annote",
	L"arabic",
	L"b4",
	L"b4j",
	L"b5",
	L"b5j",
	L"bezier",
	L"booktitle",
	L"boxedminipage",
	L"boxit",
//		"bp",
//		"cm",
	L"dbltopnumber",
//		"dd",
	L"eclepsf",
	L"eepic",
	L"enumi",
	L"enumii",
	L"enumiii",
	L"enumiv",
	L"epic",
	L"epsbox",
	L"epsf",
	L"fancybox",
	L"fancyheadings",
	L"fleqn",
	L"footnote",
	L"howpublished",
	L"jabbrv",
	L"jalpha",
//		"article",
	L"jarticle",
	L"jsarticle",
//		"book",
	L"jbook",
	L"jsbook",
//		"letter",
	L"jletter",
//		"plain",
	L"jplain",
//		"report",
	L"jreport",
	L"jtwocolumn",
	L"junsrt",
	L"leqno",
	L"makeidx",
	L"markboth",
	L"markright",
//		"mm",
	L"multicol",
	L"myheadings",
	L"openbib",
//		"pc",
//		"pt",
	L"secnumdepth",
//		"sp",
	L"titlepage",
	L"tjarticle",
	L"topnumber",
	L"totalnumber",
	L"twocolumn",
	L"twoside",
	L"yomi"//,
//		"zh",
//		"zw"
};
int g_nKeywordsTEX2 = _countof(g_ppszKeywordsTEX2);

