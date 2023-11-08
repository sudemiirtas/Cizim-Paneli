#include "stdafx.h"
#include "DrawObj.h"

// CircleObj sınıfının kurucu fonksiyonu
CircleObj::CircleObj() { 
    objectType = 3; // ObjectType'ı dairesel olarak ayarla
}

// CircleObj sınıfının yıkıcı fonksiyonu
CircleObj::~CircleObj() {}

// Daireyi çizen fonksiyon
void CircleObj::Paint(HDC hdc, int Xoffset, int Yoffset)
{
    // Başlangıç tamamlanmadıysa, işlem yapma
	if (!startFinished)
		return;
    
    // Başlangıç ve bitiş noktaları aynıysa, işlem yapma
	if (ptBeg.x == ptEnd.x && ptBeg.y == ptEnd.y)
		return;
    
	HPEN hPen = SwitchColor(); // Rengi değiştir
	SelectObject(hdc, hPen); // Kalem nesnesini seç
    
	HBRUSH hBrush;
	switch (backgroundColor)
	{
	case 1:
		hBrush = CreateSolidBrush(RGB(180, 180, 180));
		break;
	case 2:
		hBrush = CreateSolidBrush(RGB(255, 0, 0));
		break;
	case 3:
		hBrush = CreateSolidBrush(RGB(0, 255, 0));
		break;
	case 4:
		hBrush = CreateSolidBrush(RGB(0, 0, 255));
		break;
	case 5:
		hBrush = CreateSolidBrush(RGB(0, 255, 255));
		break;
	case 6:
		hBrush = CreateSolidBrush(RGB(255, 255, 0));
		break;
	case 7:
		hBrush = CreateSolidBrush(RGB(255, 0, 255));
		break;
	default:
		hBrush = NULL;
	}

	if (hBrush != NULL)
		SelectObject(hdc, hBrush); // Fırça nesnesini seç
	else
		SelectObject(hdc, GetStockObject(NULL_BRUSH)); // Boş fırça kullan

	int top = (ptBeg.y < ptEnd.y ? ptBeg.y : ptEnd.y);
	int left = (ptBeg.x < ptEnd.x ? ptBeg.x : ptEnd.x);
	int buttom = (ptBeg.y > ptEnd.y ? ptBeg.y : ptEnd.y);
	int right = (ptBeg.x > ptEnd.x ? ptBeg.x : ptEnd.x);

	// Daireyi çiz
	Ellipse(hdc, left - Xoffset, top - Yoffset, right - Xoffset, buttom - Yoffset);

	DeleteObject(hPen); // Kalem nesnesini sil
	if (hBrush != NULL)
		DeleteObject(hBrush); // Fırça nesnesini sil
	ReleaseColor(hdc); // Renk değişikliğini geri al
}

// Seçili dikdörtgeni çizen fonksiyon
void CircleObj::PaintSelectedRect(HDC hdc, int Xoffset, int Yoffset)
{
    // Başlangıç ve bitiş noktaları aynıysa, işlem yapma
	if (ptBeg.x == ptEnd.x && ptBeg.y == ptEnd.y)
		return;

	HPEN hpen, hpenOld;
	hpen = CreatePen(PS_DASH, 1, RGB(0, 0, 0));
	hpenOld = (HPEN)SelectObject(hdc, hpen);
	HGDIOBJ oldBrush = SelectObject(hdc, GetStockObject(NULL_BRUSH));  // Boş bir kesik çizgi dikdörtgen çiz

	int top = (ptBeg.y < ptEnd.y ? ptBeg.y : ptEnd.y) - Yoffset;
	int left = (ptBeg.x < ptEnd.x ? ptBeg.x : ptEnd.x) - Xoffset;
	int bottom = (ptBeg.y > ptEnd.y ? ptBeg.y : ptEnd.y) - Yoffset;
	int right = (ptBeg.x > ptEnd.x ? ptBeg.x : ptEnd.x) - Xoffset;

	// Dikdörtgeni çiz
	Rectangle(hdc, left, top, right, bottom);

	// Kullanılan kalem nesnesini geri yükle
	SelectObject(hdc, hpenOld);
	DeleteObject(hpen);
	DeleteObject(hpenOld);

	SelectObject(hdc, GetStockObject(WHITE_BRUSH));  // 8 adet beyaz küçük dikdörtgen çiz
	// 8 köşeyi çiz
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
	SelectObject(hdc, oldBrush); // Kullanılan fırça nesnesini geri yükle
	DeleteObject(oldBrush); // Eski fırca nesnesini sil
}

// Nesnenin fareye çarpıp çarpmadığını kontrol eden fonksiyon
bool CircleObj::CheckObjectCollision(int mouseX, int mouseY)
{
	// Daire fonksiyonu: (x-h)^2/a^2 + (y-k)^2/b^2 = 1
	int top = (ptBeg.y < ptEnd.y ? ptBeg.y : ptEnd.y);
	int left = (ptBeg.x < ptEnd.x ? ptBeg.x : ptEnd.x);
	int buttom = (ptBeg.y > ptEnd.y ? ptBeg.y : ptEnd.y);
	int right = (ptBeg.x > ptEnd.x ? ptBeg.x : ptEnd.x);

    // Fare noktası dairenin dışındaysa, false döndür
	if (mouseY < top || mouseY > buttom || mouseX < left || mouseX > right)
		return false;

    // Dairenin yarı eksenleri
	float a = (float)(right - left) / 2.0;
	float b = (float)(buttom - top) / 2.0;

    // Dairenin merkezi
	float h = (float)(right + left) / 2.0;
	float k = (float)(buttom + top) / 2.0;

    // Daire denklemine göre sonucu hesapla
	float result = pow(mouseX - h, 2) / pow(a, 2) + pow(mouseY - k, 2) / pow(b, 2);
	float delta = 0.1;

	if (backgroundColor != 0)
	{
		if (result <= 1)
			return true;
	}
	else if (result <= 1 + delta && result >= 1 - delta)
	{
		return true;
	}

	return false;
}
