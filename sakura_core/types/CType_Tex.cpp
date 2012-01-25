#include "StdAfx.h"
#include "types/CType.h"
#include "doc/CEditDoc.h"
#include "doc/CDocLineMgr.h"
#include "doc/CDocOutline.h"
#include "outline/CFuncInfoArr.h"
#include "view/CEditView.h" // SColorStrategyInfo
#include "view/colors/CColorStrategy.h"

/* TeX */
//Oct. 31, 2000 JEPRO TeX  ���[�U�ɑ���
//Oct. 31, 2000 JEPRO TeX ���[�U�ɑ���	//Mar. 10, 2001 JEPRO �ǉ�
void CType_Tex::InitTypeConfigImp(STypeConfig* pType)
{
	//���O�Ɗg���q
	_tcscpy( pType->m_szTypeName, _T("TeX") );
	_tcscpy( pType->m_szTypeExts, _T("tex,ltx,sty,bib,log,blg,aux,bbl,toc,lof,lot,idx,ind,glo") );

	//�ݒ�
	pType->m_cLineComment.CopyTo( 0, L"%", -1 );				/* �s�R�����g�f���~�^ */
	pType->m_eDefaultOutline = OUTLINE_TEX;						/* �A�E�g���C����͕��@ */
	pType->m_nKeyWordSetIdx[0] = 9;								/* �L�[���[�h�Z�b�g */
	pType->m_nKeyWordSetIdx[1] = 10;							/* �L�[���[�h�Z�b�g2 */	//Jan. 19, 2001 JEPRO
	pType->m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp = false;	//�V���O���N�H�[�e�[�V�����������F�����\�����Ȃ�
	pType->m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp = false;	//�_�u���N�H�[�e�[�V�����������F�����\�����Ȃ�
}




/*! TeX �A�E�g���C�����

	@author naoh
	@date 2003.07.21 naoh �V�K�쐬
	@date 2005.01.03 naoh �u�}�v�Ȃǂ�"}"���܂ޕ����ɑ΂���C���Aprosper��slide�ɑΉ�
*/
void CDocOutline::MakeTopicList_tex(CFuncInfoArr* pcFuncInfoArr)
{
	const wchar_t*	pLine;
	CLogicInt		nLineLen;
	int				i;
	int				j;
	int				k;

	const int nMaxStack = 8;	//	�l�X�g�̍Ő[
	int nDepth = 0;				//	���܂̃A�C�e���̐[����\�����l�B
	wchar_t szTag[32], szTitle[256];			//	�ꎞ�̈�
	int thisSection=0, lastSection = 0;	// ���݂̃Z�N�V������ނƈ�O�̃Z�N�V�������
	int stackSection[nMaxStack];		// �e�[���ł̃Z�N�V�����̔ԍ�
	int nStartTitlePos;					// \section{dddd} �� dddd �̕����̎n�܂�ԍ�
	int bNoNumber;						// * �t�̏ꍇ�̓Z�N�V�����ԍ���t���Ȃ�

	// ��s����
	CLogicInt	nLineCount;
	for(nLineCount=CLogicInt(0);nLineCount<m_pcDocRef->m_cDocLineMgr.GetLineCount();nLineCount++)
	{
		pLine	=	m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		if(!pLine) break;
		// �ꕶ������
		for(i=0;i<nLineLen-1;i++)
		{
			if(pLine[i] == L'%') break;	// �R�����g�Ȃ�ȍ~�͂���Ȃ�
			if(nDepth>=nMaxStack)continue;
			if(pLine[i] != L'\\')continue;	// �u\�v���Ȃ��Ȃ玟�̕�����
			++i;
			// ���������u\�v�ȍ~�̕�����`�F�b�N
			for(j=0;i+j<nLineLen && j<_countof(szTag)-1;j++)
			{
				if(pLine[i+j] == L'{' && !(i+j>0 && _IS_SJIS_1((unsigned char)pLine[i+j-1])) ) {	// SJIS1�`�F�b�N
					bNoNumber = (pLine[i+j-1] == '*');
					nStartTitlePos = j+i+1;
					break;
				}
				szTag[j] = pLine[i+j];
			}
			if(j==0) continue;
			if(bNoNumber){
				szTag[j-1] = L'\0';
			}else{
				szTag[j]   = L'\0';
			}

			thisSection = 0;
			if(!wcscmp(szTag,L"subsubsection")) thisSection = 4;
			else if(!wcscmp(szTag,L"subsection")) thisSection = 3;
			else if(!wcscmp(szTag,L"section")) thisSection = 2;
			else if(!wcscmp(szTag,L"chapter")) thisSection = 1;
			else if(!wcscmp(szTag,L"begin")) {		// begin�Ȃ� prosper��slide�̉\�����l��
				// �����{slide}{}�܂œǂ݂Ƃ��Ă���
				if(wcsstr(pLine, L"{slide}")){
					k=0;
					for(j=nStartTitlePos+1;i+j<nLineLen && j<_countof(szTag)-1;j++)
					{
						if(pLine[i+j] == '{' && !(i+j>0 && _IS_SJIS_1((unsigned char)pLine[i+j-1])) ) {	// SJIS1�`�F�b�N
							nStartTitlePos = j+i+1;
							break;
						}
						szTag[k++]	=	pLine[i+j];
					}
					szTag[k] = '\0';
					thisSection = 1;
				}
			}

			if( thisSection > 0)
			{
				// section�̒��g�擾
				for(k=0;nStartTitlePos+k<nLineLen && k<_countof(szTitle)-1;k++)
				{
					if(_IS_SJIS_1((unsigned char)pLine[k+nStartTitlePos])) {
						szTitle[k] = pLine[k+nStartTitlePos];
						k++;	// ���̓`�F�b�N�s�v
					} else if(pLine[k+nStartTitlePos] == '}') {
						break;
					}
					szTitle[k] = pLine[k+nStartTitlePos];
				}
				szTitle[k] = '\0';

				CLayoutPoint ptPos;

				WCHAR tmpstr[256];
				WCHAR secstr[4];

				m_pcDocRef->m_cLayoutMgr.LogicToLayout(
					CLogicPoint(i, nLineCount),
					&ptPos
				);

				int sabunSection = thisSection - lastSection;
				if(lastSection == 0){
					nDepth = 0;
					stackSection[0] = 1;
				}else{
					nDepth += sabunSection;
					if(sabunSection > 0){
						if(nDepth >= nMaxStack) nDepth=nMaxStack-1;
						stackSection[nDepth] = 1;
					}else{
						if(nDepth < 0) nDepth=0;
						++stackSection[nDepth];
					}
				}
				tmpstr[0] = L'\0';
				if(!bNoNumber){
					for(k=0; k<=nDepth; k++){
						auto_sprintf(secstr, L"%d.", stackSection[k]);
						wcscat(tmpstr, secstr);
					}
					wcscat(tmpstr, L" ");
				}
				wcscat(tmpstr, szTitle);
				pcFuncInfoArr->AppendData(nLineCount+CLogicInt(1),ptPos.GetY2()+CLayoutInt(1), tmpstr, 0, nDepth);
				if(!bNoNumber) lastSection = thisSection;
			}
			i	+=	j;
		}
	}
}



const wchar_t* g_ppszKeywordsTEX[] = {
//Nov. 20, 2000 JEPRO	�啝�ǉ� & �኱�C���E�폜 --�قƂ�ǃR�}���h�̂�
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

//Jan. 19, 2001 JEPRO	TeX �̃L�[���[�h2�Ƃ��ĐV�K�ǉ� & �ꕔ���� --���R�}���h�ƃI�v�V�����������S
const wchar_t* g_ppszKeywordsTEX2[] = {
	//	���R�}���h
	//Jan. 19, 2001 JEPRO �{����{}�t���ŃL�[���[�h�ɂ������������P��Ƃ��ĔF�����Ă���Ȃ��̂Ŏ~�߂�
	L"abstract",
	L"array"
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
	//	�X�^�C���I�v�V����
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

