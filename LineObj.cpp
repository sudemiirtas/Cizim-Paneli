#include "stdafx.h"
#include "DrawObj.h"

// LineObj sınıfının yapıcı fonksiyonu
LineObj::LineObj() { objectType = 1; }

// LineObj'un çizim işlemini gerçekleştiren fonksiyon
void LineObj::Paint(HDC hdc, int Xoffset, int Yoffset)
{
    if (!startFinished)
        return;
    if (ptBeg.x == ptEnd.x && ptBeg.y == ptEnd.y)
        return;

    HPEN hPen = SwitchColor();
    SelectObject(hdc, hPen);

    MoveToEx(hdc, ptBeg.x - Xoffset, ptBeg.y - Yoffset, NULL);
    LineTo(hdc, ptEnd.x - Xoffset, ptEnd.y - Yoffset);

    DeleteObject(hPen);
    ReleaseColor(hdc);
}

// Seçili çizim objesinin etrafına çizilen sınırları çizen fonksiyon
void LineObj::PaintSelectedRect(HDC hdc, int Xoffset, int Yoffset)
{
    if (ptBeg.x == ptEnd.x && ptBeg.y == ptEnd.y)
        return;

    HPEN hpen, hpenOld;
    hpen = CreatePen(PS_DASH, 1, RGB(0, 0, 0));
    hpenOld = (HPEN)SelectObject(hdc, hpen);
    HGDIOBJ oldBrush = SelectObject(hdc, GetStockObject(NULL_BRUSH));  // Boş çizgi çerçevesi çiz

    // Bir şey yap...
    int top = (ptBeg.y < ptEnd.y ? ptBeg.y : ptEnd.y) - Yoffset;
    int left = (ptBeg.x < ptEnd.x ? ptBeg.x : ptEnd.x) - Xoffset;
    int bottom = (ptBeg.y > ptEnd.y ? ptBeg.y : ptEnd.y) - Yoffset;
    int right = (ptBeg.x > ptEnd.x ? ptBeg.x : ptEnd.x) - Xoffset;
    Rectangle(hdc, left, top, right, bottom);

    // Kalemi dön
    SelectObject(hdc, hpenOld);
    DeleteObject(hpen);

    SelectObject(hdc, GetStockObject(WHITE_BRUSH));

    // 8 noktayı çiz
    {
        // Sol üst
        Rectangle(hdc, left - 4, top - 4, left + 1, top + 1);
        // Sağ üst
        Rectangle(hdc, right - 1, top - 4, right + 4, top + 1);
        // Sol alt
        Rectangle(hdc, left - 4, bottom - 1, left + 1, bottom + 4);
        // Sağ alt
        Rectangle(hdc, right - 1, bottom - 1, right + 4, bottom + 4);
        // Sol orta
        Rectangle(hdc, left - 4, (bottom + top) / 2 - 3, left + 1, (bottom + top) / 2 + 2);
        // Sağ orta
        Rectangle(hdc, right - 1, (bottom + top) / 2 - 3, right + 4, (bottom + top) / 2 + 2);
        // Üst orta
        Rectangle(hdc, (right + left) / 2 - 3, top - 4, (right + left) / 2 + 2, top + 1);
        // Alt orta
        Rectangle(hdc, (right + left) / 2 - 3, bottom - 1, (right + left) / 2 + 2, bottom + 4);
    }

    SelectObject(hdc, oldBrush);
    DeleteObject(oldBrush);
}

// Fare tıklamasının seçili nesnenin etrafında olup olmadığını kontrol eden fonksiyon
bool LineObj::CheckObjectCollision(int mouseX, mouseY)
{
    float slope, intercept, epsilon;
    float x1, y1, x2, y2;
    float px, py;
    float left, top, right, bottom; // Çizgi Segmenti İçin Sınırlama Kutusu
    float dx, dy;
    epsilon = 4.0 + lineWidth;  // Hata toleransı için

    x1 = ptBeg.x;
    y1 = ptBeg.y;
    x2 = ptEnd.x;
    y2 = ptEnd.y;
    px = mouseX;
    py = mouseY;

    dx = x2 - x1;
    dy = y2 - y1;

    if (dx == 0)  // Dikey çizgi
    {
        // Yalnızca Y koordinatını kontrol et
        if (y1 < y2)
        {
            top = y1;
            bottom = y2;
        }
        else
        {
            top = y1;
            bottom = y2;
        }

        if (px == x1 && py >= top && py <= bottom)
            return true;
        else
            return false;
    }

    if (x1 < x2)
    {
        left = x1;
        right = x2;
    }
    else
    {
        left = x2;
        right = x1;
    }

    if (y1 < y2)
    {
        top = y1;
        bottom = y2;
    }
    else
    {
        top = y2;
        bottom = y1;
    }

    if (mouseY < top || mouseY> bottom || mouseX < left || mouseX > right)  // İlk olarak fare aralık dışında ise, sadece false döndür
        return false;

    slope = dy / dx;
    intercept = y1 - slope * x1;

    if (slope * px + intercept > (py - epsilon) && slope * px + intercept < (py + epsilon))
    {
        if (px >= left && px <= right && py >= top && py <= bottom)
        {
            return true;
        }
    }
    return false;
}
