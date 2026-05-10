#pragma once
#include "Person.h"
#include <ostream>

class Doctor : public Person {
private:
    char  specialization[100];
    float fee;

public:
    Doctor();
    Doctor(int id, const char* name, const char* specialization,
        const char* contact, const char* password, float fee);
    ~Doctor() override;

    const char* getSpecialization() const;
    float       getFee()            const;

    bool matchesSpecialization(const char* query) const;

    bool operator==(const Doctor& other) const;    
    friend std::ostream& operator<<(std::ostream& os, const Doctor& d);

    void displayMenu() override;
    void saveToFile()  override;

    void   toFileLine(char* buf, int maxLen) const;
    static Doctor fromFileLine(const char* line);
};