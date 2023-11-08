#include "stdafx.h"
#include "DrawObj.h"

// DrawObj sınıfının kurucu fonksiyonu
DrawObj::DrawObj()
{
    color = 0;
    objectType = 0;
    startFinished = false;
    endFinished = false;
    backgroundColor = 0;
    lineWidth = 1;
}

// Nesneyi temizleyen fonksiyon
void DrawObj::Clean()
{
    ptBeg.x = 0;
    ptBeg.y = 0;
    ptEnd.x = 0;
    ptEnd.y = 0;
    startFinished = false;
    endFinished = false;
    color = 0;
}

// Fare ile nesnenin çevresini çizen fonksiyon
void DrawObj::PaintMouseOnRect(HDC hdc, int Xoffset, int Yoffset)
{
    // Nesnenin başlangıç ve bitiş noktaları aynıysa işlem yapma
    try
    {
        if (ptBeg.x == ptEnd.x && ptBeg.y == ptEnd.y)
            return;
    }
    catch (...)  // Eğer bir hata oluşursa (örneğin erişim ihlali), işlem yapma
    {
        return;
    }

    HPEN hpen, hpenOld;
    HGDIOBJ oldBrush = SelectObject(hdc, GetStockObject(NULL_BRUSH));
    hpen = CreatePen(PS_DOT, 1, RGB(0, 0, 0));
    hpenOld = (HPEN)SelectObject(hdc, hpen);
    SetBkColor(hdc, RGB(255, 255, 255));  // Kalem arka planını ayarla
    SetBkMode(hdc, OPAQUE);  // Kalem arka plan modunu mevcut arka planı üzerine yazma moduna ayarla

    int top = (ptBeg.y < ptEnd.y ? ptBeg.y : ptEnd.y) - Yoffset;
    int left = (ptBeg.x < ptEnd.x ? ptBeg.x : ptEnd.x) - Xoffset;
    int buttom = (ptBeg.y > ptEnd.y ? ptBeg.y : ptEnd.y) - Yoffset;
    int right = (ptBeg.x > ptEnd.x ? ptBeg.x : ptEnd.x) - Xoffset;
    Rectangle(hdc, left, top, right, buttom);

    // Kalemi geri yükle
    SelectObject(hdc, hpenOld);
    DeleteObject(hpen);
    SelectObject(hdc, oldBrush);
    DeleteObject(oldBrush);
    DeleteObject(hpenOld);
}

// Nesneye başlangıç noktası veren fonksiyon
void DrawObj::MakeStart(int x, int y, int currentColor, int currentBgColor, int currentLineWidth)
{
    ptBeg.x = x;
    ptBeg.y = y;
    ptEnd.x = x;
    ptEnd.y = y;
    startFinished = true;
    endFinished = false;
    color = currentColor;
    backgroundColor = currentBgColor;
    lineWidth = currentLineWidth;
}

// Nesneye bitiş noktası veren fonksiyon
void DrawObj::MakeEnd(int x, int y, int xCurrentScroll, int yCurrentScroll)
{
    ptEnd.x = x + xCurrentScroll;
    ptEnd.y = y + yCurrentScroll;
    endFinished = true;
}

// Fare ile nesnenin boyutunu değiştirme işlemini kontrol eden fonksiyon
int DrawObj::CheckMouseIsOnSizingOpint(int mouseX, int mouseY)
{
    int top = (ptBeg.y < ptEnd.y ? ptBeg.y : ptEnd.y);
    int left = (ptBeg.x < ptEnd.x ? ptBeg.x : ptEnd.x);
    int buttom = (ptBeg.y > ptEnd.y ? ptBeg.y : ptEnd.y);
    int right = (ptBeg.x > ptEnd.x ? ptBeg.x : ptEnd.x);

    if (mouseY < top - 4 || mouseY > buttom + 4 || mouseX < left - 4 || mouseX > right + 4)  
        return 0;

    if (mouseY >= top - 4 && mouseY <= top + 1 && mouseX >= left - 4 && mouseX <= left + 1)
        return 1;  // Sol üst
    else if (mouseX >= right - 1 && mouseX <= right + 4 && mouseY >= buttom - 1 && mouseY <= buttom + 4)
        return 2;  // Sağ alt
    else if (mouseX >= left - 4 && mouseX <= left + 1 && mouseY >= buttom - 1 && mouseY <= buttom + 4)
        return 4;  // Sol alt
    else if (mouseX >= right - 1 && mouseX <= right + 4 && mouseY >= top - 4 && mouseY <= top + 1)
        return 3;  // Sağ üst
    else if (mouseX >= left - 4 && mouseX <= left + 1 && mouseY >= (buttom + top) / 2 - 3 && mouseY <= (buttom + top) / 2 + 2)
        return 5;  // Sol
    else if (mouseX >= right - 1 && mouseX <= right + 4 && mouseY >= (buttom + top) / 2 - 3 && mouseY <= (buttom + top) / 2 + 2)
        return 6;  // Sağ
    else if (mouseX >= (right + left) / 2 - 3 && mouseX <= (right + left) / 2 + 2 && mouseY >= top - 4 && mouseY <= top + 1)
        return 7;  // Üst
    else if (mouseX >= (right + left) / 2 - 3 and mouseX <= (right + left) / 2 + 2 and mouseY >= buttom - 1 and mouseY <= buttom + 4)
        return 8;  // Alt

    if (objectType == 1 || objectType == 3)
    {
        if ((abs(mouseX - left) <= 1 || abs(mouseX - right) <= 1) && (mouseY > top && mouseY < buttom))
        {
            return 9;  // Seçim dikdörtgeninin üzerinde
        }
        if ((abs(mouseY - top) <= 1 || abs(mouseY - buttom) <= 1) && (mouseX > left && mouseX < right))
        {
            return 9;  // Seçim dikdörtgeninin üzerinde
        }
    }

    return 0;
}

// Nesneyi hareket ettirmek için kullanılan orijinal başlangıç ve bitiş noktalarını kaydeden fonksiyon
void DrawObj::StartToMove(int mouseX, int mouseY)
{
    originalMouseX = mouseX;
    originalMouseY = mouseY;
    originalBegin = ptBeg;
    originalEnd = ptEnd;
}

// Nesneyi hareket ettiren fonksiyon
void DrawObj::Moving(int mouseX, int mouseY)
{
    int deltaX, deltaY;
    deltaX = mouseX - originalMouseX;
    deltaY = mouseY - originalMouseY;

    ptBeg.x = originalBegin.x + deltaX;
    ptBeg.y = originalBegin.y + deltaY;
    ptEnd.x = originalEnd.x + deltaX;
    ptEnd.y = originalEnd.y + deltaY;

    if (ptBeg.x < 1)
    {
        int delta = 1 - ptBeg.x;
        ptBeg.x = 1;
        ptEnd.x += delta;
    }
    else if (ptEnd.x > 1994)
    {
        int delta = 1994 - ptEnd.x;
        ptEnd.x = 1994;
        ptBeg.x += delta;
    }

    if (ptBeg.y < 1)
    {
        int delta = 1 - ptBeg.y;
        ptBeg.y = 1;
        ptEnd.y += delta;
    }
    else if (ptEnd.y > 1995)
    {
        int delta = 1995 - ptEnd.y;
        ptEnd.y = 1995;
        ptBeg.y += delta;
    }
}

// Nesneyi yeniden boyutlandıran fonksiyon
void DrawObj::Resizing(int mouseX, int mouseY, int mode)
{
    int deltaX, deltaY;
    deltaX = mouseX - originalMouseX;
    deltaY = mouseY - originalMouseY;

    int beginDeltaX = 0, beginDeltaY = 0, endDeltaX = 0, endDeltaY = 0;

    switch (mode)
    {
    case 1:
        beginDeltaX = deltaX;
        beginDeltaY = deltaY;
        break;
    case 2:
        endDeltaX = deltaX;
        endDeltaY = deltaY;
        break;
    case 3:
        endDeltaX = deltaX;
        beginDeltaY = deltaY;
        break;
    case 4:
        beginDeltaX = deltaX;
        endDeltaY = deltaY;
        break;
    case 5:
        beginDeltaX = deltaX;
        break;
    case 6:
        endDeltaX = deltaX;
        break;
    case 7:
        beginDeltaY = deltaY;
        break;
    case 8:
        endDeltaY = deltaY;
        break;
    default:
        return;
        break;
    }

    if (originalBegin.x < originalEnd.x && originalBegin.y < originalEnd.y)
    {
        ptBeg.x = originalBegin.x + beginDeltaX;
        ptBeg.y = originalBegin.y + beginDeltaY;
        ptEnd.x = originalEnd.x + endDeltaX;
        ptEnd.y = originalEnd.y + endDeltaY;
    }
    else if (originalBegin.x < originalEnd.x && originalBegin.y >= originalEnd.y)
    {
        ptBeg.x = originalBegin.x + beginDeltaX;
        ptBeg.y = originalBegin.y + endDeltaY;
        ptEnd.x = originalEnd.x + endDeltaX;
        ptEnd.y = originalEnd.y + beginDeltaY;
    }
    else if (originalBegin.x >= originalEnd.x && originalBegin.y < originalEnd.y)
    {
        ptBeg.x = originalBegin.x + endDeltaX;
        ptBeg.y = originalBegin.y + beginDeltaY;
        ptEnd.x = originalEnd.x + beginDeltaX;
        ptEnd.y = originalEnd.y + endDeltaY;
    }
    else
    {
        ptBeg.x = originalBegin.x + endDeltaX;
        ptBeg.y = originalBegin.y + endDeltaY;
        ptEnd.x = originalEnd.x + beginDeltaX;
        ptEnd.y = originalEnd.y + beginDeltaY;
    }

    if (ptEnd.x > 1990)
        ptEnd.x = 1990;
    if (ptEnd.y > 1990)
        ptEnd.y = 1990;

    if (ptBeg.x < 1)
        ptBeg.x = 1;
    if (ptBeg.y < 1)
        ptBeg.y = 1;

    if (ptBeg.x > 1990)
        ptBeg.x = 1990;
    if (ptBeg.y > 1990)
        ptBeg.y = 1990;

    if (ptEnd.x < 1)
        ptEnd.x = 1;
    if (ptEnd.y < 1)
        ptEnd.y = 1;
}

// Kalem rengini değiştiren fonksiyon
HPEN DrawObj::SwitchColor()
{
    HPEN hPen;
    switch (color)
    {
    case 0:
        hPen = CreatePen(PS_SOLID, lineWidth, RGB(0, 0, 0));
        break;
    case 1:
        hPen = CreatePen(PS_SOLID, lineWidth, RGB(180, 180, 180));
        break;
    case 2:
        hPen = CreatePen(PS_SOLID, lineWidth, RGB(255, 0, 0));
        break;
    case 3:
        hPen = CreatePen(PS_SOLID, lineWidth, RGB(0, 255, 0));
        break;
    case 4:
        hPen = CreatePen(PS_SOLID, lineWidth, RGB(0, 0, 255));
        break;
    case 5:
        hPen = CreatePen(PS_SOLID, lineWidth, RGB(0, 255, 255));
        break;
    case 6:
        hPen = CreatePen(PS_SOLID, lineWidth, RGB(255, 255, 0));
        break;
    case 7:
        hPen = CreatePen(PS_SOLID, lineWidth, RGB(255, 0, 255));
        break;
    default:
        hPen = CreatePen(PS_SOLID, lineWidth, RGB(0, 0, 0));
    }
    return hPen;
}

// Kalem rengini geri bırakan fonksiyon
void DrawObj::ReleaseColor(HDC hdc)
{
    SelectObject(hdc, GetStockObject(DC_PEN));
    SelectObject(hdc, GetStockObject(NULL_BRUSH));
}
