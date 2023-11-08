#include "stdafx.h"         // Ön derlenmiş başlık dosyasını dahil et
#include "mylog.h"          // "mylog" sınıfının tanımını içeren başlık dosyasını dahil et
#include "Globals.h"        // Global değişkenleri içeren başlık dosyasını dahil et
#include "WM_Command.h"     // "WM_Command" sınıfının tanımını içeren başlık dosyasını dahil et

enum Operations { Add, Del, Move, Resize, Edit, Texting };  // İşlemler adlı bir sıralı liste (enum) tanımla

void mylog::Undo()
{
    if (ops.empty())
        return;

    json J = ops.back();
    ops.pop_back();
    jredo = J;

    if (J.empty())
        return;

    int op = J["operation"];
    switch (op)
    {
    case Add:  // Ekle işlemini geri al
    {
        // Sadece sondaki nesneyi sil
        delete Globals::var().drawObjList.back();
        Globals::var().drawObjList.pop_back();
        Globals::var().selectedObjectPtr = nullptr;
        Globals::var().hasSelected = false;
        break;
    }
    case Del:  // Sil işlemini geri al
    {
        int pos = J["which"];

        if (pos == -1)  // Yeni metni geri getirme işlemi için
        {
            Globals::var().newText.MakeStart(J["ptBeg"][0], J["ptBeg"][1], J["color"], J["backgroundColor"], J["lineWidth"]);
            Globals::var().newText.MakeEnd(J["ptEnd"][0], J["ptEnd"][1], 0, 0);
            vector<string> text = J["text"];
            Globals::var().newText.text = text;
            Globals::var().newText.inputPos.x = J["inputpos"][0];
            Globals::var().newText.inputPos.y = J["inputpos"][1];
            Globals::var().newText.CalculateCaretPosition();
            Globals::var().newText.startFinished = true;
            Globals::var().newText.endFinished = false;

            // Yeni metni seç
            Globals::var().selectedObjectPtr = &Globals::var().newText;
            Globals::var().hasSelected = true;
            break;
        }

        auto it = Globals::var().drawObjList.begin();
        std::advance(it, pos);
        DrawObj* ptr = *it;

        // Nesneyi belirtilen konumda geri ekleyin
        switch ((int)J["objectType"])
        {
        case Line:
        {
            LineObj newLine;
            newLine.MakeStart(J["ptBeg"][0], J["ptBeg"][1], J["color"], J["backgroundColor"], J["lineWidth"]);
            newLine.MakeEnd(J["ptEnd"][0], J["ptEnd"][1], 0, 0);
            Globals::var().drawObjList.insert(it, new LineObj(newLine));
            break;
        }
        case Rect:
        {
            RectangularObj newRect;
            newRect.MakeStart(J["ptBeg"][0], J["ptBeg"][1], J["color"], J["backgroundColor"], J["lineWidth"]);
            newRect.MakeEnd(J["ptEnd"][0], J["ptEnd"][1], 0, 0);
            Globals::var().drawObjList.insert(it, new RectangularObj(newRect));
            break;
        }
        case Circle:
        {
            CircleObj newCircle;
            newCircle.MakeStart(J["ptBeg"][0], J["ptBeg"][1], J["color"], J["backgroundColor"], J["lineWidth"]);
            newCircle.MakeEnd(J["ptEnd"][0], J["ptEnd"][1], 0, 0);
            Globals::var().drawObjList.insert(it, new CircleObj(newCircle));
            break;
        }
        case Text:
        {
            TextObj newText;
            newText.MakeStart(J["ptBeg"][0], J["ptBeg"][1], J["color"], J["backgroundColor"], J["lineWidth"]);
            newText.MakeEnd(J["ptEnd"][0], J["ptEnd"][1], 0, 0);
            vector<string> text = J["text"];
            newText.text = text;
            Globals::var().drawObjList.insert(it, new TextObj(newText));
            break;
        }
        }

        // Silinen nesneyi serbest bırak
        delete ptr;
        break;
    }
    case Move:  // Taşı işlemini geri al
    {
        int pos = J["which"];

        if (pos == -1)  // Yeni metni taşıma işlemi için
        {
            int deltaX, deltaY;
            deltaX = J["deltax"];
            deltaY = J["deltay"];
            Globals::var().newText.ptBeg.x = J["start"][0];
            Globals::var().newText.ptBeg.y = J["start"][1];
            Globals::var().newText.ptEnd.x += deltaX;
            Globals::var().newText.ptEnd.y += deltaY;
            break;
        }

        auto it = Globals::var().drawObjList.begin();
        std::advance(it, pos);

        int deltaX, deltaY;
        deltaX = J["deltax"];
        deltaY = J["deltay"];
        (*it)->ptBeg.x = J["start"][0];
        (*it)->ptBeg.y = J["start"][1];
        (*it)->ptEnd.x += deltaX;
        (*it)->ptEnd.y += deltaY;
        break;
    }
    case Resize:  // Yeniden Boyutlandır işlemini geri al
    {
        int pos = J["which"];

        if (pos == -1)  // Yeni metni yeniden boyutlandırma işlemi için
        {
            Globals::var().newText.ptBeg.x = J["oldBegin"][0];
            Globals::var().newText.ptBeg.y = J["oldBegin"][1];
            Globals::var().newText.ptEnd.x = J["oldEnd"][0];
            Globals->var().newText.ptEnd.y = J["oldEnd"][1];
            break;
        }

        auto it = Globals::var().drawObjList.begin();
        std::advance(it, pos);

        (*it)->ptBeg.x = J["oldBegin"][0];
        (*it)->ptBeg.y = J["oldBegin"][1];
        (*it)->ptEnd.x = J["oldEnd"][0];
        (*it)->ptEnd.y = J["oldEnd"][1];
        break;
    }
    case Edit:  // Düzenle işlemini geri al
    {
        int pos = J["which"];

        if (pos == -1)  // Yeni metni düzenleme işlemi için
        {
            Globals::var().newText.color = J["oldColor"];
            Globals::var().newText.backgroundColor = J["oldBgColor"];
            break;
        }

        auto it = Globals::var().drawObjList.begin();
        std::advance(it, pos);

        (*it)->color = J["oldColor"];

        if ((*it)->objectType < 4)
        {
            (*it)->lineWidth = J["oldWidth"];

            if ((*it)->objectType > 1)
            {
                (*it)->backgroundColor = J["oldBgColor"];
            }
        }
        break;
    }
    case Texting:  // Metni düzenleme işlemini geri al
    {
        int pos = J["which"];
        vector<string> vs = J["oldText"];
        POINT in;
        in.x = J["oldInput"][0];
        in.y = J["oldInput"][1];

        if (pos != -1)
        {
            auto it = Globals::var().drawObjList.begin();
            std::advance(it, pos);
            TextObj* t = dynamic_cast<TextObj*>(*it);
            t->text = vs;
            t->inputPos = in;
            t->CalculateCaretPosition();
        }
        else
        {
            // Yeni metni güncelle
            UpdateNewText(vs, in);
        }
        break;
    }
    default:
        break;
    }
}

void mylog::Redo()
{
    json J = jredo;

    if (J.empty())
        return;

    int op = J["operation"];
    switch (op)
    {
    case Add:  // Ekle işlemini tekrarla
    {
        // Nesneyi belirtilen konuma ekle
        switch ((int)J["objectType"])
        {
        case Line:
        {
            LineObj newLine;
            newLine.MakeStart(J["ptBeg"][0], J["ptBeg"][1], J["color"], J["backgroundColor"], J["lineWidth"]);
            newLine.MakeEnd(J["ptEnd"][0], J["ptEnd"][1], 0, 0);
            Globals::var().drawObjList.push_back(new LineObj(newLine));
            break;
        }
        case Rect:
        {
            RectangularObj newRect;
            newRect.MakeStart(J["ptBeg"][0], J["ptBeg"][1], J["color"], J["backgroundColor"], J["lineWidth"]);
            newRect.MakeEnd(J["ptEnd"][0], J["ptEnd"][1], 0, 0);
            Globals::var().drawObjList.push_back(new RectangularObj(newRect));
            break;
        }
        case Circle:
        {
            CircleObj newCircle;
            newCircle.MakeStart(J["ptBeg"][0], J["ptBeg"][1], J["color"], J["backgroundColor"], J["lineWidth"]);
            newCircle.MakeEnd(J["ptEnd"][0], J["ptEnd"][1], 0, 0);
            Globals::var().drawObjList.push_back(new CircleObj(newCircle));
            break;
        }
        case Text:
        {
            TextObj newText;
            newText.MakeStart(J["ptBeg"][0], J["ptBeg"][1], J["color"], J["backgroundColor"], J["lineWidth"]);
            newText.MakeEnd(J["ptEnd"][0], J["ptEnd"][1], 0, 0);
            vector<string> text = J["text"];
            newText.text = text;
            Globals::var().drawObjList.push_back(new TextObj(newText));
            break;
        }
        }
        break;
    }
    case Del:  // Sil işlemini tekrarla
    {
        // Belirtilen konumu sil
        int pos = J["which"];

        if (pos == -1)  // Yeni metni silme işlemi için
        {
            Globals::var().newText.Clean();
            break;
        }

        auto it = Globals::var().drawObjList.begin();
        std::advance(it, pos);
        DrawObj* ptr = *it;

        Globals::var().drawObjList.erase(it);
        delete ptr;

        Globals::var().selectedObjectPtr = nullptr;
        Globals::var().hasSelected = false;
        break;
    }
    case Move:  // Taşı işlemini tekrarla
    {
        // Belirtilen konumun ptBeg/End değerini geri taşı
        int pos = J["which"];
        if (pos == -1)  // Yeni metni taşıma işlemi için
        {
            int deltaX, deltaY;
            deltaX = J["deltax"];
            deltaY = J["deltay"];
            Globals::var().newText.ptBeg.x -= deltaX;
            Globals::var().newText.ptBeg.y -= deltaY;
            Globals::var().newText.ptEnd.x -= deltaX;
            Globals->var().newText.ptEnd.y -= deltaY;
            break;
        }

        auto it = Globals::var().drawObjList.begin();
        std::advance it, pos);

        int deltaX, deltaY;
        deltaX = J["deltax"];
        deltaY = J["deltay"];
        (*it)->ptBeg.x -= deltaX;
        (*it)->ptBeg.y -= deltaY;
        (*it)->ptEnd.x -= deltaX;
        (*it)->ptEnd.y -= deltaY;
        break;
    }
    case Resize:  // Yeniden boyutlandırma işlemini tekrarla
    {
        // Yeni noktalara geri dön
        int pos = J["which"];

        if (pos == -1)  // Yeni metni yeniden boyutlandırma işlemi için
        {
            Globals::var().newText.ptBeg.x = J["newBegin"][0];
            Globals::var().newText.ptBeg.y = J["newBegin"][1];
            Globals->var().newText.ptEnd.x = J["newEnd"][0];
            Globals->var().newText.ptEnd.y = J["newEnd"][1];
            break;
        }

        auto it = Globals::var().drawObjList.begin();
        std::advance it, pos);

        (*it)->ptBeg.x = J["newBegin"][0];
        (*it)->ptBeg.y = J["newBegin"][1];
        (*it)->ptEnd.x = J["newEnd"][0];
        (*it)->ptEnd.y = J["newEnd"][1];
        break;
    }
    case Edit:  // Düzenle işlemini tekrarla
    {
        // Yeni renk/genişlik değerlerine geri dön
        int pos = J["which"];
        if (pos == -1)  // Yeni metni düzenleme işlemi için
        {
            Globals::var().newText.color = J["newColor"];
            Globals::var().newText.backgroundColor = J["newBgColor"];
           
    break;
    }
auto it = Globals::var().drawObjList.begin();  // Geçerli bir öğe için bir iterator oluştur
std::advance(it, pos);  // Iteratorü "pos" adındaki değer kadar ilerlet

string test = J.dump();  // JSON verilerini bir dizeye dök ve "test" adlı değişkene kaydet

// Geçerli öğenin renk bilgisini JSON'dan alınan "newColor" ile güncelle
(*it)->color = J["newColor"];

if ((*it)->objectType < 4)  // Geçerli öğenin "objectType" değeri 4'ten küçükse
{
    // Öğenin kalınlığını JSON'dan alınan "newWidth" ile güncelle
    (*it)->lineWidth = J["newWidth"];

    if ((*it)->objectType > 1)  // Geçerli öğenin "objectType" değeri 1'den büyükse
    {
        // Öğenin arka plan rengini JSON'dan alınan "newBgColor" ile güncelle
        (*it)->backgroundColor = J["newBgColor"];
    }
}

break;  // Bu "case" bölümünün sona erdiğini belirtmek için kullanılır

// "Texting" durumu için "case" bölümü başlangıcı
case Texting:
{
    string test = J.dump();  // JSON verilerini bir dizeye dök ve "test" adlı değişkene kaydet
    int pos = J["which"];  // JSON'dan "which" değerini al ve "pos" adlı değişkene kaydet
    vector<string> vs = J["newText"];  // JSON'dan "newText" değerini al ve "vs" adlı vektöre kaydet
    POINT in;
    in.x = J["newInput"][0];  // JSON'dan "newInput" dizisinin ilk öğesini "in.x" değişkenine kaydet
    in.y = J["newInput"][1];  // JSON'dan "newInput" dizisinin ikinci öğesini "in.y" değişkenine kaydet

    if (pos != -1)  // Eğer "pos" -1 değilse (yani geçerli bir öğenin düzenlenmesi gerekiyorsa)
    {
        auto it = Globals::var().drawObjList.begin();  // Geçerli bir öğe için bir iterator oluştur
        std::advance(it, pos);  // Iteratorü "pos" adındaki değer kadar ilerlet
        TextObj* t = dynamic_cast<TextObj*>(*it);  // Öğeyi bir "TextObj" türüne dönüştür
        t->text = vs;  // Metni "vs" ile güncelle
        t->inputPos = in;  // Giriş pozisyonunu "in" ile güncelle
        t->CalculateCaretPosition();  // İmleç pozisyonunu hesapla
    }
    else  // Eğer "pos" -1 ise (yani yeni metin eklenmesi gerekiyorsa)
    {
        UpdateNewText(vs, in);  // Yeni metni "vs" vektörü ve "in" ile ekleyen işlevi çağır
    }
    break;  // Bu "case" bölümünün sona erdiğini belirtmek için kullanılır
}

default:  // Diğer durumlar için
{
    // Herhangi bir özel işlem yapmayacak, bu "case" bölümü sadece varsayılan durumu temsil ediyor
}
break;  // Bu "default" bölümünün sona erdiğini belirtmek için kullanılır
}

ops.push_back(jredo);  // "jredo" işlemini geri almak için operasyonları "ops" listesine ekler
jredo.clear();  // "jredo" işlem listesini temizler
}
void mylog::ClearLogs()
{
	ops.clear();  // "ops" adlı liste verisini temizle, tüm işlemleri kaldır
	jmove.clear();  // "jmove" adlı JSON nesnesini temizle
	jredo.clear();  // "jredo" adlı JSON nesnesini temizle
	ToggleUndoButton();  // Geri alma düğmesini güncellemek için bir işlevi çağır
}

// Belirtilen nesneyi işlem günlüğüne eklemek için kullanılan işlev
void mylog::PushObject(DrawObj* it, json jit)
{
	// JSON nesnesine öğe özelliklerini ekleyerek işlemi tanımla
	jit["objectType"] = it->objectType;
	jit["ptBeg"] = { it->ptBeg.x, it->ptBeg.y };
	jit["ptEnd"] = { it->ptEnd.x, it->ptEnd.y };
	jit["color"] = it->color;
	jit["backgroundColor"] = it->backgroundColor;
	jit["lineWidth"] = it->lineWidth;
	if (it->objectType == Text)
	{
		// Eğer öğe bir metin öğesi ise, metin ve giriş pozisyonunu JSON'a ekleyin
		TextObj* t = dynamic_cast<TextObj*>(it);
		vector<string> ls = t->text;
		jit["text"] = ls;
		jit["inputpos"] = { t->inputPos.x, t->inputPos.y };
	}
	ops.push_back(jit);  // İşlemi "ops" adlı işlem listesine ekleyin
	ToggleUndoButton();  // Geri alma düğmesini güncellemek için bir işlevi çağır
}

// Öğe eklemek için kullanılan işlemi başlat
void mylog::OP_add(DrawObj * it)
{
	json jit;
	jit["operation"] = Add;  // İşlem türünü "Add" olarak ayarlayın
	PushObject(it, jit);  // Öğeyi işlem günlüğüne eklemek için işlemi çağır
}

// Öğe silmek için kullanılan işlemi başlat
void mylog::OP_del(DrawObj * it, int pos)
{
	json jit;
	jit["operation"] = Del;  // İşlem türünü "Del" olarak ayarlayın
	jit["which"] = pos;  // Hangi öğenin silindiğini belirtmek için "which" özelliğini ayarlayın
	PushObject(it, jit);  // Öğeyi işlem günlüğüne eklemek için işlemi çağır
}

// Taşımadan önce yapılacak işlemleri başlat
void mylog::OP_moveStart(DrawObj * d, int pos)
{
	jmove.clear();  // "jmove" JSON nesnesini temizle
	// Eski ptBeg pozisyonunu bilme
	jmove["operation"] = Move;  // İşlem türünü "Move" olarak ayarlayın
	jmove["start"] = { d->ptBeg.x, d->ptBeg.y };  // Eski ptBeg pozisyonunu kaydedin
	jmove["which"] = pos;  // Hangi öğenin taşındığını belirtmek için "which" özelliğini ayarlayın
}

// Taşıma işlemi tamamlandıktan sonra yapılacak işlemleri başlat
void mylog::OP_moveEnd(DrawObj * d)
{
	int x = jmove["start"][0];
	int y = jmove["start"][1];
	int dx = x - d->ptBeg.x;
	int dy = y - d->ptBeg.y;
	jmove["deltax"] = dx;  // Yalnızca delta pozisyonunu kaydet
	jmove["deltay"] = dy;
	ops.push_back(jmove);  // İşlemi "ops" adlı işlem listesine ekleyin
	ToggleUndoButton();  // Geri alma düğmesini güncellemek için bir işlevi çağır
}

// Yeniden boyutlandırmadan önce yapılacak işlemleri başlat
void mylog::OP_sizeStart(DrawObj * d, int pos)
{
	jmove.clear();  // "jmove" JSON nesnesini temizle
	// Eski ptBeg pozisyonunu bilme
	jmove["operation"] = Resize;  // İşlem türünü "Resize" olarak ayarlayın
	jmove["oldBegin"] = { d->ptBeg.x, d->ptBeg.y };  // Eski ptBeg pozisyonunu kaydedin
	jmove["oldEnd"] = { d->ptEnd.x, d->ptEnd.y };  // Eski ptEnd pozisyonunu kaydedin
	jmove["which"] = pos;  // Hangi öğenin boyutlandırıldığını belirtmek için "which" özelliğini ayarlayın
}

// Yeniden boyutlandırma işlemi tamamlandıktan sonra yapılacak işlemleri başlat
void mylog::OP_sizeEnd(DrawObj * d)
{
	jmove["newBegin"] = { d->ptBeg.x, d->ptBeg.y };  // Yeni ptBeg pozisyonunu kaydedin
	jmove["newEnd"] = { d->ptEnd.x, d->ptEnd.y };  // Yeni ptEnd pozisyonunu kaydedin
	ops.push_back(jmove);  // İşlemi "ops" adlı işlem listesine ekleyin
	ToggleUndoButton();  // Geri alma düğmesini güncellemek için bir işlevi çağır
}

// Metni değiştirmeden önce yapılacak işlemleri başlat
void mylog::OP_textStart(DrawObj * d, int pos)
{
	jmove.clear();  // "jmove" JSON nesnesini temizle
	jmove["operation"] = Texting;  // İşlem türünü "Texting" olarak ayarlayın

	TextObj* t = dynamic_cast<TextObj*>(d);
	vector<string> vs = t->text;
	jmove["oldText"] = vs;  // Eski metni kaydedin
	jmove["oldInput"] = { t->inputPos.x, t->inputPos.y };  // Eski giriş pozisyonunu kaydedin
	jmove["which"] = pos;  // Hangi metinin değiştirildiğini belirtmek için "which" özelliğini ayarlayın
}

// Metni değiştirme işlemi tamamlandıktan sonra yapılacak işlemleri başlat
void mylog::OP_textEnd(DrawObj * d)
{
	TextObj* t = dynamic_cast<TextObj*>(d);
	vector<string> vs = t->text;
	jmove["newText"] = vs;  // Yeni metni kaydedin
	jmove["newInput"] = { t->inputPos.x, t->inputPos.y };  // Yeni giriş pozisyonunu kaydedin
	ops.push_back(jmove);  // İşlemi "ops" adlı işlem listesine ekleyin
	ToggleUndoButton();  // Geri alma düğmesini güncellemek için bir işlevi çağır
}

// Değişiklik işlemi başlat
void mylog::OP_modifyStart(DrawObj * d, int pos)
{
	jmove.clear();  // "jmove" JSON nesnesini temizle
	jmove["operation"] = Operations::Edit;  // İşlem türünü "Edit" olarak ayarlayın
	jmove["oldColor"] = d->color;  // Eski rengi kaydedin
	jmove["which"] = pos;  // Hangi öğenin değiştirildiğini belirtmek için "which" özelliğini ayarlayın
	if (d->objectType < 4)
	{
		jmove["oldWidth"] = d->lineWidth;  // Eski çizgi kalınlığını kaydedin
		if (d->objectType > 1)
		{
			jmove["oldBgColor"] = d->backgroundColor;  // Eski arka plan rengini kaydedin
		}
	}
	else // Tip = metin
	{
		jmove["oldBgColor"] = d->backgroundColor;  // Eski arka plan rengini kaydedin
	}
}

// Değişiklik işlemi tamamlandıktan sonra yapılacak işlemleri başlat
void mylog::OP_modifyEnd(DrawObj * d)
{
	jmove["newColor"] = d->color;  // Yeni rengi kaydedin
	if (d->objectType < 4)
	{
		jmove["newWidth"] = d->lineWidth;  // Yeni çizgi kalınlığını kaydedin
	}
	if (d->objectType > 1)
	{
		jmove["newBgColor"] = d->backgroundColor;  // Yeni arka plan rengini kaydedin
	}

	// Eğer değişiklik yoksa, işlemi eklemeyin
	bool modified = false;
	if (d->color != (int)jmove["oldColor"])
	{
		jmove["newColor"] = d->color;
		modified = true;
	}

	if (d->objectType < 4)
	{
		if (d->lineWidth != (int)jmove["oldWidth"])
		{
			modified = true;
		}
		if (d->objectType > 1 && d->objectType < 4)
		{
			if (d->backgroundColor != (int)jmove["oldBgColor"])
			{
				modified = true;
			}
		}
	}
	else  // Tip = metin
	{
		if (d->backgroundColor != (int)jmove["oldBgColor"])
		{
			modified = true;
		}
	}

	if (modified)
	{
		ops.push_back(jmove);  // İşlemi "ops" adlı işlem listesine ekleyin
		ToggleUndoButton();  // Geri alma düğmesini güncellemek için bir işlevi çağır
	}
}
