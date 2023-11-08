#pragma once

#include "Windows.h"
#include <string>
#include <list>
#include <vector>
using namespace std;

enum DrawType { Line = 1, Rect, Circle, Text };

// DrawObj sınıfının tanımı
class DrawObj
{
public:
    POINT ptBeg;  // Başlangıç noktası
    POINT ptEnd;  // Bitiş noktası
    int objectType;  // Nesne türü: 1 = Çizgi; 2 = Dikdörtgen; 3 = Elips; 4 = Metin; 0 = Tanımsız
    int color;  // Renk
    int backgroundColor;  // Arka plan rengi (0 = saydam)
    int lineWidth;  // Çizgi kalınlığı (1pt - 5pt)
    bool startFinished, endFinished;  // Başlangıç ve bitiş noktalarının tamamlanıp tamamlanmadığını belirleyen bayraklar

    // Yapıcı fonksiyon
    DrawObj();
    void Clean();  // Nesneyi temizleme işlevi
    virtual void Paint(HDC hdc, int Xoffset, int Yoffset) = 0;
    virtual void PaintSelectedRect(HDC hdc, int Xoffset, int Yoffset) = 0;
    virtual bool CheckObjectCollision(int mouseX, int mouseY) = 0;

    void PaintMouseOnRect(HDC hdc, int Xoffset, int Yoffset);
    void MakeStart(int x, int y, int currentColor, int currentBgColor, int currentLineWidth);
    void MakeEnd(int x, int y, int xCurrentScroll, int yCurrentScroll);
    int CheckMouseIsOnSizingOpint(int mouseX, int mouseY);
    void StartToMove(int mouseX, int mouseY);
    virtual void Moving(int mouseX, int mouseY);
    void Resizing(int mouseX, int mouseY, int mode);

protected:
    int originalMouseX, originalMouseY;
    POINT originalBegin, originalEnd;

    HPEN SwitchColor();
    void ReleaseColor(HDC hdc);
};

// LineObj sınıfının tanımı
class LineObj : public DrawObj
{
public:
    LineObj();
    virtual ~LineObj() {}
    virtual void Paint(HDC hdc, int Xoffset, int Yoffset) override;
    virtual void PaintSelectedRect(HDC hdc, int Xoffset, int Yoffset) override;
    virtual bool CheckObjectCollision(int mouseX, mouseY);
};

// TextObj sınıfının tanımı
class TextObj : public DrawObj
{
private:
    int textWidth, textHeight, maxTextWidth;
    bool AddChar(int c);
    bool AddNewLine();
    bool Backspace();
    bool Del();

public:
    vector<string> text;
    POINT tailPos;
    POINT caretPos;
    POINT inputPos;

    // Yapıcı fonksiyon
    TextObj();
    void Clean();
    virtual ~TextObj();
    virtual void Paint(HDC hdc, int Xoffset, int Yoffset);
    virtual void PaintSelectedRect(HDC hdc, int Xoffset, int Yoffset);
    virtual bool CheckObjectCollision(int mouseX, int mouseY);
    bool KeyIn(int wParam);
    void ResizingText(int mouseX, int mouseY, int mode);
    bool CheckTextBoxBigEnough(int X, int Y);
    void CalculateCaretPosition();
    void Moving(int mouseX, int mouseY);
};

// RectangularObj sınıfının tanımı
class RectangularObj : public DrawObj
{
public:
    RectangularObj();
    virtual ~RectangularObj();
    virtual void Paint(HDC hdc, int Xoffset, int Yoffset) override;
    virtual void PaintSelectedRect(HDC hdc, int Xoffset, int Yoffset) override;
    virtual bool CheckObjectCollision(int mouseX, int mouseY) override;
};

// CircleObj sınıfının tanımı
class CircleObj : public DrawObj
{
public:
    CircleObj();
    virtual ~CircleObj();
    virtual void Paint(HDC hdc, int Xoffset, int Yoffset) override;
    virtual void PaintSelectedRect(HDC hdc, int Xoffset, int Yoffset) override;
    virtual bool CheckObjectCollision(int mouseX, int mouseY) override;
};
