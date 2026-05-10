#pragma once

class Person {
protected:
    int    id;
    char   name[100];
    char   contact[20];
    char   password[50];

public:
    Person();
    Person(int id, const char* name, const char* contact, const char* password);
    virtual ~Person();

    int         getId()      const;
    const char* getName()    const;
    const char* getContact() const;

   
    virtual void displayMenu() = 0;
    virtual void saveToFile() = 0;

    
    bool checkPassword(const char* input) const;

    static bool strEqual(const char* a, const char* b);
    static void strCopy(char* dst, const char* src, int maxLen);
    static int  strLen(const char* s);

protected:
    static bool strEqualIgnoreCase(const char* a, const char* b);
};