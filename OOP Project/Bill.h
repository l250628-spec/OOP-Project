#pragma once
#include <ostream>

class Bill {
public:
    enum Status { UNPAID, PAID, CANCELLED };

private:
    int    billId;
    int    patientId;
    int    appointmentId;
    float  amount;
    Status status;
    char   date[12];     

public:
    Bill();
    Bill(int billId, int patientId, int appointmentId,
        float amount, Status status, const char* date);
    ~Bill();

    int         getId()        const;
    int         getPatientId()     const;
    int         getAppointmentId() const;
    float       getAmount()        const;
    Status      getStatus()        const;
    const char* getStatusStr()     const;
    const char* getDate()          const;

    void setStatus(Status s);

    int daysFromToday() const;

    void   toFileLine(char* buf, int maxLen) const;
    static Bill fromFileLine(const char* line);
    static Status parseStatus(const char* s);
};