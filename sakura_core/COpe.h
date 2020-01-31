/*!	@file
	@brief 編集操作要素

	@author Norio Nakatani
	@date 1998/06/09 新規作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#pragma once

#include <vector>

#include "mem/CNativeW.h"

//! アンドゥバッファ用 操作コード
enum EOpeCode
{
    OPE_UNKNOWN   = 0, //!< 不明(未使用)
    OPE_INSERT    = 1, //!< 挿入
    OPE_DELETE    = 2, //!< 削除
    OPE_REPLACE   = 3, //!< 置換
    OPE_MOVECARET = 4, //!< キャレット移動
};

class CLineData final
{
  public:
    CNativeW cmemLine;
    int nSeq;
    CLineData()                        = default;
    CLineData(const CLineData &source) = default;
    CLineData(CLineData &&other) noexcept
    {
        swap(other);
    }

  private:
    void swap(CLineData &o) noexcept
    {
        std::swap(cmemLine, o.cmemLine);
        std::swap(nSeq, o.nSeq);
    }

  public:
    CLineData &operator=(const CLineData &rhs) = default;
    CLineData &operator                        =(CLineData &&rhs) noexcept
    {
        swap(rhs);
        return *this;
    }
};

typedef std::vector<CLineData> COpeLineData;

/*!
	編集操作要素
	
	Undoのためにに操作手順を記録するために用いる。
	1オブジェクトが１つの操作を表す。
*/
//2007.10.17 kobake 解放漏れを防ぐため、データをポインタではなくインスタンス実体で持つように変更
class COpe
{
  public:
    COpe(EOpeCode eCode); /* COpeクラス構築 */
    virtual ~COpe(); /* COpeクラス消滅 */

    virtual void DUMP(void); /* 編集操作要素のダンプ */

    EOpeCode GetCode() const
    {
        return m_nOpe;
    }

  private:
    EOpeCode m_nOpe; //!< 操作種別

  public:
    CLogicPoint m_ptCaretPos_PHY_Before; //!< キャレット位置。文字単位。			[共通]
    CLogicPoint m_ptCaretPos_PHY_After; //!< キャレット位置。文字単位。			[共通]
};

//!削除
class CDeleteOpe final : public COpe
{
  public:
    CDeleteOpe()
        : COpe(OPE_DELETE)
    {
        m_ptCaretPos_PHY_To.Set(CLogicInt(0), CLogicInt(0));
    }
    void DUMP(void) override; /* 編集操作要素のダンプ */
  public:
    CLogicPoint m_ptCaretPos_PHY_To; //!< 操作前のキャレット位置。文字単位。	[DELETE]
    COpeLineData m_cOpeLineData; //!< 操作に関連するデータ				[DELETE/INSERT]
    int m_nOrgSeq;
};

//!挿入
class CInsertOpe final : public COpe
{
  public:
    CInsertOpe()
        : COpe(OPE_INSERT)
    {
    }
    void DUMP(void) override; /* 編集操作要素のダンプ */
  public:
    COpeLineData m_cOpeLineData; //!< 操作に関連するデータ				[DELETE/INSERT]
    int m_nOrgSeq;
};

//!置換
class CReplaceOpe final : public COpe
{
  public:
    CReplaceOpe()
        : COpe(OPE_REPLACE)
    {
        m_ptCaretPos_PHY_To.Set(CLogicInt(0), CLogicInt(0));
    }

  public:
    CLogicPoint m_ptCaretPos_PHY_To; //!< 操作前のキャレット位置。文字単位。	[DELETE]
    COpeLineData m_pcmemDataIns; //!< 操作に関連するデータ				[INSERT]
    COpeLineData m_pcmemDataDel; //!< 操作に関連するデータ				[DELETE]
    int m_nOrgInsSeq;
    int m_nOrgDelSeq;
};

//!キャレット移動
class CMoveCaretOpe final : public COpe
{
  public:
    CMoveCaretOpe()
        : COpe(OPE_MOVECARET)
    {
    }
    CMoveCaretOpe(const CLogicPoint &ptBefore, const CLogicPoint &ptAfter)
        : COpe(OPE_MOVECARET)
    {
        m_ptCaretPos_PHY_Before = ptBefore;
        m_ptCaretPos_PHY_After  = ptAfter;
    }
    CMoveCaretOpe(const CLogicPoint &ptCaretPos)
        : COpe(OPE_MOVECARET)
    {
        m_ptCaretPos_PHY_Before = ptCaretPos;
        m_ptCaretPos_PHY_After  = ptCaretPos;
    }
};
