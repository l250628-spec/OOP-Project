# MediCore Hospital Management System

## Object Oriented Programming (OOP) Project – Spring 2026

### Student Information
- **Name:** Laiqa Nadeem
- **Roll Number:** 25L-0628
- **Section:** BCS-2B

---

# Project Overview

MediCore Hospital Management System is a console-based C++ application developed using Object Oriented Programming concepts. The system manages hospital operations including patients, doctors, appointments, billing, prescriptions, medical records and administration.

The system supports:
- Patient Login
- Doctor Login
- Admin Login
- Appointment Management
- Billing System
- Medical Records
- File Handling Persistence
- Exception Handling
- Operator Overloading
- Templates

All data is stored in `.txt` files and persists after program restart.

---

# Features

## Patient Features
- Book Appointment
- Cancel Appointment
- View Appointments
- View Medical Records
- View Bills
- Pay Bills
- Top Up Balance

## Doctor Features
- View Today's Appointments
- Mark Appointment Complete
- Mark Appointment No-Show
- Write Prescription
- View Patient Medical History

## Admin Features
- Add Doctor
- Remove Doctor
- View All Patients
- View All Doctors
- View All Appointments
- View Unpaid Bills
- Discharge Patient
- View Security Logs
- Generate Daily Report

---

# OOP Concepts Used

- Abstraction
- Encapsulation
- Inheritance
- Polymorphism
- Operator Overloading
- Templates
- Exception Handling
- Dynamic Memory Allocation
- File Handling

---

# Required Classes

## Base Class
- `Person` (Abstract Class)

## Derived Classes
- `Patient`
- `Doctor`
- `Admin`

## Other Classes
- `Appointment`
- `Bill`
- `Prescription`
- `Storage<T>`
- `FileHandler`
- `Validator`

## Exception Classes
- `HospitalException`
- `FileNotFoundException`
- `InvalidInputException`
- `InsufficientFundsException`
- `SlotUnavailableException`

---

# Operator Overloading

Implemented operator overloads:
- `Patient += amount`
- `Patient -= amount`
- `Patient == Patient`
- `Doctor == Doctor`
- `Appointment == Appointment`
- `<<` for Patient
- `<<` for Doctor
- `<<` for Appointment

---

# File Structure

## Source Files
```text
main.cpp

headers/
│
├── Person.h
├── Patient.h
├── Doctor.h
├── Admin.h
├── Appointment.h
├── Bill.h
├── Prescription.h
├── Storage.h
├── FileHandler.h
├── Validator.h
├── HospitalException.h
├── FileNotFoundException.h
├── InvalidInputException.h
├── InsufficientFundsException.h
├── SlotUnavailableException.h

src/
│
├── Person.cpp
├── Patient.cpp
├── Doctor.cpp
├── Admin.cpp
├── Appointment.cpp
├── Bill.cpp
├── Prescription.cpp
├── FileHandler.cpp
├── Validator.cpp
├── HospitalException.cpp
├── FileNotFoundException.cpp
├── InvalidInputException.cpp
├── InsufficientFundsException.cpp
├── SlotUnavailableException.cpp