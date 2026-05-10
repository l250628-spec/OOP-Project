#pragma once
#include <ostream>

class Appointment
{
public:
    static const char* SLOTS[8];
    enum Status { PENDING, COMPLETED, CANCELLED, NOSHOW };

private:
    int appointmentId;
    int patientId;
    int doctorId;
    char date[12];       
    char timeSlot[6];   
    Status status;
public:
    Appointment();
    Appointment(int, int, int, const char*, const char*, Status status = PENDING);
    ~Appointment();

   
    int getAppointmentId() const;
    int getId() const;
    int getPatientId() const;
    int getDoctorId() const;
    const char* getDate() const;
    const char* getTimeSlot() const;
    Status getStatus() const;
    const char* getStatusStr() const;

   
    void setStatus(Status);

   
    bool operator==(const Appointment&) const;
    friend std::ostream& operator<<(std::ostream&, const Appointment&);

   
    int compareDateTo(const Appointment&) const; 

  
    void toFileLine(char*, int) const;
    static Appointment fromFileLine(const char*);

   
    static bool isValidSlot(const char*);

    static Status parseStatus(const char*);
};