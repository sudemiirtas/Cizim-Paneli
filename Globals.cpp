#include "stdafx.h"
#include "Globals.h"

Globals::Globals()
{
    // Dosya adı varsayılan olarak "My Paint" olarak ayarlanır.
    fileName = "My Paint";
    
    // Başlangıçta çizim modu 0 (Çizim modu) olarak ayarlanır.
    currentDrawMode = 0;
    
    // Başlangıçta değiştirme durumu 0 olarak ayarlanır.
    modifyState = 0;
    
    // Seçili nesne işaretçisi başlangıçta boş olarak ayarlanır.
    selectedObjectPtr = NULL;
    
    // Önceden seçilen nesne işaretçisi başlangıçta boş olarak ayarlanır.
    preSelectObject = NULL;
    
    // Mouse başlangıçta basılı değil olarak ayarlanır.
    mouseHasDown = false;
    
    // Çocuk pencere sınıfı için bir ad belirlenir. NULL olamaz!
    wcsncpy_s(szChildClass, L"123", 3);

    // Hata ayıklama modu başlangıçta kapalı olarak ayarlanır.
    debugMode = false;
}

Globals & Globals::var()
{
    static Globals g;
    return g;
}
