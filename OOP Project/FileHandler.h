#pragma once
#include "Storage.h"
#include "Patient.h"
#include "Doctor.h"
#include "Admin.h"
#include "Appointment.h"
#include "Bill.h"
#include "Prescription.h"
#include "HospitalException.h"

class FileHandler {
public:
    static void loadPatients(Storage<Patient>& store);
    static void loadDoctors(Storage<Doctor>& store);
    static void loadAppointments(Storage<Appointment>& store);
    static void loadBills(Storage<Bill>& store);
    static void loadPrescriptions(Storage<Prescription>& store);
    static Admin loadAdmin();   

    static void appendPatient(const Patient& p);
    static void appendDoctor(const Doctor& d);
    static void appendAppointment(const Appointment& a);
    static void appendBill(const Bill& b);
    static void appendPrescription(const Prescription& p);

    static void updatePatient(const Patient& p);
    static void updateDoctor(const Doctor& d);
    static void updateAppointment(const Appointment& a);
    static void updateBill(const Bill& b);

    static void deletePatient(int id);
    static void deleteDoctor(int id);

    static void archivePatient(const Patient& p,
        Storage<Appointment>& appts,
        Storage<Bill>& bills,
        Storage<Prescription>& prescs);

    static void appendSecurityLog(const char* role,
        const char* enteredId,
        const char* result);

    static void readSecurityLog(char* buf, int maxLen);

private:
   
    static const char* PATIENTS_FILE;
    static const char* DOCTORS_FILE;
    static const char* ADMIN_FILE;
    static const char* APPOINTMENTS_FILE;
    static const char* BILLS_FILE;
    static const char* PRESCRIPTIONS_FILE;
    static const char* SECURITY_LOG_FILE;
    static const char* DISCHARGED_FILE;

    template<typename T>
    static void rewriteFile(const char* filename, Storage<T>& store);

    FileHandler() = delete;
};


