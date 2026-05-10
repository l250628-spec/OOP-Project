#pragma once
#include "Person.h"

class Admin : public Person {
public:
    Admin();
    Admin(int id, const char* name, const char* password);
    ~Admin() override;

    void displayMenu() override;
    void saveToFile()  override;

    static Admin fromFileLine(const char* line);
};