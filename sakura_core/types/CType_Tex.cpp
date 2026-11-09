/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include <wchar.h>
#include <algorithm>
#include "types/CType.h"
#include "doc/CEditDoc.h"
#include "doc/CDocOutline.h"
#include "doc/logic/CDocLine.h"
#include "outline/CFuncInfoArr.h"
#include "view/colors/EColorIndexType.h"

/* TeX */
//Oct. 31, 2000 JEPRO TeX  ユーザーに贈る
//Oct. 31, 2000 JEPRO TeX ユーザーに贈る	//Mar. 10, 2001 JEPRO 追加
void CType_Tex::InitTypeConfigImp(STypeConfig* pType)
{
	//名前と拡張子
	wcscpy( pType->m_szTypeName, LTEXT("TeX") );
	wcscpy( pType->m_szTypeExts, LTEXT("tex,ltx,sty,bib,log,blg,aux,bbl,toc,lof,lot,idx,ind,glo") );

	//設定
	pType->m_cLineComment.CopyTo( 0, LTEXT("%"), -1 );				/* 行コメントデリミタ */
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
template<size_t HierarchyCount>
class TagProcessor
{
	// 環境
	CFuncInfoArr &refFuncInfoArr;
	CLayoutMgr   &refLayoutMgr;
	// 定数
	std::array<LPCWSTR, HierarchyCount> TagHierarchy; // 大きい構造から順に並べたタグの配列。\ マークは抜き。
	// 状態
	int tagDepth = 0;      // 直前のタグの深さ。TagHierarchy[tagDepth] == 直前のタグ;
	int treeDepth = -1;     // 直前のタグの「ツリーにおける」深さ。
	std::array<int, HierarchyCount> serials{}; // タグの各深さで割り振ったトピック番号の最大値を記憶しておく。
	// 作業場
	StaticString<256> szTopic;   // トピック番号 + トピックタイトル; (タグが * で終わっている場合はトピック番号を省略する)

public:
	TagProcessor(CFuncInfoArr& fia, CLayoutMgr& lmgr, const std::array<LPCWSTR, HierarchyCount>& tagHierarchy)
	: refFuncInfoArr(fia)
	, refLayoutMgr(lmgr)
	, TagHierarchy(tagHierarchy)
	{
	}

	/** \tag{title} を受け取って、タグに対応したアウトライントピックを追加する。 */
	const WCHAR* operator()(
		CLogicInt nLineNumber,    // 行番号
		const WCHAR* pLine,     // 行文字列へのポインタ
		const WCHAR* pTag,      // \section{dddd} または \section*{dddd} の、s を指すポインタ。 
		const WCHAR* pTagEnd,   // \section{dddd} または \section*{dddd} の、{ を指すポインタ。
		const WCHAR* pTitle,    // \section{dddd} または \section*{dddd} の、先頭の d を指すポインタ。
		const WCHAR* pTitleEnd, // \section{dddd} または \section*{dddd} の、} を指すポインタ。
		const WCHAR* pLineEnd   // (改行を含む)行文字列末尾の文字の次を指すポインタ
	) {
		const bool bAddNumber = pTag < pTagEnd && pTagEnd[-1] != LCHAR('*'); // タグが * 付きかどうか = トピックに番号を付けるかどうか
		if (! bAddNumber) {
			pTagEnd -= 1;
		}

		// 現在のタグの深さ(depth)を求める。
		int depth; // Tag Depth
		for (depth = HierarchyCount - 1; 0 <= depth; --depth) {
			if (ptrdiff_t(::wcslen(TagHierarchy[depth])) == pTagEnd - pTag
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
			if (0 == wcsncmp(LTEXT("begin"), pTag, pTagEnd - pTag)) {
				if (const WCHAR* pSlide = wcsstr(pTagEnd, LTEXT("{slide}"))) {
					pTitle = wmemchr(pSlide + 7, LCHAR('{'), pLineEnd - pSlide);
					pTitle = pTitle ? pTitle + 1 : pLineEnd;
					pTitleEnd = wmemchr(pTitle, LCHAR('}'), pLineEnd - pTitle);
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
			if (i < int(std::size(serials)) && 0 == serials[i]) {
				treeDepth += 1;
			}
		}
		if (treeDepth < 0) {
			treeDepth = 0; // 最初のトピックの場合や、最初のトピックが深い階層(section や subsection など)だったあとに、chapter が現れた場合など。
		}
		assert(treeDepth < HierarchyCount);

		// 2. トピック番号を更新する。
		serials[depth] += 1; // インクリメント
		for (int i = depth + 1; i <= tagDepth && i < int(std::size(serials)); ++i) {
			serials[i] = 0; // リセット
		}

		// 3. tagDepth を現在のタグの深さ(depth)で更新する。
		tagDepth = depth;

		// トピック文字列
		szTopic[0] = LCHAR('\0');
		WCHAR* pTopicEnd = szTopic; // 書き込みポインタ。

		// トピック文字列を作成する(1)。トビック番号をバッファに埋め込む。
		if (bAddNumber) {
			assert(4 * HierarchyCount + 2 <= std::size(szTopic)); // 4 はトピック番号「ddd.」のドットを含む最大桁数。+2 はヌル文字を含む " " の分。
			int i = 0;
			while (i <= tagDepth && serials[i] == 0) {
				i += 1; // "0." プリフィックスを表示しないようにスキップする。
			}
			for (; i <= tagDepth && i < int(std::size(serials)); ++i) {
				// "1.", "2.", "3.",..., "10.",..., "100.",...,"999.", "000.", "001.",...
				pTopicEnd += auto_sprintf(pTopicEnd, serials[i]/1000 ? LTEXT("%03d.") : LTEXT("%d."), serials[i]%1000);
			}
			*pTopicEnd++ = LCHAR(' ');
			*pTopicEnd   = LCHAR('\0');
		}
		assert(pTopicEnd < szTopic + std::size(szTopic));

		// トピック文字列を作成する(2)。タイトルをバッファに埋め込む。
		const auto copyLen = std::min<ptrdiff_t>(szTopic + std::size(szTopic) - 1 - pTopicEnd, pTitleEnd - pTitle);
		wmemcpy(pTopicEnd, pTitle, copyLen);
		pTopicEnd += copyLen;
		*pTopicEnd = LCHAR('\0');

		// トピックツリーにトピックを追加する。
		CLayoutPoint ptPos;
		refLayoutMgr.LogicToLayout(
			CLogicPoint(int(pTag - pLine), nLineNumber),
			&ptPos
		);
		refFuncInfoArr.AppendData(nLineNumber + CLogicInt(1), ptPos.GetY2() + CLayoutInt(1), szTopic, 0, treeDepth);

		// ループ継続
		return pTitleEnd;
	}
};
template<size_t HierarchyCount> inline
TagProcessor<HierarchyCount>
MakeTagProcessor(CFuncInfoArr& fia, CLayoutMgr& lmgr, const std::array<LPCWSTR, HierarchyCount>& tagHierarchy)
{
	return TagProcessor<HierarchyCount>(fia, lmgr, tagHierarchy);
}

/** アウトライン解析の補助クラス */
class TagIterator
{
	const CDocLineMgr& refDocLineMgr;

public:
	explicit TagIterator(const CDocLineMgr& dlmgr)
	: refDocLineMgr(dlmgr)
	{}

	/** ドキュメント全体を先頭からスキャンして、見つけた \tag{title} を TagProcessor に渡す。 */
	template<size_t HierarchyCount>
	void each(TagProcessor<HierarchyCount>&& process)
	{
		const CLogicInt nLineCount = refDocLineMgr.GetLineCount();
		for (CLogicInt nLineLen, nLineNumber = CLogicInt(0); nLineNumber < nLineCount; ++nLineNumber) {
			const WCHAR* const pLine = refDocLineMgr.GetLine(nLineNumber)->GetDocLineStrWithEOL(&nLineLen);
			const WCHAR* const pLineEnd = pLine + nLineLen;
			if (! pLine) { // [EOF] のみの行。
				break;
			}

			const WCHAR *pTag      = nullptr; // \section{dddd} または \section*{dddd} の、s を指すポインタ。
			const WCHAR *pTagEnd   = nullptr; // \section{dddd} または \section*{dddd} の、{ を指すポインタ。
			const WCHAR *pTitle    = nullptr; // \section{dddd} または \section*{dddd} の、先頭の d を指すポインタ。
			const WCHAR *pTitleEnd = nullptr; // \section{dddd} または \section*{dddd} の、} を指すポインタ。

			const WCHAR Meta[] = { LCHAR('\\'), LCHAR('%') };
			const WCHAR* p = pLine;
			while (pLineEnd != (p = std::find_first_of(p, pLineEnd, Meta, Meta + int(std::size(Meta))))) {
				if (*p == LCHAR('%')) {
					break; // コメントなので以降はいらない。
				}
				assert(*p == LCHAR('\\'));

				// '\' の後ろから、'{' を目印にタグとタイトルを見つける。
				pTag      = p + 1;
				pTagEnd   = std::find(pTag, pLineEnd, LCHAR('{'));
				pTitle    = pTagEnd < pLineEnd ? pTagEnd + 1 : pLineEnd;
				pTitleEnd = std::find(pTitle, pLineEnd, LCHAR('}'));

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
	const std::array TagHierarchy = {
		LTEXT("chapter"),
		LTEXT("section"),
		LTEXT("subsection"),
		LTEXT("subsubsection")
	};
	TagIterator(m_pcDocRef->m_cDocLineMgr).each(
		MakeTagProcessor(*pcFuncInfoArr, m_pcDocRef->m_cLayoutMgr, TagHierarchy)
	);
}

const WCHAR* g_ppszKeywordsTEX[] = {
//Nov. 20, 2000 JEPRO	大幅追加 & 若干修正・削除 --ほとんどコマンドのみ
	LTEXT("error"),
	LTEXT("Warning"),
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
	LTEXT("\\aa"),
	LTEXT("\\AA"),
	LTEXT("\\acute"),
	LTEXT("\\addcontentsline"),
	LTEXT("\\addtocounter"),
	LTEXT("\\addtolength"),
	LTEXT("\\ae"),
	LTEXT("\\AE"),
	LTEXT("\\aleph"),
	LTEXT("\\alpha"),
	LTEXT("\\alph"),
	LTEXT("\\Alph"),
	LTEXT("\\and"),
	LTEXT("\\angle"),
	LTEXT("\\appendix"),
	LTEXT("\\approx"),
	LTEXT("\\arabic"),
	LTEXT("\\arccos"),
	LTEXT("\\arctan"),
	LTEXT("\\arg"),
	LTEXT("\\arrayrulewidth"),
	LTEXT("\\arraystretch"),
	LTEXT("\\ast"),
	LTEXT("\\atop"),
	LTEXT("\\author"),
	LTEXT("\\b"),
	LTEXT("\\backslash"),
	LTEXT("\\bar"),
	LTEXT("\\baselineskip"),
	LTEXT("\\baselinestretch"),
	LTEXT("\\begin"),
	LTEXT("\\beta"),
	LTEXT("\\bf"),
	LTEXT("\\bibitem"),
	LTEXT("\\bibliography"),
	LTEXT("\\bibliographystyle"),
	LTEXT("\\big"),
	LTEXT("\\Big"),
	LTEXT("\\bigcap"),
	LTEXT("\\bigcirc"),
	LTEXT("\\bigcup"),
	LTEXT("\\bigg"),
	LTEXT("\\Bigg"),
	LTEXT("\\Biggl"),
	LTEXT("\\Biggm"),
	LTEXT("\\biggl"),
	LTEXT("\\biggm"),
	LTEXT("\\biggr"),
	LTEXT("\\Biggr"),
	LTEXT("\\bigl"),
	LTEXT("\\bigm"),
	LTEXT("\\Bigm"),
	LTEXT("\\Bigl"),
	LTEXT("\\bigodot"),
	LTEXT("\\bigoplus"),
	LTEXT("\\bigotimes"),
	LTEXT("\\bigr"),
	LTEXT("\\Bigr"),
	LTEXT("\\bigskip"),
	LTEXT("\\bigtriangledown"),
	LTEXT("\\bigtriangleup"),
	LTEXT("\\boldmath"),
	LTEXT("\\bot"),
	LTEXT("\\Box"),
	LTEXT("\\brace"),
	LTEXT("\\breve"),
	LTEXT("\\bullet"),
	LTEXT("\\bye"),
	LTEXT("\\c"),
	LTEXT("\\cal"),
	LTEXT("\\cap"),
	LTEXT("\\caption"),
	LTEXT("\\cc"),
	LTEXT("\\cdot"),
	LTEXT("\\cdots"),
	LTEXT("\\centering"),
	LTEXT("\\chapter"),
	LTEXT("\\check"),
	LTEXT("\\chi"),
	LTEXT("\\choose"),
	LTEXT("\\circ"),
	LTEXT("\\circle"),
	LTEXT("\\cite"),
	LTEXT("\\clearpage"),
	LTEXT("\\cline"),
	LTEXT("\\closing"),
	LTEXT("\\clubsuit"),
	LTEXT("\\colon"),
	LTEXT("\\columnsep"),
	LTEXT("\\columnseprule"),
	LTEXT("\\cong"),
	LTEXT("\\cot"),
	LTEXT("\\coth"),
	LTEXT("\\cr"),
	LTEXT("\\cup"),
	LTEXT("\\d"),
	LTEXT("\\dag"),
	LTEXT("\\dagger"),
	LTEXT("\\date"),
	LTEXT("\\dashbox"),
	LTEXT("\\ddag"),
	LTEXT("\\ddot"),
	LTEXT("\\ddots"),
	LTEXT("\\def"),
	LTEXT("\\deg"),
	LTEXT("\\delta"),
	LTEXT("\\Delta"),
	LTEXT("\\det"),
	LTEXT("\\diamond"),
	LTEXT("\\diamondsuit"),
	LTEXT("\\dim"),
	LTEXT("\\displaystyle"),
	LTEXT("\\documentclass"),
	LTEXT("\\documentstyle"),
	LTEXT("\\dot"),
	LTEXT("\\doteq"),
	LTEXT("\\dotfill"),
	LTEXT("\\Downarrow"),
	LTEXT("\\downarrow"),
	LTEXT("\\ell"),
	LTEXT("\\em"),
	LTEXT("\\emptyset"),
	LTEXT("\\encl"),
	LTEXT("\\end"),
	LTEXT("\\enspace"),
	LTEXT("\\enskip"),
	LTEXT("\\epsilon"),
	LTEXT("\\eqno"),
	LTEXT("\\equiv"),
	LTEXT("\\evensidemargin"),
	LTEXT("\\eta"),
	LTEXT("\\exists"),
	LTEXT("\\exp"),
	LTEXT("\\fbox"),
	LTEXT("\\fboxrule"),
	LTEXT("\\flat"),
	LTEXT("\\footnote"),
	LTEXT("\\footnotesize"),
	LTEXT("\\forall"),
	LTEXT("\\frac"),
	LTEXT("\\frame"),
	LTEXT("\\framebox"),
	LTEXT("\\gamma"),
	LTEXT("\\Gamma"),
	LTEXT("\\gcd"),
	LTEXT("\\ge"),
	LTEXT("\\geq"),
	LTEXT("\\gets"),
	LTEXT("\\gg"),
	LTEXT("\\grave"),
	LTEXT("\\gt"),
	LTEXT("\\H"),
	LTEXT("\\hat"),
	LTEXT("\\hbar"),
	LTEXT("\\hbox"),
	LTEXT("\\headsep"),
	LTEXT("\\heartsuit"),
	LTEXT("\\hfil"),
	LTEXT("\\hfill"),
	LTEXT("\\hline"),
	LTEXT("\\hom"),
	LTEXT("\\hrulefill"),
	LTEXT("\\hskip"),
	LTEXT("\\hspace"),
	LTEXT("\\hspace*"),
	LTEXT("\\huge"),
	LTEXT("\\Huge"),
	LTEXT("\\i"),
	LTEXT("\\Im"),
	LTEXT("\\imath"),
	LTEXT("\\in"),
	LTEXT("\\include"),
	LTEXT("\\includegraphics"),
	LTEXT("\\includeonly"),
	LTEXT("\\indent"),
	LTEXT("\\index"),
	LTEXT("\\inf"),
	LTEXT("\\infty"),
	LTEXT("\\input"),
	LTEXT("\\int"),
	LTEXT("\\iota"),
	LTEXT("\\it"),
	LTEXT("\\item"),
	LTEXT("\\itemsep"),
	LTEXT("\\j"),
	LTEXT("\\jmath"),
	LTEXT("\\kappa"),
	LTEXT("\\ker"),
	LTEXT("\\kern"),
	LTEXT("\\kill"),
	LTEXT("\\l"),
	LTEXT("\\L"),
	LTEXT("\\label"),
	LTEXT("\\lambda"),
	LTEXT("\\Lambda"),
	LTEXT("\\land"),
	LTEXT("\\langle"),
	LTEXT("\\large"),
	LTEXT("\\Large"),
	LTEXT("\\LARGE"),
	LTEXT("\\LaTeX"),
	LTEXT("\\LaTeXe"),
	LTEXT("\\lceil"),
	LTEXT("\\ldots"),
	LTEXT("\\le"),
	LTEXT("\\leftarrow"),
	LTEXT("\\Leftarrow"),
	LTEXT("\\lefteqn"),
	LTEXT("\\leftharpoondown"),
	LTEXT("\\leftharpoonup"),
	LTEXT("\\leftmargin"),
	LTEXT("\\leftrightarrow"),
	LTEXT("\\Leftrightarrow"),
	LTEXT("\\leq"),
	LTEXT("\\leqno"),
	LTEXT("\\lfloor"),
	LTEXT("\\lg"),
	LTEXT("\\lim"),
	LTEXT("\\liminf"),
	LTEXT("\\limsup"),
	LTEXT("\\line"),
	LTEXT("\\linebreak"),
	LTEXT("\\linewidth"),
	LTEXT("\\listoffigures"),
	LTEXT("\\listoftables"),
	LTEXT("\\ll"),
	LTEXT("\\llap"),
	LTEXT("\\ln"),
	LTEXT("\\lnot"),
	LTEXT("\\log"),
	LTEXT("\\longleftarrow"),
	LTEXT("\\Longleftarrow"),
	LTEXT("\\longleftrightarrow"),
	LTEXT("\\Longleftrightarrow"),
	LTEXT("\\longrightarrow"),
	LTEXT("\\Longrightarrow"),
	LTEXT("\\lor"),
	LTEXT("\\lower"),
	LTEXT("\\magstep"),
	LTEXT("\\makeatletter"),
	LTEXT("\\makeatother"),
	LTEXT("\\makebox"),
	LTEXT("\\makeindex"),
	LTEXT("\\maketitle"),
	LTEXT("\\makelabels"),
	LTEXT("\\mathop"),
	LTEXT("\\mapsto"),
	LTEXT("\\markboth"),
	LTEXT("\\markright"),
	LTEXT("\\mathstrut"),
	LTEXT("\\max"),
	LTEXT("\\mbox"),
	LTEXT("\\mc"),
	LTEXT("\\medskip"),
	LTEXT("\\mid"),
	LTEXT("\\min"),
	LTEXT("\\mit"),
	LTEXT("\\mp"),
	LTEXT("\\mu"),
	LTEXT("\\multicolumn"),
	LTEXT("\\multispan"),
	LTEXT("\\multiput"),
	LTEXT("\\nabla"),
	LTEXT("\\natural"),
	LTEXT("\\ne"),
	LTEXT("\\neg"),
	LTEXT("\\nearrow"),
	LTEXT("\\nwarrow"),
	LTEXT("\\neq"),
	LTEXT("\\newblock"),
	LTEXT("\\newcommand"),
	LTEXT("\\newenvironment"),
	LTEXT("\\newfont"),
	LTEXT("\\newlength"),
	LTEXT("\\newline"),
	LTEXT("\\newpage"),
	LTEXT("\\newtheorem"),
	LTEXT("\\ni"),
	LTEXT("\\noalign"),
	LTEXT("\\noindent"),
	LTEXT("\\nolimits"),
	LTEXT("\\nolinebreak"),
	LTEXT("\\nonumber"),
	LTEXT("\\nopagebreak"),
	LTEXT("\\normalsize"),
	LTEXT("\\not"),
	LTEXT("\\notice"),
	LTEXT("\\notin"),
	LTEXT("\\nu"),
	LTEXT("\\o"),
	LTEXT("\\O"),
	LTEXT("\\oddsidemargin"),
	LTEXT("\\odot"),
	LTEXT("\\oe"),
	LTEXT("\\OE"),
	LTEXT("\\oint"),
	LTEXT("\\Omega"),
	LTEXT("\\omega"),
	LTEXT("\\ominus"),
	LTEXT("\\oplus"),
	LTEXT("\\opening"),
	LTEXT("\\otimes"),
	LTEXT("\\owns"),
	LTEXT("\\overleftarrow"),
	LTEXT("\\overline"),
	LTEXT("\\overrightarrow"),
	LTEXT("\\overvrace"),
	LTEXT("\\oval"),
	LTEXT("\\P"),
	LTEXT("\\pagebreak"),
	LTEXT("\\pagenumbering"),
	LTEXT("\\pageref"),
	LTEXT("\\pagestyle"),
	LTEXT("\\par"),
	LTEXT("\\parallel"),
	LTEXT("\\paragraph"),
	LTEXT("\\parbox"),
	LTEXT("\\parindent"),
	LTEXT("\\parskip"),
	LTEXT("\\partial"),
	LTEXT("\\perp"),
	LTEXT("\\phi"),
	LTEXT("\\Phi"),
	LTEXT("\\pi"),
	LTEXT("\\Pi"),
	LTEXT("\\pm"),
	LTEXT("\\Pr"),
	LTEXT("\\prime"),
	LTEXT("\\printindex"),
	LTEXT("\\prod"),
	LTEXT("\\propto"),
	LTEXT("\\ps"),
	LTEXT("\\psi"),
	LTEXT("\\Psi"),
	LTEXT("\\put"),
	LTEXT("\\qquad"),
	LTEXT("\\quad"),
	LTEXT("\\raisebox"),
	LTEXT("\\rangle"),
	LTEXT("\\rceil"),
	LTEXT("\\Re"),
	LTEXT("\\ref"),
	LTEXT("\\renewcommand"),
	LTEXT("\\renewenvironment"),
	LTEXT("\\rfloor"),
	LTEXT("\\rho"),
	LTEXT("\\right"),
	LTEXT("\\rightarrow"),
	LTEXT("\\Rightarrow"),
	LTEXT("\\rightharpoondown"),
	LTEXT("\\rightharpoonup"),
	LTEXT("\\rightleftharpoonup"),
	LTEXT("\\rightmargin"),
	LTEXT("\\rm"),
	LTEXT("\\rule"),
	LTEXT("\\roman"),
	LTEXT("\\Roman"),
	LTEXT("\\S"),
	LTEXT("\\samepage"),
	LTEXT("\\sb"),
	LTEXT("\\sc"),
	LTEXT("\\scriptsize"),
	LTEXT("\\scriptscriptstyle"),
	LTEXT("\\scriptstyle"),
	LTEXT("\\searrow"),
	LTEXT("\\sec"),
	LTEXT("\\section"),
	LTEXT("\\setcounter"),
	LTEXT("\\setlength"),
	LTEXT("\\settowidth"),
	LTEXT("\\setminus"),
	LTEXT("\\sf"),
	LTEXT("\\sharp"),
	LTEXT("\\sigma"),
	LTEXT("\\Sigma"),
	LTEXT("\\signature"),
	LTEXT("\\sim"),
	LTEXT("\\simeq"),
	LTEXT("\\sin"),
	LTEXT("\\sinh"),
	LTEXT("\\sl"),
	LTEXT("\\sloppy"),
	LTEXT("\\small"),
	LTEXT("\\smash"),
	LTEXT("\\smallskip"),
	LTEXT("\\sp"),
	LTEXT("\\spadesuit"),
	LTEXT("\\special"),
	LTEXT("\\sqrt"),
	LTEXT("\\ss"),
	LTEXT("\\star"),
	LTEXT("\\stackrel"),
	LTEXT("\\strut"),
	LTEXT("\\subparagraph"),
	LTEXT("\\subsection"),
	LTEXT("\\subset"),
	LTEXT("\\subseteq"),
	LTEXT("\\subsubsection"),
	LTEXT("\\sum"),
	LTEXT("\\sup"),
	LTEXT("\\supset"),
	LTEXT("\\supseteq"),
	LTEXT("\\swarrow"),
	LTEXT("\\t"),
	LTEXT("\\tableofcontents"),
	LTEXT("\\tan"),
	LTEXT("\\tanh"),
	LTEXT("\\tau"),
	LTEXT("\\TeX"),
	LTEXT("\\textbf"),
	LTEXT("\\textgreater"),
	LTEXT("\\textgt"),
	LTEXT("\\textheight"),
	LTEXT("\\textit"),
	LTEXT("\\textless"),
	LTEXT("\\textmc"),
	LTEXT("\\textrm"),
	LTEXT("\\textsc"),
	LTEXT("\\textsf"),
	LTEXT("\\textsl"),
	LTEXT("\\textstyle"),
	LTEXT("\\texttt"),
	LTEXT("\\textwidth"),
	LTEXT("\\thanks"),
	LTEXT("\\thebibliography"),
	LTEXT("\\theequation"),
	LTEXT("\\thepage"),
	LTEXT("\\thesection"),
	LTEXT("\\theta"),
	LTEXT("\\Theta"),
	LTEXT("\\thicklines"),
	LTEXT("\\thinlines"),
	LTEXT("\\thinspace"),
	LTEXT("\\thisepage"),
	LTEXT("\\thisepagestyle"),
	LTEXT("\\tie"),
	LTEXT("\\tilde"),
	LTEXT("\\times"),
	LTEXT("\\tiny"),
	LTEXT("\\title"),
	LTEXT("\\titlepage"),
	LTEXT("\\to"),
	LTEXT("\\toaddress"),
	LTEXT("\\topmargin"),
	LTEXT("\\triangle"),
	LTEXT("\\tt"),
	LTEXT("\\twocolumn"),
	LTEXT("\\u"),
	LTEXT("\\underline"),
	LTEXT("\\undervrace"),
	LTEXT("\\unitlength"),
	LTEXT("\\Uparrow"),
	LTEXT("\\uparrow"),
	LTEXT("\\updownarrow"),
	LTEXT("\\Updownarrow"),
	LTEXT("\\uplus"),
	LTEXT("\\upsilon"),
	LTEXT("\\Upsilon"),
	LTEXT("\\usepackage"),
	LTEXT("\\v"),
	LTEXT("\\varepsilon"),
	LTEXT("\\varphi"),
	LTEXT("\\varpi"),
	LTEXT("\\varrho"),
	LTEXT("\\varsigma"),
	LTEXT("\\vartheta"),
	LTEXT("\\vbox"),
	LTEXT("\\vcenter"),
	LTEXT("\\vec"),
	LTEXT("\\vector"),
	LTEXT("\\vee"),
	LTEXT("\\verb"),
	LTEXT("\\verb*"),
	LTEXT("\\verbatim"),
	LTEXT("\\vert"),
	LTEXT("\\Vert"),
	LTEXT("\\vfil"),
	LTEXT("\\vfill"),
	LTEXT("\\vrule"),
	LTEXT("\\vskip"),
	LTEXT("\\vspace"),
	LTEXT("\\vspace*"),
	LTEXT("\\wedge"),
	LTEXT("\\widehat"),
	LTEXT("\\widetilde"),
	LTEXT("\\wp"),
	LTEXT("\\wr"),
	LTEXT("\\wrapfigure"),
	LTEXT("\\xi"),
	LTEXT("\\Xi"),
	LTEXT("\\zeta")//,
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
int g_nKeywordsTEX = int(std::size(g_ppszKeywordsTEX));

//Jan. 19, 2001 JEPRO	TeX のキーワード2として新規追加 & 一部復活 --環境コマンドとオプション名が中心
const WCHAR* g_ppszKeywordsTEX2[] = {
	//	環境コマンド
	//Jan. 19, 2001 JEPRO 本当は{}付きでキーワードにしたかったが単語として認識してくれないので止めた
	LTEXT("abstract"),
	LTEXT("array"),
	LTEXT("center"),
	LTEXT("description"),
	LTEXT("document"),
	LTEXT("displaymath"),
	LTEXT("em"),
	LTEXT("enumerate"),
	LTEXT("eqnarray"),
	LTEXT("eqnarray*"),
	LTEXT("equation"),
	LTEXT("figure"),
	LTEXT("figure*"),
	LTEXT("floatingfigure"),
	LTEXT("flushleft"),
	LTEXT("flushright"),
	LTEXT("itemize"),
	LTEXT("letter"),
	LTEXT("list"),
	LTEXT("math"),
	LTEXT("minipage"),
	LTEXT("multicols"),
	LTEXT("namelist"),
	LTEXT("picture"),
	LTEXT("quotation"),
	LTEXT("quote"),
	LTEXT("sloppypar"),
	LTEXT("subeqnarray"),
	LTEXT("subeqnarray*"),
	LTEXT("subequations"),
	LTEXT("subfigure"),
	LTEXT("tabbing"),
	LTEXT("table"),
	LTEXT("table*"),
	LTEXT("tabular"),
	LTEXT("tabular*"),
	LTEXT("tatepage"),
	LTEXT("thebibliography"),
	LTEXT("theindex"),
	LTEXT("titlepage"),
	LTEXT("trivlist"),
	LTEXT("verbatim"),
	LTEXT("verbatim*"),
	LTEXT("verse"),
	LTEXT("wrapfigure"),
	//	スタイルオプション
	LTEXT("a4"),
	LTEXT("a4j"),
	LTEXT("a5"),
	LTEXT("a5j"),
	LTEXT("Alph"),
	LTEXT("alph"),
	LTEXT("annote"),
	LTEXT("arabic"),
	LTEXT("b4"),
	LTEXT("b4j"),
	LTEXT("b5"),
	LTEXT("b5j"),
	LTEXT("bezier"),
	LTEXT("booktitle"),
	LTEXT("boxedminipage"),
	LTEXT("boxit"),
//		"bp",
//		"cm",
	LTEXT("dbltopnumber"),
//		"dd",
	LTEXT("eclepsf"),
	LTEXT("eepic"),
	LTEXT("enumi"),
	LTEXT("enumii"),
	LTEXT("enumiii"),
	LTEXT("enumiv"),
	LTEXT("epic"),
	LTEXT("epsbox"),
	LTEXT("epsf"),
	LTEXT("fancybox"),
	LTEXT("fancyheadings"),
	LTEXT("fleqn"),
	LTEXT("footnote"),
	LTEXT("howpublished"),
	LTEXT("jabbrv"),
	LTEXT("jalpha"),
//		"article",
	LTEXT("jarticle"),
	LTEXT("jsarticle"),
//		"book",
	LTEXT("jbook"),
	LTEXT("jsbook"),
//		"letter",
	LTEXT("jletter"),
//		"plain",
	LTEXT("jplain"),
//		"report",
	LTEXT("jreport"),
	LTEXT("jtwocolumn"),
	LTEXT("junsrt"),
	LTEXT("leqno"),
	LTEXT("makeidx"),
	LTEXT("markboth"),
	LTEXT("markright"),
//		"mm",
	LTEXT("multicol"),
	LTEXT("myheadings"),
	LTEXT("openbib"),
//		"pc",
//		"pt",
	LTEXT("secnumdepth"),
//		"sp",
	LTEXT("titlepage"),
	LTEXT("tjarticle"),
	LTEXT("topnumber"),
	LTEXT("totalnumber"),
	LTEXT("twocolumn"),
	LTEXT("twoside"),
	LTEXT("yomi")//,
//		"zh",
//		"zw"
};
int g_nKeywordsTEX2 = int(std::size(g_ppszKeywordsTEX2));
