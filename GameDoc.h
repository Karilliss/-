#pragma once
#include "GameData.h"

class CGameDoc : public CDocument
{
protected:
    CGameDoc() noexcept;
    DECLARE_DYNCREATE(CGameDoc)

public:
    GameState m_gameState;

public:
    virtual BOOL OnNewDocument();
    virtual void Serialize(CArchive& ar);

#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

protected:
    virtual ~CGameDoc();
    DECLARE_MESSAGE_MAP()
};

