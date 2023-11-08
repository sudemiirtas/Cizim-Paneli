#pragma once

#include "json.hpp"
#include "DrawObj.h"
#include <windows.h>
#include <shobjidl.h> 
#include <string>
#include <iostream>
#include <fstream>

// Dosyayı kaydetmek için json'u kullanma
using json = nlohmann::json;
using namespace std;

//static PWSTR lastFilePath = NULL;

int SaveToFile(const list<DrawObj*>& saveList, std::string& fileName); // Save file
int ReadFromFile(list<DrawObj*>& readList, std::string& fileName);     // Open file
int SaveToLastFilePath(const list<DrawObj*>& saveList);                // Save as file
PWSTR BasicFileOpen();
PWSTR BasicFileSave();
std::string WstrToStr(const std::wstring &);
bool EndWith(const std::string &, const std::string &);
std::wstring StrToWstr(const std::string& s);
int DisplayConfirmClearMessageBox(const string fileName);
int DisplayConfirmNewFileMessageBox(const string fileName);
