#include "pch.h"
#include "framework.h"
#include "DominoPuzzleApp.h"
#include "GameUI.h"
#include "GameDoc.h"
#include "GameLogic.h"
#include "GameState.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(GameUI, CView)

BEGIN_MESSAGE_MAP(GameUI, CView)
    ON_WM_LBUTTONDOWN()
    ON_WM_RBUTTONDOWN()
    ON_WM_KEYDOWN()
    ON_WM_TIMER()
    ON_COMMAND(ID_GAME_NEW_EASY, &GameUI::OnGameNewEasy)
    ON_COMMAND(ID_GAME_NEW_MEDIUM, &GameUI::OnGameNewMedium)
    ON_COMMAND(ID_GAME_NEW_HARD, &GameUI::OnGameNewHard)
    ON_COMMAND(ID_GAME_RESET, &GameUI::OnGameReset)
    ON_COMMAND(ID_GAME_HINT, &GameUI::OnGameHint)
    ON_COMMAND(ID_GAME_AUTOSOLVE, &GameUI::OnGameAutoSolve)
    ON_COMMAND(ID_ORIENTATION_HORIZONTAL, &GameUI::OnGameOrientationHorizontal)
    ON_COMMAND(ID_ORIENTATION_VERTICAL, &GameUI::OnGameOrientationVertical)
    ON_UPDATE_COMMAND_UI(ID_ORIENTATION_HORIZONTAL, &GameUI::OnUpdateOrientationHorizontal)
    ON_UPDATE_COMMAND_UI(ID_ORIENTATION_VERTICAL, &GameUI::OnUpdateOrientationVertical)
END_MESSAGE_MAP()

GameUI::GameUI()
{
    SetTimer(1, 1000, nullptr); // Timer for game updates
}

GameUI::~GameUI()
{
    KillTimer(1);
}

CGameDoc* GameUI::GetDocument() const
{
    ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CGameDoc)));
    return (CGameDoc*)m_pDocument;
}

BOOL GameUI::PreCreateWindow(CREATESTRUCT& cs)
{
    return CView::PreCreateWindow(cs);
}

void GameUI::OnDraw(CDC* pDC)
{
    CGameDoc* pDoc = GetDocument();
    ASSERT_VALID(pDoc);
    if (!pDoc || !pDC) return;

    CRect clientRect;
    GetClientRect(&clientRect);

    // Draw background
    pDC->FillSolidRect(clientRect, RGB(30, 60, 114));

    // Draw game elements
    DrawGrid(pDC);
    DrawDominoPanel(pDC);
    DrawStats(pDC);
}

void GameUI::DrawGrid(CDC* pDC)
{
    CGameDoc* pDoc = GetDocument();
    GameState& state = pDoc->m_gameState;

    CPen gridPen(PS_SOLID, 1, RGB(100, 120, 150));
    CPen* pOldPen = pDC->SelectObject(&gridPen);

    for (int row = 0; row < GameState::GRID_SIZE; row++) {
        for (int col = 0; col < GameState::GRID_SIZE; col++) {
            CRect cellRect = GetCellRect(row, col);
            DrawCell(pDC, row, col, cellRect);
        }
    }

    pDC->SelectObject(pOldPen);
}

void GameUI::DrawCell(CDC* pDC, int row, int col, const CRect& rect)
{
    CGameDoc* pDoc = GetDocument();
    GameState& state = pDoc->m_gameState;

    // Determine cell color
    COLORREF cellColor;
    if (state.gameGrid[row][col] > 0) {
        cellColor = RGB(255, 255, 200); // Number cell
    }
    else if (state.dominoGrid[row][col] != -1) {
        cellColor = RGB(200, 255, 200); // Domino cell
    }
    else {
        cellColor = RGB(240, 240, 240); // Empty cell
    }

    // Draw cell background
    CBrush cellBrush(cellColor);
    pDC->FillRect(rect, &cellBrush);

    // Draw cell border
    pDC->Rectangle(rect);

    // Draw cell content
    if (state.gameGrid[row][col] > 0) {
        CString value;
        value.Format(_T("%d"), state.gameGrid[row][col]);
        pDC->SetTextColor(RGB(0, 0, 0));
        pDC->SetBkMode(TRANSPARENT);
        pDC->DrawText(value, rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
}

void GameUI::DrawDomino(CDC* pDC, const Domino& domino, const CRect& rect, bool selected)
{
    COLORREF dominoColor = selected ? RGB(150, 255, 150) : RGB(200, 255, 200);
    CBrush dominoBrush(dominoColor);
    pDC->FillRect(rect, &dominoBrush);

    // Draw domino border
    CPen borderPen(PS_SOLID, 2, RGB(0, 0, 0));
    CPen* pOldPen = pDC->SelectObject(&borderPen);
    pDC->Rectangle(rect);
    pDC->SelectObject(pOldPen);

    // Draw domino values
    CString value1, value2;
    value1.Format(_T("%d"), domino.value1);
    value2.Format(_T("%d"), domino.value2);

    pDC->SetTextColor(RGB(0, 0, 0));
    pDC->SetBkMode(TRANSPARENT);

    if (domino.orientation == Orientation::HORIZONTAL) {
        CRect leftHalf(rect);
        leftHalf.right = rect.left + rect.Width() / 2;
        pDC->DrawText(value1, leftHalf, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        CRect rightHalf(rect);
        rightHalf.left = rect.left + rect.Width() / 2;
        pDC->DrawText(value2, rightHalf, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
    else {
        CRect topHalf(rect);
        topHalf.bottom = rect.top + rect.Height() / 2;
        pDC->DrawText(value1, topHalf, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        CRect bottomHalf(rect);
        bottomHalf.top = rect.top + rect.Height() / 2;
        pDC->DrawText(value2, bottomHalf, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
}

void GameUI::DrawDominoPanel(CDC* pDC)
{
    CRect clientRect;
    GetClientRect(&clientRect);
    CRect panelRect(clientRect.right - DOMINO_PANEL_WIDTH, 0, clientRect.right, clientRect.bottom);

    // Draw panel background
    pDC->FillSolidRect(panelRect, RGB(50, 80, 120));

    // Draw available dominoes
    CGameDoc* pDoc = GetDocument();
    for (size_t i = 0; i < pDoc->m_gameState.availableDominoes.size(); i++) {
        CRect dominoRect = GetDominoRect((int)i);
        bool selected = (i == pDoc->m_gameState.selectedDomino);
        DrawDomino(pDC, pDoc->m_gameState.availableDominoes[i], dominoRect, selected);
    }
}

void GameUI::DrawStats(CDC* pDC)
{
    CGameDoc* pDoc = GetDocument();
    GameState& state = pDoc->m_gameState;

    CRect clientRect;
    GetClientRect(&clientRect);
    CRect statsRect(10, 10, clientRect.right - DOMINO_PANEL_WIDTH - 10, 30);

    DWORD elapsed = (GetTickCount() - state.gameStartTime) / 1000;
    CString statsText;
    statsText.Format(_T("Time: %s | Dominoes: %d/%d | Hints: %d"),
        FormatTime(elapsed),
        (int)state.placedDominoes.size(),
        GameState::TOTAL_DOMINOES,
        state.hintsUsed);

    pDC->SetTextColor(RGB(255, 255, 255));
    pDC->SetBkMode(TRANSPARENT);
    pDC->DrawText(statsText, statsRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
}

CRect GameUI::GetCellRect(int row, int col)
{
    return CRect(
        GRID_MARGIN + col * CELL_SIZE,
        GRID_MARGIN + row * CELL_SIZE,
        GRID_MARGIN + (col + 1) * CELL_SIZE,
        GRID_MARGIN + (row + 1) * CELL_SIZE
    );
}

CRect GameUI::GetDominoRect(int index)
{
    CRect clientRect;
    GetClientRect(&clientRect);
    int x = clientRect.right - DOMINO_PANEL_WIDTH + 10;
    int y = 20 + index * (DOMINO_SIZE + 10);
    return CRect(x, y, x + DOMINO_SIZE, y + DOMINO_SIZE);
}

void GameUI::OnLButtonDown(UINT nFlags, CPoint point)
{
    CGameDoc* pDoc = GetDocument();
    GameState& state = pDoc->m_gameState;

    // Check if clicked on domino panel
    int dominoIndex = GetDominoFromPoint(point);
    if (dominoIndex != -1) {
        state.selectedDomino = dominoIndex;
        Invalidate();
        return;
    }

    // Place domino on grid
    if (state.selectedDomino != -1) {
        int row, col;
        GetCellFromPoint(point, row, col);

        // Set game state reference for game logic
        m_gameLogic.SetGameState(&state);

        if (m_gameLogic.PlaceDomino(row, col)) {
            if (m_gameLogic.CheckWin()) {
                ShowMessage(_T("Congratulations! You solved the puzzle!"));
            }
            Invalidate();
        }
    }
}

void GameUI::OnRButtonDown(UINT nFlags, CPoint point)
{
    CGameDoc* pDoc = GetDocument();
    int row, col;
    GetCellFromPoint(point, row, col);

    if (pDoc->m_gameState.dominoGrid[row][col] != -1) {
        // Set game state reference for game logic
        m_gameLogic.SetGameState(&pDoc->m_gameState);
        m_gameLogic.RemoveDomino(row, col);
        Invalidate();
    }
}

void GameUI::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    CGameDoc* pDoc = GetDocument();
    GameState& state = pDoc->m_gameState;

    if (nChar == VK_SPACE && state.selectedDomino != -1) {
        state.currentOrientation = (state.currentOrientation == Orientation::HORIZONTAL)
            ? Orientation::VERTICAL
            : Orientation::HORIZONTAL;
        Invalidate();
    }
    CView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void GameUI::OnTimer(UINT_PTR nIDEvent)
{
    if (nIDEvent == 1) {
        Invalidate(); // Refresh display every second
    }
    CView::OnTimer(nIDEvent);
}

// Game control methods
void GameUI::NewGame(Difficulty difficulty)
{
    CGameDoc* pDoc = GetDocument();
    m_gameLogic.SetGameState(&pDoc->m_gameState);

    if (m_gameLogic.GeneratePuzzle(difficulty)) {
        Invalidate();
    }
    else {
        ShowMessage(_T("Failed to generate puzzle. Try again."));
    }
}

void GameUI::ResetGame()
{
    CGameDoc* pDoc = GetDocument();
    m_gameLogic.SetGameState(&pDoc->m_gameState);
    m_gameLogic.ResetGame();
    Invalidate();
}

void GameUI::GetHint()
{
    CGameDoc* pDoc = GetDocument();
    m_gameLogic.SetGameState(&pDoc->m_gameState);
    m_gameLogic.GetHint();
    Invalidate();
}

void GameUI::AutoSolve()
{
    CGameDoc* pDoc = GetDocument();
    m_gameLogic.SetGameState(&pDoc->m_gameState);
    m_gameLogic.AutoSolve();
    Invalidate();
}

// Command handlers
void GameUI::OnGameNewEasy() { NewGame(Difficulty::EASY); }
void GameUI::OnGameNewMedium() { NewGame(Difficulty::MEDIUM); }
void GameUI::OnGameNewHard() { NewGame(Difficulty::HARD); }
void GameUI::OnGameReset() { ResetGame(); }
void GameUI::OnGameHint() { GetHint(); }
void GameUI::OnGameAutoSolve() { AutoSolve(); }

void GameUI::OnGameOrientationHorizontal()
{
    CGameDoc* pDoc = GetDocument();
    pDoc->m_gameState.currentOrientation = Orientation::HORIZONTAL;
    Invalidate();
}

void GameUI::OnGameOrientationVertical()
{
    CGameDoc* pDoc = GetDocument();
    pDoc->m_gameState.currentOrientation = Orientation::VERTICAL;
    Invalidate();
}

void GameUI::OnUpdateOrientationHorizontal(CCmdUI* pCmdUI)
{
    CGameDoc* pDoc = GetDocument();
    pCmdUI->SetCheck(pDoc->m_gameState.currentOrientation == Orientation::HORIZONTAL);
}

void GameUI::OnUpdateOrientationVertical(CCmdUI* pCmdUI)
{
    CGameDoc* pDoc = GetDocument();
    pCmdUI->SetCheck(pDoc->m_gameState.currentOrientation == Orientation::VERTICAL);
}

// Utility methods
void GameUI::GetCellFromPoint(CPoint point, int& row, int& col)
{
    row = (point.y - GRID_MARGIN) / CELL_SIZE;
    col = (point.x - GRID_MARGIN) / CELL_SIZE;

    // Clamp values to grid bounds
    row = max(0, min(row, GameState::GRID_SIZE - 1));
    col = max(0, min(col, GameState::GRID_SIZE - 1));
}

int GameUI::GetDominoFromPoint(CPoint point)
{
    CRect clientRect;
    GetClientRect(&clientRect);

    // Check if point is in domino panel
    if (point.x < clientRect.right - DOMINO_PANEL_WIDTH)
        return -1;

    // Check each domino position
    CGameDoc* pDoc = GetDocument();
    for (size_t i = 0; i < pDoc->m_gameState.availableDominoes.size(); i++) {
        CRect rect = GetDominoRect((int)i);
        if (rect.PtInRect(point)) {
            return (int)i;
        }
    }
    return -1;
}

CString GameUI::FormatTime(DWORD elapsed)
{
    DWORD minutes = elapsed / 60;
    DWORD seconds = elapsed % 60;
    CString timeStr;
    timeStr.Format(_T("%02d:%02d"), minutes, seconds);
    return timeStr;
}

void GameUI::ShowMessage(const CString& message)
{
    MessageBox(message, _T("Domino Puzzle"), MB_OK | MB_ICONINFORMATION);
}

// Printing support (stubs)
BOOL GameUI::OnPreparePrinting(CPrintInfo* pInfo)
{
    return DoPreparePrinting(pInfo);
}

void GameUI::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/) {}
void GameUI::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/) {}

#ifdef _DEBUG
void GameUI::AssertValid() const
{
    CView::AssertValid();
}

void GameUI::Dump(CDumpContext& dc) const
{
    CView::Dump(dc);
}
#endif