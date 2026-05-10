#pragma once
#include "Person.h"
#include <ostream>

class Patient : public Person {
private:
    int    age;
    char   gender[4];   
    float  balance;

public:
    Patient();
    Patient(int id, const char* name, int age, char gender,
        const char* contact, const char* password, float balance);
    ~Patient() override;

    int         getAge()     const;
    char        getGender()  const;
    float       getBalance() const;

    void setBalance(float b);

    Patient& operator+=(float amount);          
    Patient& operator-=(float amount);         
    bool     operator==(const Patient& other) const; 
    friend std::ostream& operator<<(std::ostream& os, const Patient& p);

    void displayMenu() override;
    void saveToFile()  override;

    void toFileLine(char* buf, int maxLen) const;

    static Patient fromFileLine(const char* line);
};
