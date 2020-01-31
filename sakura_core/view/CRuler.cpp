/*! @file */
#include "StdAfx.h"
#include "CRuler.h"
#include "CTextArea.h"
#include "view/CEditView.h"
#include "doc/CEditDoc.h"
#include "types/CTypeSupport.h"

CRuler::CRuler(const CEditView *pEditView, const CEditDoc *pEditDoc)
    : m_pEditView(pEditView)
    , m_pEditDoc(pEditDoc)
    , m_hFont(NULL)
    , m_nRulerHeight(0)
{
    m_nOldRulerDrawX = 0; // 前回描画したルーラーのキャレット位置 2002.02.25 Add By KK
    m_nOldRulerWidth = 0; // 前回描画したルーラーのキャレット幅   2002.02.25 Add By KK
}

CRuler::~CRuler()
{
    if (m_hFont)
    {
        ::DeleteObject(m_hFont);
    }
}

//2007.08.26 kobake UNICODE用にX位置を変更
void CRuler::_DrawRulerCaret(CGraphics &gr, int nCaretDrawPosX, int nCaretWidth)
{
    //描画領域 -> hRgn
    RECT rc;
    rc.left   = nCaretDrawPosX + 1; // 2012.07.27 Moca 1px右に修正
    rc.right  = rc.left + m_pEditView->GetTextMetrics().GetHankakuDx() - 1;
    rc.top    = 0;
    rc.bottom = m_pEditView->GetTextArea().GetAreaTop() - m_pEditView->GetTextArea().GetTopYohaku() - 1;
    HRGN hRgn = ::CreateRectRgnIndirect(&rc);

    //ブラシ作成 -> hBrush
    HBRUSH hBrush;
    if (0 == nCaretWidth)
    {
        hBrush = ::CreateSolidBrush(RGB(128, 128, 128));
    }
    else
    {
        hBrush = ::CreateSolidBrush(RGB(0, 0, 0));
    }

    //領域を描画 (色を反転させる)
    int nROP_Old     = ::SetROP2(gr, R2_NOTXORPEN);
    HBRUSH hBrushOld = (HBRUSH)::SelectObject(gr, hBrush);
    ::SelectObject(gr, hBrush);
    ::PaintRgn(gr, hRgn);
    ::SelectObject(gr, hBrushOld);
    ::SetROP2(gr, nROP_Old);

    //描画オブジェクト破棄
    ::DeleteObject(hRgn);
    ::DeleteObject(hBrush);
}

/*! 
	ルーラーのキャレットを再描画	2002.02.25 Add By KK
	@param hdc [in] デバイスコンテキスト
	DispRulerの内容を元に作成
*/
void CRuler::DrawRulerCaret(CGraphics &gr)
{
    if (m_pEditView->GetTextArea().GetViewLeftCol() <= m_pEditView->GetCaret().GetCaretLayoutPos().GetX() && m_pEditView->GetTextArea().GetRightCol() + 2 >= m_pEditView->GetCaret().GetCaretLayoutPos().GetX())
    {
        if (m_pEditView->GetRuler().m_nOldRulerDrawX == m_pEditView->GetCaret().CalcCaretDrawPos(m_pEditView->GetCaret().GetCaretLayoutPos()).x && m_pEditView->GetCaret().GetCaretSize().cx == m_pEditView->GetRuler().m_nOldRulerWidth)
        {
            //前描画した位置画同じ かつ ルーラーのキャレット幅が同じ
            return;
        }

        //元位置をクリア m_nOldRulerWidth
        this->_DrawRulerCaret(gr, m_nOldRulerDrawX, m_nOldRulerWidth);

        //新しい位置で描画   2007.08.26 kobake UNICODE用にX位置を変更
        this->_DrawRulerCaret(
            gr,
            m_pEditView->GetCaret().CalcCaretDrawPos(m_pEditView->GetCaret().GetCaretLayoutPos()).x,
            m_pEditView->GetCaret().GetCaretSize().cx);
    }
}

//! ルーラーの背景のみ描画 2007.08.29 kobake 追加
void CRuler::DrawRulerBg(CGraphics &gr)
{
    //必要なインターフェース
    CommonSetting *pCommon = &GetDllShareData().m_Common;

    //サポート
    CTypeSupport cRulerType(m_pEditView, COLORIDX_RULER);

    // フォント設定 (ルーラー上の数字用)
    if (m_hFont && m_nRulerHeight != pCommon->m_sWindow.m_nRulerHeight)
    {
        ::DeleteObject(m_hFont);
        m_hFont = NULL;
    }
    if (m_hFont == NULL)
    {
        LOGFONT lf          = {0};
        lf.lfHeight         = 1 - pCommon->m_sWindow.m_nRulerHeight; //	2002/05/13 ai
        lf.lfWidth          = 0;
        lf.lfEscapement     = 0;
        lf.lfOrientation    = 0;
        lf.lfWeight         = 400;
        lf.lfItalic         = 0;
        lf.lfUnderline      = 0;
        lf.lfStrikeOut      = 0;
        lf.lfCharSet        = 0;
        lf.lfOutPrecision   = 3;
        lf.lfClipPrecision  = 2;
        lf.lfQuality        = 1;
        lf.lfPitchAndFamily = 34;
        wcscpy_s(lf.lfFaceName, L"Arial");
        m_hFont        = ::CreateFontIndirect(&lf);
        m_nRulerHeight = pCommon->m_sWindow.m_nRulerHeight;
    }
    assert(m_hFont != NULL);
    HFONT hFontOld = (HFONT)::SelectObject(gr, m_hFont);
    ::SetBkMode(gr, TRANSPARENT);

    //背景塗りつぶし
    RECT rc;
    rc.left   = 0;
    rc.top    = 0;
    rc.right  = m_pEditView->GetTextArea().GetAreaRight();
    rc.bottom = m_pEditView->GetTextArea().GetAreaTop() - m_pEditView->GetTextArea().GetTopYohaku();
    cRulerType.FillBack(gr, rc);

    //ルーラー色設定
    gr.PushPen(cRulerType.GetTextColor(), 0);
    gr.PushTextForeColor(cRulerType.GetTextColor());

    //描画開始位置
    int nX = m_pEditView->GetTextArea().GetAreaLeft();
    int nY = m_pEditView->GetTextArea().GetRulerHeight() - 2;

    //	Aug. 14, 2005 genta 折り返し幅をLayoutMgrから取得するように
    //	2005.11.10 Moca 1dot足りない
    CLayoutXInt nMaxLineColum = m_pEditDoc->m_cLayoutMgr.GetMaxLineLayout();
    CKetaXInt nMaxLineKetas   = m_pEditDoc->m_cLayoutMgr.GetMaxLineKetas();
    int nToX                  = m_pEditView->GetTextArea().GetAreaLeft() + m_pEditView->GetTextMetrics().GetCharPxWidth(nMaxLineColum - m_pEditView->GetTextArea().GetViewLeftCol()) + 1;
    if (nToX > m_pEditView->GetTextArea().GetAreaRight())
    {
        nToX = m_pEditView->GetTextArea().GetAreaRight();
    }

    //目盛を描画
    const int oneColumn = (Int)m_pEditView->GetTextMetrics().GetLayoutXDefault();
    CLayoutXInt i       = m_pEditView->GetTextArea().GetViewLeftCol();
    CKetaXInt keta      = CKetaXInt(((Int)i) / oneColumn);
    const int dx        = m_pEditView->GetTextMetrics().GetHankakuDx(); // PPでもDx
    // 先頭がかけている場合は次の桁に進む
    const int pxOffset = (Int)i % oneColumn;
    if (pxOffset)
    {
        nX += oneColumn - pxOffset;
        i += CLayoutXInt(oneColumn - pxOffset); // CLayoutXInt == pixel
        ++keta;
    }

    // 目盛り線を1本ずつ描画するのではなく後述する PolyPolyline でまとめて描画を行う
    const int nWidth          = (Int)(m_pEditView->GetTextArea().GetRightCol() - i);
    const size_t nLinesToDraw = 1 + std::min<int>((nWidth + 1 + 1 + oneColumn - 1) / oneColumn, nMaxLineKetas - keta + 1);
    auto &apt                 = m_apt;
    auto &asz                 = m_asz;
    apt.resize(nLinesToDraw * 2);
    asz.resize(nLinesToDraw, 2);
    // 下線 (ルーラーと本文の境界)
    apt[0]     = POINT{m_pEditView->GetTextArea().GetAreaLeft(), nY + 1};
    apt[1]     = POINT{nToX, nY + 1};
    size_t idx = 1;
    while (i <= m_pEditView->GetTextArea().GetRightCol() + 1 && keta <= nMaxLineKetas)
    {
        apt[idx * 2 + 0] = POINT{nX, nY};
        //ルーラー終端の区切り(大)
        if (keta == nMaxLineKetas)
        {
            apt[idx * 2 + 1] = POINT{nX, 0};
        }
        //10目盛おきの区切り(大)と数字
        else if (0 == keta % 10)
        {
            wchar_t szColumn[32];
            apt[idx * 2 + 1] = POINT{nX, 0};
            _itow(((Int)keta) / 10, szColumn, 10);
            ::TextOut(gr, nX + 2 + 0, -1 + 0, szColumn, wcslen(szColumn));
        }
        //5目盛おきの区切り(中)
        else if (0 == keta % 5)
        {
            apt[idx * 2 + 1] = POINT{nX, nY - 6};
        }
        //毎目盛の区切り(小)
        else
        {
            apt[idx * 2 + 1] = POINT{nX, nY - 3};
        }
        ++idx;
        assert(idx <= nLinesToDraw);

        nX += dx;
        i += oneColumn;
        keta++;
    }
    ::PolyPolyline(gr, &apt[0], &asz[0], (DWORD)nLinesToDraw);

    //色戻す
    gr.PopTextForeColor();
    gr.PopPen();

    //フォント戻す
    ::SelectObject(gr, hFontOld);
}

/*! ルーラー描画

	@date 2005.08.14 genta 折り返し幅をLayoutMgrから取得するように
*/
void CRuler::DispRuler(HDC hdc)
{
    //サポート
    CTypeSupport cRulerType(m_pEditView, COLORIDX_RULER);

    if (!m_pEditView->GetDrawSwitch())
    {
        return;
    }
    if (!cRulerType.IsDisp() || m_pEditView->m_bMiniMap)
    {
        return;
    }

    // 描画対象
    CGraphics gr(hdc);

    // 2002.02.25 Add By KK ルーラー全体を描き直す必要がない場合は、ルーラ上のキャレットのみ描きなおす
    if (!m_bRedrawRuler)
    {
        DrawRulerCaret(gr);
    }
    else
    {
        // 背景描画
        DrawRulerBg(gr);

        // キャレット描画
        if (m_pEditView->GetTextArea().GetViewLeftCol() <= m_pEditView->GetCaret().GetCaretLayoutPos().GetX() && m_pEditView->GetTextArea().GetRightCol() + 2 >= m_pEditView->GetCaret().GetCaretLayoutPos().GetX())
        {
            _DrawRulerCaret(gr, m_pEditView->GetCaret().CalcCaretDrawPos(m_pEditView->GetCaret().GetCaretLayoutPos()).x, m_pEditView->GetCaret().GetCaretSize().cx);
        }

        m_bRedrawRuler = false; //m_bRedrawRuler = true で指定されるまで、ルーラのキャレットのみを再描画 2002.02.25 Add By KK
    }

    //描画したルーラーのキャレット位置・幅を保存 2002.02.25 Add By KK
    m_nOldRulerDrawX = m_pEditView->GetCaret().CalcCaretDrawPos(m_pEditView->GetCaret().GetCaretLayoutPos()).x;
    m_nOldRulerWidth = m_pEditView->GetCaret().GetCaretSize().cx;
}
