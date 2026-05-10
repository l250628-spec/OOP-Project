#ifdef _MSC_VER
#  pragma warning(disable : 4996)
#  pragma warning(disable : 4101)
#endif

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <cstdio>
#include <ctime>
#include <cmath>

#include "Person.h"
#include "Patient.h"
#include "Doctor.h"
#include "Admin.h"
#include "Appointment.h"
#include "Bill.h"
#include "Prescription.h"
#include "Storage.h"
#include "FileHandler.h"
#include "Validator.h"
#include "HospitalException.h"


static const sf::Color P_BG(245, 244, 240);   
static const sf::Color P_SURFACE(255, 255, 253);   
static const sf::Color P_SIDEBAR(30, 32, 54);   
static const sf::Color P_SIDE_HOV(48, 51, 82);   
static const sf::Color P_SIDE_SEL(99, 91, 255);  
static const sf::Color P_VIOLET(99, 91, 255);   
static const sf::Color P_VIOLET_L(123, 116, 255);   
static const sf::Color P_ROSE(244, 63, 94);   
static const sf::Color P_ROSE_L(251, 113, 133);
static const sf::Color P_EMERALD(16, 185, 129);   
static const sf::Color P_EMERALD_L(52, 211, 153);
static const sf::Color P_GOLD(245, 158, 11);   
static const sf::Color P_GOLD_L(251, 191, 36);
static const sf::Color P_SKY(14, 165, 233);   
static const sf::Color P_SKY_L(56, 189, 248);
static const sf::Color P_INK(22, 24, 38);   
static const sf::Color P_SUB(100, 102, 120);   
static const sf::Color P_RULE(220, 218, 212);   
static const sf::Color P_INP_LINE(180, 178, 172);   
static const sf::Color P_INP_ACT(99, 91, 255);   
static const sf::Color P_OVERLAY(22, 24, 38, 160); 

static const float SIDEBAR_W = 220.f;
static const float WIN_W = 1280.f;
static const float WIN_H = 800.f;
static const float CONTENT_X = SIDEBAR_W + 40.f;
static const float CONTENT_W = WIN_W - SIDEBAR_W - 80.f;

enum class Screen {
    ROLE_SELECT,
    LOGIN,
    PATIENT_MENU, DOCTOR_MENU, ADMIN_MENU,
    BOOK_APPT, CANCEL_APPT, VIEW_APPTS,
    VIEW_RECORDS, VIEW_BILLS, PAY_BILL, TOPUP,
    DOC_TODAY, DOC_MARK, DOC_PRESCRIPTION, DOC_HISTORY,
    ADM_ADD_DOC, ADM_VIEW_ALL, ADM_DISCHARGE,
    ADM_SECURITY, ADM_REPORT,
    MSG_BOX
};
enum class Role { NONE, PATIENT, DOCTOR, ADMIN };

static void itosBuf(int v, char* buf) {
    if (v == 0) { buf[0] = '0';buf[1] = '\0';return; }
    char t[16];int ti = 0;bool neg = (v < 0);if (neg)v = -v;
    while (v > 0) { t[ti++] = (char)('0' + v % 10);v /= 10; }
    int bi = 0; if (neg)buf[bi++] = '-';
    for (int j = ti - 1;j >= 0;--j)buf[bi++] = t[j]; buf[bi] = '\0';
}
static void ftosBuf(float f, char* buf) {
    int w = (int)f, fr = (int)((f - (float)w) * 100.f + 0.5f);
    char wb[16], fb[8]; itosBuf(w, wb); itosBuf(fr, fb);
    int i = 0;
    for (int j = 0;wb[j];++j)buf[i++] = wb[j];
    buf[i++] = '.'; if (fr < 10)buf[i++] = '0';
    for (int j = 0;fb[j];++j)buf[i++] = fb[j]; buf[i] = '\0';
}
static void sapp(char* d, const char* s, int cap) {
    int dl = 0; while (d[dl])++dl;
    for (int i = 0;s[i] && dl < cap - 1;++i)d[dl++] = s[i]; d[dl] = '\0';
}

struct UiButton {
    sf::RectangleShape pill;
    sf::RectangleShape accent;   
    sf::Text           lbl;
    sf::Color          cn, ch, textCol;
    bool               hov = false;
    bool               sidebarStyle = false; 

    void initPill(float x, float y, float w, float h,
        const char* txt, sf::Font& font,
        sf::Color normal = sf::Color(99, 91, 255),
        sf::Color hover = sf::Color(123, 116, 255),
        sf::Color tc = sf::Color(255, 255, 255))
    {
        cn = normal; ch = hover; textCol = tc; sidebarStyle = false;
        pill.setPosition(x, y);
        pill.setSize(sf::Vector2f(w, h));
        pill.setFillColor(normal);

        accent.setPosition(x, y);
        accent.setSize(sf::Vector2f(4, h));
        accent.setFillColor(sf::Color(255, 255, 255, 180));

        lbl.setFont(font); lbl.setString(txt);
        lbl.setCharacterSize(14);
        lbl.setFillColor(tc);
        sf::FloatRect lb = lbl.getLocalBounds();
        lbl.setOrigin(lb.left + lb.width / 2.f, lb.top + lb.height / 2.f);
        lbl.setPosition(x + w / 2.f, y + h / 2.f);
    }

    void initSide(float y, const char* txt, sf::Font& font,
        sf::Color selCol = sf::Color(99, 91, 255))
    {
        sidebarStyle = true;
        cn = sf::Color(0, 0, 0, 0);  
        ch = sf::Color(48, 51, 82); 
        textCol = sf::Color(190, 192, 210);

        pill.setPosition(0, y);
        pill.setSize(sf::Vector2f(SIDEBAR_W, 46));
        pill.setFillColor(cn);

        accent.setPosition(0, y);
        accent.setSize(sf::Vector2f(3, 46));
        accent.setFillColor(selCol);

        lbl.setFont(font); lbl.setString(txt);
        lbl.setCharacterSize(13);
        lbl.setFillColor(textCol);
        sf::FloatRect lb = lbl.getLocalBounds();
        lbl.setOrigin(0, lb.top + lb.height / 2.f);
        lbl.setPosition(22.f, y + 23.f);
    }

    void update(sf::Vector2f m) {
        hov = pill.getGlobalBounds().contains(m);
        if (!sidebarStyle) pill.setFillColor(hov ? ch : cn);
        else pill.setFillColor(hov ? ch : cn);
    }

    void draw(sf::RenderWindow& w) {
        w.draw(pill);
        if (hov || sidebarStyle) w.draw(accent);
        w.draw(lbl);
    }

    bool hit(sf::Vector2f p) const {
        return pill.getGlobalBounds().contains(p);
    }
};

struct UiInput {
    sf::RectangleShape underline;   
    sf::RectangleShape activeLine;  
    sf::Text           caption, display;
    char   buf[256] = {};
    int    len = 0;
    bool   focused = false, isPw = false;

    void init(float x, float y, float w,
        const char* cap, sf::Font& font, bool pw = false)
    {
        isPw = pw; len = 0; buf[0] = '\0';

        caption.setFont(font); caption.setString(cap);
        caption.setCharacterSize(11);
        caption.setFillColor(P_SUB);
        caption.setPosition(x, y);

        underline.setPosition(x, y + 38);
        underline.setSize(sf::Vector2f(w, 1.5f));
        underline.setFillColor(P_INP_LINE);

        activeLine.setPosition(x, y + 38);
        activeLine.setSize(sf::Vector2f(0, 2.f));  
        activeLine.setFillColor(P_INP_ACT);

        display.setFont(font);
        display.setCharacterSize(15);
        display.setFillColor(P_INK);
        display.setPosition(x, y + 18);
    }

    void onChar(sf::Uint32 c) {
        if (!focused)return;
        if (c == '\b') { if (len > 0)buf[--len] = '\0'; }
        else if (c >= 32 && len < 254) { buf[len++] = (char)c;buf[len] = '\0'; }
    }

    void draw(sf::RenderWindow& w, float totalW = 360.f) {
        float tw = focused ? totalW : 0.f;
        activeLine.setSize(sf::Vector2f(tw, 2.f));

        w.draw(underline);
        w.draw(activeLine);
        w.draw(caption);

        if (isPw) {
            char s[256]; for (int i = 0;i < len;++i)s[i] = '*'; s[len] = '\0';
            display.setString(s);
        }
        else {
            display.setString(buf);
        }
        // cursor char
        char shown[258];
        Person::strCopy(shown, display.getString().toAnsiString().c_str(), 258);
        if (focused) { sapp(shown, "|", 258); }
        display.setString(shown);
        w.draw(display);
    }
    void clear() { len = 0; buf[0] = '\0'; }
};

static Storage<Patient>      gP;
static Storage<Doctor>       gD;
static Storage<Appointment>  gA;
static Storage<Bill>         gB;
static Storage<Prescription> gRx;
static Admin                 gAdmin;

static Screen gScreen = Screen::ROLE_SELECT;
static Screen gRetScreen = Screen::ROLE_SELECT;
static Role   gRole = Role::NONE;
static int    gUid = -1;
static int    gFails = 0;
static char   gMsg[512] = {};
static char   gList[10240] = {};
static float  gScroll = 0.f;

static UiButton sideNav[12];
static int      sideNavCount = 0;

static void lClear() { gList[0] = '\0'; }
static void lAdd(const char* s) { sapp(gList, s, 10240); }

static void sortAscDate(Appointment* a, int n) {
    for (int i = 0;i < n - 1;++i)
        for (int j = 0;j < n - i - 1;++j)
            if (a[j].compareDateTo(a[j + 1]) > 0) {
                Appointment t = a[j];a[j] = a[j + 1];a[j + 1] = t;
            }
}
static void sortDescDate(Appointment* a, int n) {
    for (int i = 0;i < n - 1;++i)
        for (int j = 0;j < n - i - 1;++j)
            if (a[j].compareDateTo(a[j + 1]) < 0) {
                Appointment t = a[j];a[j] = a[j + 1];a[j + 1] = t;
            }
}

static void buildMyAppts() {
    lClear();
    lAdd("ID   | Date        | Slot  | Status     | Doctor\n");
    lAdd("-----+-------------+-------+------------+--------------------\n");
    Appointment tmp[100]; int tc = 0;
    for (int i = 0;i < gA.size();++i)
        if (gA[i].getPatientId() == gUid) tmp[tc++] = gA[i];
    sortAscDate(tmp, tc);
    for (int i = 0;i < tc;++i) {
        char ln[256]; ln[0] = '\0';
        char id[8]; itosBuf(tmp[i].getAppointmentId(), id);
        sapp(ln, id, 256);sapp(ln, " | ", 256);
        sapp(ln, tmp[i].getDate(), 256);sapp(ln, " | ", 256);
        sapp(ln, tmp[i].getTimeSlot(), 256);sapp(ln, " | ", 256);
        sapp(ln, tmp[i].getStatusStr(), 256);sapp(ln, "     | ", 256);
        Doctor* d = gD.findById(tmp[i].getDoctorId());
        sapp(ln, d ? d->getName() : "?", 256);sapp(ln, "\n", 256);
        lAdd(ln);
    }
    if (tc == 0) lAdd("No appointments found.\n");
}
static void buildMyBills(bool unpaidOnly) {
    lClear();
    lAdd("Bill | Appt | Amount (PKR) | Status     | Date\n");
    lAdd("-----+------+--------------+------------+-----------\n");
    float tot = 0.f; int cnt = 0;
    for (int i = 0;i < gB.size();++i) {
        if (gB[i].getPatientId() != gUid) continue;
        if (unpaidOnly && gB[i].getStatus() != Bill::UNPAID) continue;
        ++cnt;
        char ln[256];ln[0] = '\0';
        char id[8];itosBuf(gB[i].getId(), id);
        sapp(ln, id, 256);sapp(ln, " | ", 256);
        char ai[8];itosBuf(gB[i].getAppointmentId(), ai);
        sapp(ln, ai, 256);sapp(ln, "   | ", 256);
        char am[16];ftosBuf(gB[i].getAmount(), am);
        sapp(ln, am, 256);sapp(ln, "         | ", 256);
        sapp(ln, gB[i].getStatusStr(), 256);sapp(ln, "   | ", 256);
        sapp(ln, gB[i].getDate(), 256);sapp(ln, "\n", 256);
        lAdd(ln);
        if (gB[i].getStatus() == Bill::UNPAID) tot += gB[i].getAmount();
    }
    if (cnt == 0) { lAdd("No bills found.\n");return; }
    char ts[16];ftosBuf(tot, ts);
    lAdd("\nTotal outstanding (PKR): ");lAdd(ts);lAdd("\n");
}
static void buildAllPatients() {
    lClear();
    lAdd("ID  | Name                 | Age | Balance (PKR) | Unpaid\n");
    lAdd("----+----------------------+-----+---------------+-------\n");
    for (int i = 0;i < gP.size();++i) {
        int ub = 0;
        for (int j = 0;j < gB.size();++j)
            if (gB[j].getPatientId() == gP[i].getId() && gB[j].getStatus() == Bill::UNPAID)++ub;
        char ln[256];ln[0] = '\0';
        char id[8];itosBuf(gP[i].getId(), id);
        sapp(ln, id, 256);sapp(ln, " | ", 256);
        sapp(ln, gP[i].getName(), 256);sapp(ln, "     | ", 256);
        char ag[8];itosBuf(gP[i].getAge(), ag);
        sapp(ln, ag, 256);sapp(ln, "  | ", 256);
        char bl[16];ftosBuf(gP[i].getBalance(), bl);
        sapp(ln, bl, 256);sapp(ln, "          | ", 256);
        char ubs[8];itosBuf(ub, ubs);
        sapp(ln, ubs, 256);sapp(ln, "\n", 256);
        lAdd(ln);
    }
    if (gP.size() == 0)lAdd("No patients registered.\n");
}
static void buildAllDoctors() {
    lClear();
    lAdd("ID  | Name                 | Specialization       | Fee\n");
    lAdd("----+----------------------+----------------------+---------\n");
    for (int i = 0;i < gD.size();++i) {
        char ln[256];ln[0] = '\0';
        char id[8];itosBuf(gD[i].getId(), id);
        sapp(ln, id, 256);sapp(ln, " | ", 256);
        sapp(ln, gD[i].getName(), 256);sapp(ln, "     | ", 256);
        sapp(ln, gD[i].getSpecialization(), 256);sapp(ln, "      | ", 256);
        char fee[16];ftosBuf(gD[i].getFee(), fee);
        sapp(ln, fee, 256);sapp(ln, "\n", 256);
        lAdd(ln);
    }
    if (gD.size() == 0)lAdd("No doctors registered.\n");
}
static void buildAllAppts() {
    lClear();
    lAdd("ID  | Patient         | Doctor          | Date       | Slot  | Status\n");
    lAdd("----+-----------------+-----------------+------------+-------+-----------\n");
    Appointment tmp[100];int tc = 0;
    for (int i = 0;i < gA.size() && tc < 100;++i)tmp[tc++] = gA[i];
    sortDescDate(tmp, tc);
    for (int i = 0;i < tc;++i) {
        char ln[256];ln[0] = '\0';
        char id[8];itosBuf(tmp[i].getAppointmentId(), id);
        sapp(ln, id, 256);sapp(ln, " | ", 256);
        Patient* p = gP.findById(tmp[i].getPatientId());
        Doctor* d = gD.findById(tmp[i].getDoctorId());
        sapp(ln, p ? p->getName() : "?", 256);sapp(ln, "       | ", 256);
        sapp(ln, d ? d->getName() : "?", 256);sapp(ln, "       | ", 256);
        sapp(ln, tmp[i].getDate(), 256);sapp(ln, " | ", 256);
        sapp(ln, tmp[i].getTimeSlot(), 256);sapp(ln, " | ", 256);
        sapp(ln, tmp[i].getStatusStr(), 256);sapp(ln, "\n", 256);
        lAdd(ln);
    }
    if (tc == 0)lAdd("No appointments found.\n");
}
static void buildUnpaidBills() {
    lClear();
    lAdd("Bill | Patient          | Amount (PKR) | Date       | Note\n");
    lAdd("-----+------------------+--------------+------------+----------\n");
    bool any = false;
    for (int i = 0;i < gB.size();++i) {
        if (gB[i].getStatus() != Bill::UNPAID)continue;
        any = true;
        char ln[256];ln[0] = '\0';
        char id[8];itosBuf(gB[i].getId(), id);
        sapp(ln, id, 256);sapp(ln, " | ", 256);
        Patient* p = gP.findById(gB[i].getPatientId());
        sapp(ln, p ? p->getName() : "?", 256);sapp(ln, "       | ", 256);
        char am[16];ftosBuf(gB[i].getAmount(), am);
        sapp(ln, am, 256);sapp(ln, "         | ", 256);
        sapp(ln, gB[i].getDate(), 256);sapp(ln, " | ", 256);
        if (gB[i].daysFromToday() > 7)sapp(ln, "[OVERDUE]", 256);
        sapp(ln, "\n", 256);
        lAdd(ln);
    }
    if (!any)lAdd("No unpaid bills.\n");
}
static void buildDailyReport() {
    lClear();
    time_t now = time(nullptr); struct tm* lt = localtime(&now);
    char today[12]; strftime(today, 12, "%d-%m-%Y", lt);
    lAdd("Daily Report  —  ");lAdd(today);lAdd("\n");
    lAdd("================================================\n");
    int total = 0, pend = 0, comp = 0, nosh = 0, canc = 0; float rev = 0.f;
    for (int i = 0;i < gA.size();++i) {
        if (!Person::strEqual(gA[i].getDate(), today))continue;
        ++total;
        switch (gA[i].getStatus()) {
        case Appointment::PENDING:  ++pend;break;
        case Appointment::COMPLETED:++comp;break;
        case Appointment::NOSHOW:   ++nosh;break;
        case Appointment::CANCELLED:++canc;break;
        }
    }
    for (int i = 0;i < gB.size();++i)
        if (Person::strEqual(gB[i].getDate(), today) && gB[i].getStatus() == Bill::PAID)
            rev += gB[i].getAmount();
    char t[16];
    itosBuf(total, t);lAdd("Total appointments : ");lAdd(t);lAdd("\n");
    itosBuf(pend, t); lAdd("  Pending          : ");lAdd(t);lAdd("\n");
    itosBuf(comp, t); lAdd("  Completed        : ");lAdd(t);lAdd("\n");
    itosBuf(nosh, t); lAdd("  No-show          : ");lAdd(t);lAdd("\n");
    itosBuf(canc, t); lAdd("  Cancelled        : ");lAdd(t);lAdd("\n");
    char r[16];ftosBuf(rev, r);
    lAdd("Revenue (PKR)      : ");lAdd(r);lAdd("\n\n");
    lAdd("Doctor summary (today):\n");
    lAdd("Doctor              | Comp | Pend | No-show\n");
    lAdd("--------------------+------+------+--------\n");
    for (int di = 0;di < gD.size();++di) {
        int dc = 0, dp = 0, dn = 0;
        for (int i = 0;i < gA.size();++i) {
            if (gA[i].getDoctorId() != gD[di].getId())continue;
            if (!Person::strEqual(gA[i].getDate(), today))continue;
            switch (gA[i].getStatus()) {
            case Appointment::COMPLETED:++dc;break;
            case Appointment::PENDING:  ++dp;break;
            case Appointment::NOSHOW:   ++dn;break;
            default:break;
            }
        }
        if (dc + dp + dn == 0)continue;
        char ln[128];ln[0] = '\0';
        sapp(ln, gD[di].getName(), 128);sapp(ln, "           | ", 128);
        char n[8];itosBuf(dc, n);sapp(ln, n, 128);sapp(ln, "    | ", 128);
        itosBuf(dp, n);sapp(ln, n, 128);sapp(ln, "    | ", 128);
        itosBuf(dn, n);sapp(ln, n, 128);sapp(ln, "\n", 128);
        lAdd(ln);
    }
}

static sf::Font* gFont = nullptr;
static void dTxt(sf::RenderWindow& w, const char* s,
    float x, float y, unsigned sz = 14,
    sf::Color col = sf::Color(22, 24, 38)) {
    sf::Text t; t.setFont(*gFont); t.setString(s);
    t.setCharacterSize(sz); t.setFillColor(col);
    t.setPosition(x, y); w.draw(t);
}


static void dTxtR(sf::RenderWindow& w, const char* s,
    float rx, float y, unsigned sz = 14,
    sf::Color col = sf::Color(22, 24, 38)) {
    sf::Text t; t.setFont(*gFont); t.setString(s);
    t.setCharacterSize(sz); t.setFillColor(col);
    t.setPosition(rx - t.getLocalBounds().width, y);
    w.draw(t);
}

static void dHeading(sf::RenderWindow& w, const char* s, float y) {
    sf::RectangleShape bar(sf::Vector2f(4, 22));
    bar.setPosition(CONTENT_X, y + 2); bar.setFillColor(P_VIOLET);
    w.draw(bar);
    sf::Text t; t.setFont(*gFont); t.setString(s);
    t.setCharacterSize(20); t.setFillColor(P_INK);
    t.setPosition(CONTENT_X + 14, y); w.draw(t);
}

static void dRule(sf::RenderWindow& w, float y) {
    sf::RectangleShape r(sf::Vector2f(CONTENT_W, 1));
    r.setPosition(CONTENT_X, y); r.setFillColor(P_RULE);
    w.draw(r);
}


static void dCard(sf::RenderWindow& w, float x, float y, float cw, float ch) {
    
    sf::RectangleShape sh(sf::Vector2f(cw, ch));
    sh.setPosition(x + 3, y + 3); sh.setFillColor(sf::Color(180, 178, 172, 80));
    w.draw(sh);
    
    sf::RectangleShape c(sf::Vector2f(cw, ch));
    c.setPosition(x, y); c.setFillColor(P_SURFACE);
    c.setOutlineColor(P_RULE); c.setOutlineThickness(1);
    w.draw(c);
}

static void dScrollList(sf::RenderWindow& w,
    float x, float y, float pw, float ph) {
    dCard(w, x, y, pw, ph);
    const float lh = 19.f;
    float cy = y + 10.f - gScroll;
    const char* p = gList;
    while (*p) {
        const char* nl = p; while (*nl && *nl != '\n')++nl;
        char tmp[512]; int len = (int)(nl - p);
        if (len > 510)len = 510;
        for (int i = 0;i < len;++i)tmp[i] = p[i]; tmp[len] = '\0';
        if (cy > y - lh && cy < y + ph) {
            sf::Text t; t.setFont(*gFont); t.setString(tmp);
            t.setCharacterSize(13); t.setFillColor(P_INK);
            t.setPosition(x + 12, cy); w.draw(t);
        }
        cy += lh; p = (*nl == '\n') ? nl + 1 : nl;
    }
}


static void dBadge(sf::RenderWindow& w, const char* s,
    float x, float y, sf::Color bg, sf::Color fg) {
    sf::Text t; t.setFont(*gFont); t.setString(s);
    t.setCharacterSize(11); t.setFillColor(fg);
    sf::FloatRect lb = t.getLocalBounds();
    sf::RectangleShape pill(sf::Vector2f(lb.width + 16, 18));
    pill.setPosition(x, y); pill.setFillColor(bg);
    w.draw(pill);
    t.setPosition(x + 8, y + 2); w.draw(t);
}

static sf::Font* _sbFont = nullptr;

static void buildSidebarPatient() {
    sideNavCount = 0;
    const char* L[9] = {
        "  Book Appointment","  Cancel Appointment","  My Appointments",
        "  Medical Records", "  My Bills",           "  Pay Bill",
        "  Top Up Balance",  "──────────────────",   "  Logout"
    };
    for (int i = 0;i < 9;++i) {
        sideNav[i].initSide(120 + i * 52, L[i], *_sbFont, P_VIOLET);
    }
    sideNavCount = 9;
}
static void buildSidebarDoctor() {
    sideNavCount = 0;
    const char* L[7] = {
        "  Today's Appts","  Mark Complete",
        "  Mark No-Show", "  Write Prescription",
        "  Patient History","──────────────────","  Logout"
    };
    for (int i = 0;i < 7;++i) {
        sideNav[i].initSide(120 + i * 52, L[i], *_sbFont, P_SKY);
    }
    sideNavCount = 7;
}
static void buildSidebarAdmin() {
    sideNavCount = 0;
    const char* L[11] = {
        "  Add Doctor",    "  Remove Doctor",
        "  All Patients",  "  All Doctors",
        "  All Appointments","  Unpaid Bills",
        "  Discharge Patient","  Security Log",
        "  Daily Report",  "──────────────","  Logout"
    };
    for (int i = 0;i < 11;++i) {
        sideNav[i].initSide(100 + i * 50, L[i], *_sbFont, P_GOLD);
    }
    sideNavCount = 11;
}


int main() {
    
    FileHandler::loadPatients(gP);
    FileHandler::loadDoctors(gD);
    FileHandler::loadAppointments(gA);
    FileHandler::loadBills(gB);
    FileHandler::loadPrescriptions(gRx);
    try { gAdmin = FileHandler::loadAdmin(); }
    catch (FileNotFoundException&) { gAdmin = Admin(1, "Admin", "admin123"); }


    sf::RenderWindow window(
        sf::VideoMode((int)WIN_W, (int)WIN_H),
        "MediCore  —  Hospital Management System",
        sf::Style::Close | sf::Style::Titlebar
    );
    window.setFramerateLimit(60);
    sf::Font font;
    if (!font.loadFromFile("C:/Users/laiqa/source/repos/OOP Project/ARIAL.TTF")) {
    }
    gFont = &font; _sbFont = &font;

   
    static UiButton btn[16];
    static UiInput  inp[6];
    int bc = 0, ic = 0;

    auto resetUI = [&]() { bc = 0; ic = 0; gScroll = 0.f; sideNavCount = 0; };

    auto showMsg = [&](const char* msg, Screen ret) {
        Person::strCopy(gMsg, msg, 512);
        gRetScreen = ret; gScreen = Screen::MSG_BOX;
        resetUI();
        btn[0].initPill(WIN_W / 2 - 80, 460, 160, 42, "OK", font);
        bc = 1;
        };

    auto goBack = [&]() {
        if (gRole == Role::PATIENT) {
            gScreen = Screen::PATIENT_MENU; resetUI(); buildSidebarPatient();
            
            const char* L[8] = {
                "Book Appointment","Cancel Appointment","My Appointments","Medical Records",
                "My Bills","Pay Bill","Top Up Balance","Logout"
            };
            sf::Color CN[8] = { P_VIOLET,P_VIOLET,P_VIOLET,P_VIOLET,P_VIOLET,P_EMERALD,P_EMERALD,P_ROSE };
            sf::Color CH[8] = { P_VIOLET_L,P_VIOLET_L,P_VIOLET_L,P_VIOLET_L,P_VIOLET_L,P_EMERALD_L,P_EMERALD_L,P_ROSE_L };
            for (int i = 0;i < 8;++i) {
                float row = (float)(i / 2), col = (float)(i % 2);
                btn[i].initPill(CONTENT_X + col * 500, 170 + row * 90, 460, 64, L[i], font, CN[i], CH[i]);
            }
            bc = 8;
        }
        else if (gRole == Role::DOCTOR) {
            gScreen = Screen::DOCTOR_MENU; resetUI(); buildSidebarDoctor();
            const char* L[6] = { "Today's Appointments","Mark Appointment Complete","Mark Appointment No-Show","Write Prescription","Patient Medical History","Logout" };
            sf::Color CN[6] = { P_SKY,P_EMERALD,P_ROSE,P_SKY,P_SKY,P_ROSE };
            sf::Color CH[6] = { P_SKY_L,P_EMERALD_L,P_ROSE_L,P_SKY_L,P_SKY_L,P_ROSE_L };
            for (int i = 0;i < 6;++i) {
                float row = (float)(i / 2), col = (float)(i % 2);
                btn[i].initPill(CONTENT_X + col * 500, 170 + row * 90, 460, 64, L[i], font, CN[i], CH[i]);
            }
            bc = 6;
        }
        else {
            gScreen = Screen::ADMIN_MENU; resetUI(); buildSidebarAdmin();
            const char* L[10] = { "Add Doctor","Remove Doctor","View All Patients","View All Doctors","All Appointments","Unpaid Bills","Discharge Patient","Security Log","Daily Report","Logout" };
            sf::Color CN[10] = { P_GOLD,P_ROSE,P_GOLD,P_GOLD,P_GOLD,P_GOLD,P_ROSE,P_GOLD,P_GOLD,P_ROSE };
            sf::Color CH[10] = { P_GOLD_L,P_ROSE_L,P_GOLD_L,P_GOLD_L,P_GOLD_L,P_GOLD_L,P_ROSE_L,P_GOLD_L,P_GOLD_L,P_ROSE_L };
            for (int i = 0;i < 10;++i) {
                float row = (float)(i / 2), col = (float)(i % 2);
                btn[i].initPill(CONTENT_X + col * 500, 155 + row * 82, 460, 60, L[i], font, CN[i], CH[i]);
            }
            bc = 10;
        }
        };

    auto tryLogin = [&]()->bool {
        if (!Validator::isPositiveInt(inp[0].buf))return false;
        int id = Validator::parseInt(inp[0].buf);
        if (gRole == Role::PATIENT) { Patient* p = gP.findById(id); if (!p || !p->checkPassword(inp[1].buf))return false; gUid = id;return true; }
        else if (gRole == Role::DOCTOR) { Doctor* d = gD.findById(id); if (!d || !d->checkPassword(inp[1].buf))return false; gUid = id;return true; }
        else { if (gAdmin.getId() == id && gAdmin.checkPassword(inp[1].buf)) { gUid = id;return true; } }
        return false;
        };

    {
        btn[0].initPill(WIN_W / 2 - 160, 310, 310, 54, "  Patient  —  Outpatient Portal", font, P_VIOLET, P_VIOLET_L);
        btn[1].initPill(WIN_W / 2 - 160, 378, 310, 54, "  Doctor   —  Clinical Dashboard", font, P_SKY, P_SKY_L);
        btn[2].initPill(WIN_W / 2 - 160, 446, 310, 54, "  Admin    —  Control Panel", font, P_GOLD, P_GOLD_L);
        btn[3].initPill(WIN_W / 2 - 160, 530, 310, 40, "Exit", font, sf::Color(200, 198, 194), sf::Color(180, 178, 174), P_INK);
        bc = 4;
    }

    while (window.isOpen()) {

        sf::Vector2f mouse(sf::Mouse::getPosition(window).x,
            sf::Mouse::getPosition(window).y);
        for (int i = 0;i < bc;++i) btn[i].update(mouse);
        for (int i = 0;i < sideNavCount;++i) sideNav[i].update(mouse);

        sf::Event ev;
        while (window.pollEvent(ev)) {
            if (ev.type == sf::Event::Closed) window.close();

            if (ev.type == sf::Event::MouseWheelScrolled) {
                gScroll -= ev.mouseWheelScroll.delta * 22.f;
                if (gScroll < 0.f)gScroll = 0.f;
            }

            if (ev.type == sf::Event::TextEntered)
                for (int i = 0;i < ic;++i)inp[i].onChar(ev.text.unicode);

            if (ev.type == sf::Event::MouseButtonPressed &&
                ev.mouseButton.button == sf::Mouse::Left)
            {
                sf::Vector2f pt((float)ev.mouseButton.x, (float)ev.mouseButton.y);
                for (int i = 0;i < ic;++i)
                    inp[i].focused = inp[i].underline.getGlobalBounds().contains(
                        sf::Vector2f(pt.x, pt.y + 10));

                if (gScreen == Screen::ROLE_SELECT) {
                    if (btn[0].hit(pt)) { gRole = Role::PATIENT;gFails = 0; }
                    else if (btn[1].hit(pt)) { gRole = Role::DOCTOR;gFails = 0; }
                    else if (btn[2].hit(pt)) { gRole = Role::ADMIN;gFails = 0; }
                    else if (btn[3].hit(pt)) { window.close(); }
                    if (btn[0].hit(pt) || btn[1].hit(pt) || btn[2].hit(pt)) {
                        gScreen = Screen::LOGIN; resetUI();
                        sf::Color ac = (gRole == Role::PATIENT) ? P_VIOLET : (gRole == Role::DOCTOR) ? P_SKY : P_GOLD;
                        sf::Color ah = (gRole == Role::PATIENT) ? P_VIOLET_L : (gRole == Role::DOCTOR) ? P_SKY_L : P_GOLD_L;
                        inp[0].init(WIN_W / 2 - 200, 280, 400, "User ID", font);
                        inp[1].init(WIN_W / 2 - 200, 360, 400, "Password", font, true);
                        btn[0].initPill(WIN_W / 2 - 200, 430, 190, 46, "Sign In", font, ac, ah);
                        btn[1].initPill(WIN_W / 2 + 10, 430, 190, 46, "< Back", font, sf::Color(200, 198, 194), sf::Color(180, 178, 174), P_INK);
                        bc = 2; ic = 2;
                    }
                }

                else if (gScreen == Screen::LOGIN) {
                    if (btn[1].hit(pt)) {
                        gRole = Role::NONE;gScreen = Screen::ROLE_SELECT;resetUI();
                        btn[0].initPill(WIN_W / 2 - 160, 310, 310, 54, "  Patient  —  Outpatient Portal", font, P_VIOLET, P_VIOLET_L);
                        btn[1].initPill(WIN_W / 2 - 160, 378, 310, 54, "  Doctor   —  Clinical Dashboard", font, P_SKY, P_SKY_L);
                        btn[2].initPill(WIN_W / 2 - 160, 446, 310, 54, "  Admin    —  Control Panel", font, P_GOLD, P_GOLD_L);
                        btn[3].initPill(WIN_W / 2 - 160, 530, 310, 40, "Exit", font, sf::Color(200, 198, 194), sf::Color(180, 178, 174), P_INK);
                        bc = 4;
                    }
                    else if (btn[0].hit(pt)) {
                        if (tryLogin()) {
                            gFails = 0; goBack();
                        }
                        else {
                            ++gFails;
                            const char* rs = (gRole == Role::PATIENT) ? "Patient" : (gRole == Role::DOCTOR) ? "Doctor" : "Admin";
                            FileHandler::appendSecurityLog(rs, inp[0].buf, "FAILED");
                            if (gFails >= 3) showMsg("Account locked. Contact admin.", Screen::ROLE_SELECT);
                            else          showMsg("Invalid credentials. Please try again.", Screen::LOGIN);
                        }
                    }
                }

                else if (gScreen == Screen::MSG_BOX) {
                    if (btn[0].hit(pt)) {
                        Screen r = gRetScreen;
                        if (r == Screen::ROLE_SELECT) {
                            gScreen = r;resetUI();
                            btn[0].initPill(WIN_W / 2 - 160, 310, 310, 54, "  Patient  —  Outpatient Portal", font, P_VIOLET, P_VIOLET_L);
                            btn[1].initPill(WIN_W / 2 - 160, 378, 310, 54, "  Doctor   —  Clinical Dashboard", font, P_SKY, P_SKY_L);
                            btn[2].initPill(WIN_W / 2 - 160, 446, 310, 54, "  Admin    —  Control Panel", font, P_GOLD, P_GOLD_L);
                            btn[3].initPill(WIN_W / 2 - 160, 530, 310, 40, "Exit", font, sf::Color(200, 198, 194), sf::Color(180, 178, 174), P_INK);
                            bc = 4;
                        }
                        else if (r == Screen::LOGIN) {
                            gScreen = r;resetUI();
                            sf::Color ac = (gRole == Role::PATIENT) ? P_VIOLET : (gRole == Role::DOCTOR) ? P_SKY : P_GOLD;
                            sf::Color ah = (gRole == Role::PATIENT) ? P_VIOLET_L : (gRole == Role::DOCTOR) ? P_SKY_L : P_GOLD_L;
                            inp[0].init(WIN_W / 2 - 200, 280, 400, "User ID", font);
                            inp[1].init(WIN_W / 2 - 200, 360, 400, "Password", font, true);
                            btn[0].initPill(WIN_W / 2 - 200, 430, 190, 46, "Sign In", font, ac, ah);
                            btn[1].initPill(WIN_W / 2 + 10, 430, 190, 46, "< Back", font, sf::Color(200, 198, 194), sf::Color(180, 178, 174), P_INK);
                            bc = 2;ic = 2;
                        }
                        else if (r == Screen::BOOK_APPT) {
                            gScreen = r;resetUI();buildSidebarPatient();
                            inp[0].init(CONTENT_X, 180, CONTENT_W / 2 - 20, "Specialization", font);
                            inp[1].init(CONTENT_X, 270, CONTENT_W / 2 - 20, "Doctor ID", font);
                            inp[2].init(CONTENT_X, 360, CONTENT_W / 2 - 20, "Date  (DD-MM-YYYY)", font);
                            inp[3].init(CONTENT_X, 450, CONTENT_W / 2 - 20, "Time Slot  (09:00 – 16:00)", font);
                            btn[0].initPill(CONTENT_X, 530, 180, 44, "Book", font, P_VIOLET, P_VIOLET_L);
                            btn[1].initPill(CONTENT_X + 196, 530, 180, 44, "< Back", font, sf::Color(200, 198, 194), sf::Color(180, 178, 174), P_INK);
                            bc = 2;ic = 4;
                        }
                        else goBack();
                    }
                }

             
                else if (gScreen == Screen::PATIENT_MENU) {
                
                    if (btn[7].hit(pt)) {
                        gUid = -1;gRole = Role::NONE;gScreen = Screen::ROLE_SELECT;resetUI();
                        btn[0].initPill(WIN_W / 2 - 160, 310, 310, 54, "  Patient  —  Outpatient Portal", font, P_VIOLET, P_VIOLET_L);
                        btn[1].initPill(WIN_W / 2 - 160, 378, 310, 54, "  Doctor   —  Clinical Dashboard", font, P_SKY, P_SKY_L);
                        btn[2].initPill(WIN_W / 2 - 160, 446, 310, 54, "  Admin    —  Control Panel", font, P_GOLD, P_GOLD_L);
                        btn[3].initPill(WIN_W / 2 - 160, 530, 310, 40, "Exit", font, sf::Color(200, 198, 194), sf::Color(180, 178, 174), P_INK);
                        bc = 4;
                    }
                    else if (btn[0].hit(pt)) { 
                        gScreen = Screen::BOOK_APPT; resetUI(); buildSidebarPatient();
                        inp[0].init(CONTENT_X, 180, CONTENT_W / 2 - 20, "Specialization", font);
                        inp[1].init(CONTENT_X, 270, CONTENT_W / 2 - 20, "Doctor ID", font);
                        inp[2].init(CONTENT_X, 360, CONTENT_W / 2 - 20, "Date  (DD-MM-YYYY)", font);
                        inp[3].init(CONTENT_X, 450, CONTENT_W / 2 - 20, "Time Slot  (09:00 – 16:00)", font);
                        btn[0].initPill(CONTENT_X, 530, 180, 44, "Book", font, P_VIOLET, P_VIOLET_L);
                        btn[1].initPill(CONTENT_X + 196, 530, 180, 44, "< Back", font, sf::Color(200, 198, 194), sf::Color(180, 178, 174), P_INK);
                        bc = 2;ic = 4;
                    }
                    else if (btn[1].hit(pt)) { 
                        buildMyAppts(); gScreen = Screen::CANCEL_APPT; resetUI(); buildSidebarPatient();
                        inp[0].init(CONTENT_X, 540, 300, "Appointment ID to cancel", font);
                        btn[0].initPill(CONTENT_X, 598, 180, 44, "Cancel", font, P_ROSE, P_ROSE_L);
                        btn[1].initPill(CONTENT_X + 196, 598, 180, 44, "< Back", font, sf::Color(200, 198, 194), sf::Color(180, 178, 174), P_INK);
                        bc = 2;ic = 1;
                    }
                    else if (btn[2].hit(pt)) { buildMyAppts();gScreen = Screen::VIEW_APPTS;resetUI();buildSidebarPatient();btn[0].initPill(CONTENT_X, 710, 180, 40, "< Back", font, sf::Color(200, 198, 194), sf::Color(180, 178, 174), P_INK);bc = 1; }
                    else if (btn[3].hit(pt)) { 
                        lClear();lAdd("Date       | Doctor               | Medicines\n");
                        lAdd("-----------+----------------------+-------------------------------\n");
                        bool any = false;
                        for (int i = 0;i < gRx.size();++i) {
                            if (gRx[i].getPatientId() != gUid)continue; any = true;
                            char ln[512];ln[0] = '\0';
                            sapp(ln, gRx[i].getDate(), 512);sapp(ln, " | ", 512);
                            Doctor* d = gD.findById(gRx[i].getDoctorId());
                            sapp(ln, d ? d->getName() : "?", 512);sapp(ln, "          | ", 512);
                            sapp(ln, gRx[i].getMedicines(), 512);sapp(ln, "\n", 512);
                            lAdd(ln);
                        }
                        if (!any)lAdd("No medical records found.\n");
                        gScreen = Screen::VIEW_RECORDS;resetUI();buildSidebarPatient();
                        btn[0].initPill(CONTENT_X, 710, 180, 40, "< Back", font, sf::Color(200, 198, 194), sf::Color(180, 178, 174), P_INK);bc = 1;
                    }
                    else if (btn[4].hit(pt)) { buildMyBills(false);gScreen = Screen::VIEW_BILLS;resetUI();buildSidebarPatient();btn[0].initPill(CONTENT_X, 710, 180, 40, "< Back", font, sf::Color(200, 198, 194), sf::Color(180, 178, 174), P_INK);bc = 1; }
                    else if (btn[5].hit(pt)) { 
                        buildMyBills(true);gScreen = Screen::PAY_BILL;resetUI();buildSidebarPatient();
                        inp[0].init(CONTENT_X, 540, 300, "Bill ID to pay", font);
                        btn[0].initPill(CONTENT_X, 598, 180, 44, "Pay", font, P_EMERALD, P_EMERALD_L);
                        btn[1].initPill(CONTENT_X + 196, 598, 180, 44, "< Back", font, sf::Color(200, 198, 194), sf::Color(180, 178, 174), P_INK);
                        bc = 2;ic = 1;
                    }
                    else if (btn[6].hit(pt)) { 
                        gScreen = Screen::TOPUP;resetUI();buildSidebarPatient();
                        inp[0].init(CONTENT_X, 300, 380, "Amount to add (PKR)", font);
                        btn[0].initPill(CONTENT_X, 370, 180, 44, "Add Funds", font, P_EMERALD, P_EMERALD_L);
                        btn[1].initPill(CONTENT_X + 196, 370, 180, 44, "< Back", font, sf::Color(200, 198, 194), sf::Color(180, 178, 174), P_INK);
                        bc = 2;ic = 1;
                    }
                }

                else if (gScreen == Screen::BOOK_APPT) {
                    if (btn[1].hit(pt)) { goBack(); }
                    else if (btn[0].hit(pt)) {
                        int did = Validator::parseInt(inp[1].buf);
                        Doctor* doc = gD.findById(did);
                        const char* date = inp[2].buf, * slot = inp[3].buf;
                        if (!doc) showMsg("Doctor not found.", Screen::BOOK_APPT);
                        else if (!Validator::isValidDate(date)) showMsg("Invalid date. Use DD-MM-YYYY.", Screen::BOOK_APPT);
                        else if (!Validator::isValidTimeSlot(slot)) showMsg("Invalid slot. Use: 09:00 10:00 11:00 12:00 13:00 14:00 15:00 16:00", Screen::BOOK_APPT);
                        else {
                            Appointment cand(0, gUid, did, date, slot);
                            bool taken = false;
                            for (int i = 0;i < gA.size();++i) if (gA[i] == cand) { taken = true;break; }
                            if (taken) { try { throw SlotUnavailableException(); } catch (SlotUnavailableException& e) { showMsg(e.what(), Screen::BOOK_APPT); } }
                            else {
                                Patient* pat = gP.findById(gUid);
                                if (pat->getBalance() < doc->getFee()) { try { throw InsufficientFundsException(); } catch (InsufficientFundsException& e) { showMsg(e.what(), Screen::BOOK_APPT); } }
                                else {
                                    *pat -= doc->getFee(); FileHandler::updatePatient(*pat);
                                    int naid = gA.maxId() + 1;
                                    Appointment na(naid, gUid, did, date, slot);
                                    gA.add(na); FileHandler::appendAppointment(na);
                                    time_t now = time(nullptr); struct tm* lt = localtime(&now);
                                    char tod[12]; strftime(tod, 12, "%d-%m-%Y", lt);
                                    int nbid = gB.maxId() + 1;
                                    Bill nb(nbid, gUid, naid, doc->getFee(), Bill::UNPAID, tod);
                                    gB.add(nb); FileHandler::appendBill(nb);
                                    char msg[64] = "Appointment booked!  ID: ";
                                    char ids[8]; itosBuf(naid, ids); sapp(msg, ids, 64);
                                    showMsg(msg, Screen::PATIENT_MENU);
                                }
                            }
                        }
                    }
                }

                else if (gScreen == Screen::CANCEL_APPT) {
                    if (btn[1].hit(pt)) { goBack(); }
                    else if (btn[0].hit(pt)) {
                        int aid = Validator::parseInt(inp[0].buf);
                        Appointment* a = nullptr;
                        for (int i = 0;i < gA.size();++i)
                            if (gA[i].getAppointmentId() == aid && gA[i].getPatientId() == gUid && gA[i].getStatus() == Appointment::PENDING) { a = &gA[i];break; }
                        if (!a) { showMsg("Invalid appointment ID.", Screen::PATIENT_MENU); }
                        else {
                            Doctor* d = gD.findById(a->getDoctorId()); float fee = d ? d->getFee() : 0.f;
                            Patient* pat = gP.findById(gUid); *pat += fee;
                            FileHandler::updatePatient(*pat);
                            a->setStatus(Appointment::CANCELLED); FileHandler::updateAppointment(*a);
                            for (int i = 0;i < gB.size();++i)
                                if (gB[i].getAppointmentId() == aid) { gB[i].setStatus(Bill::CANCELLED);FileHandler::updateBill(gB[i]);break; }
                            char msg[64] = "Cancelled.  PKR "; char fs[16]; ftosBuf(fee, fs); sapp(msg, fs, 64); sapp(msg, " refunded.", 64);
                            showMsg(msg, Screen::PATIENT_MENU);
                        }
                    }
                }

                
                else if (gScreen == Screen::TOPUP) {
                    if (btn[1].hit(pt)) { goBack(); }
                    else if (btn[0].hit(pt)) {
                        try {
                            if (!Validator::isPositiveFloat(inp[0].buf)) throw InvalidInputException("Amount must be positive.");
                            Patient* pat = gP.findById(gUid);
                            *pat += Validator::parseFloat(inp[0].buf);
                            FileHandler::updatePatient(*pat);
                            char msg[64] = "Balance updated.  New: PKR "; char bs[16]; ftosBuf(pat->getBalance(), bs); sapp(msg, bs, 64);
                            showMsg(msg, Screen::PATIENT_MENU);
                        }
                        catch (InvalidInputException& e) { showMsg(e.what(), Screen::TOPUP); }
                    }
                }
                else if (gScreen == Screen::PAY_BILL) {
                    if (btn[1].hit(pt)) { goBack(); }
                    else if (btn[0].hit(pt)) {
                        int bid = Validator::parseInt(inp[0].buf);
                        Bill* bl = nullptr;
                        for (int i = 0;i < gB.size();++i)
                            if (gB[i].getId() == bid && gB[i].getPatientId() == gUid && gB[i].getStatus() == Bill::UNPAID) { bl = &gB[i];break; }
                        if (!bl) { showMsg("Invalid bill ID.", Screen::PATIENT_MENU); }
                        else {
                            Patient* pat = gP.findById(gUid);
                            if (pat->getBalance() < bl->getAmount()) { try { throw InsufficientFundsException(); } catch (InsufficientFundsException& e) { showMsg(e.what(), Screen::PATIENT_MENU); } }
                            else {
                                *pat -= bl->getAmount(); FileHandler::updatePatient(*pat);
                                bl->setStatus(Bill::PAID); FileHandler::updateBill(*bl);
                                char msg[64] = "Bill paid.  Balance: PKR "; char bs[16]; ftosBuf(pat->getBalance(), bs); sapp(msg, bs, 64);
                                showMsg(msg, Screen::PATIENT_MENU);
                            }
                        }
                    }
                }

                else if (gScreen == Screen::DOCTOR_MENU) {
                    if (btn[5].hit(pt)) {
                        gUid = -1;gRole = Role::NONE;gScreen = Screen::ROLE_SELECT;resetUI();
                        btn[0].initPill(WIN_W / 2 - 160, 310, 310, 54, "  Patient  —  Outpatient Portal", font, P_VIOLET, P_VIOLET_L);
                        btn[1].initPill(WIN_W / 2 - 160, 378, 310, 54, "  Doctor   —  Clinical Dashboard", font, P_SKY, P_SKY_L);
                        btn[2].initPill(WIN_W / 2 - 160, 446, 310, 54, "  Admin    —  Control Panel", font, P_GOLD, P_GOLD_L);
                        btn[3].initPill(WIN_W / 2 - 160, 530, 310, 40, "Exit", font, sf::Color(200, 198, 194), sf::Color(180, 178, 174), P_INK);
                        bc = 4;
                    }
                    else if (btn[0].hit(pt)) { 
                        time_t now = time(nullptr); struct tm* lt = localtime(&now);
                        char tod[12]; strftime(tod, 12, "%d-%m-%Y", lt);
                        lClear(); lAdd("ID  | Patient          | Time  | Status\n");
                        lAdd("----+------------------+-------+----------\n");
                        bool any = false;
                        for (int i = 0;i < gA.size();++i) {
                            if (gA[i].getDoctorId() != gUid || !Person::strEqual(gA[i].getDate(), tod))continue;
                            any = true; char ln[128];ln[0] = '\0';
                            char id[8];itosBuf(gA[i].getAppointmentId(), id);
                            sapp(ln, id, 128);sapp(ln, " | ", 128);
                            Patient* p = gP.findById(gA[i].getPatientId());
                            sapp(ln, p ? p->getName() : "?", 128);sapp(ln, "       | ", 128);
                            sapp(ln, gA[i].getTimeSlot(), 128);sapp(ln, " | ", 128);
                            sapp(ln, gA[i].getStatusStr(), 128);sapp(ln, "\n", 128);
                            lAdd(ln);
                        }
                        if (!any)lAdd("No appointments scheduled for today.\n");
                        gScreen = Screen::DOC_TODAY;resetUI();buildSidebarDoctor();
                        btn[0].initPill(CONTENT_X, 710, 180, 40, "< Back", font, sf::Color(200, 198, 194), sf::Color(180, 178, 174), P_INK);bc = 1;
                    }
                    else if (btn[1].hit(pt)) { 
                        gScreen = Screen::DOC_MARK;resetUI();buildSidebarDoctor();
                        inp[0].init(CONTENT_X, 280, 380, "Appointment ID", font);
                        inp[1].init(10, 760, 10, "", font); inp[1].buf[0] = '0';inp[1].buf[1] = '\0';inp[1].len = 1;
                        btn[0].initPill(CONTENT_X, 360, 220, 44, "Mark Complete", font, P_EMERALD, P_EMERALD_L);
                        btn[1].initPill(CONTENT_X + 236, 360, 180, 44, "< Back", font, sf::Color(200, 198, 194), sf::Color(180, 178, 174), P_INK);
                        bc = 2;ic = 2;
                    }
                    else if (btn[2].hit(pt)) { 
                        gScreen = Screen::DOC_MARK;resetUI();buildSidebarDoctor();
                        inp[0].init(CONTENT_X, 280, 380, "Appointment ID", font);
                        inp[1].init(10, 760, 10, "", font); inp[1].buf[0] = '1';inp[1].buf[1] = '\0';inp[1].len = 1;
                        btn[0].initPill(CONTENT_X, 360, 220, 44, "Mark No-Show", font, P_ROSE, P_ROSE_L);
                        btn[1].initPill(CONTENT_X + 236, 360, 180, 44, "< Back", font, sf::Color(200, 198, 194), sf::Color(180, 178, 174), P_INK);
                        bc = 2;ic = 2;
                    }
                    else if (btn[3].hit(pt)) { 
                        gScreen = Screen::DOC_PRESCRIPTION;resetUI();buildSidebarDoctor();
                        inp[0].init(CONTENT_X, 175, CONTENT_W / 2, "Completed Appointment ID", font);
                        inp[1].init(CONTENT_X, 265, CONTENT_W - 20, "Medicines  (Name Dose;Name Dose;...)", font);
                        inp[2].init(CONTENT_X, 380, CONTENT_W - 20, "Notes  (max 300 chars)", font);
                        btn[0].initPill(CONTENT_X, 470, 220, 44, "Save Prescription", font, P_SKY, P_SKY_L);
                        btn[1].initPill(CONTENT_X + 236, 470, 180, 44, "< Back", font, sf::Color(200, 198, 194), sf::Color(180, 178, 174), P_INK);
                        bc = 2;ic = 3;
                    }
                    else if (btn[4].hit(pt)) { 
                        gScreen = Screen::DOC_HISTORY;resetUI();buildSidebarDoctor();
                        inp[0].init(CONTENT_X, 280, 300, "Patient ID", font);
                        btn[0].initPill(CONTENT_X, 360, 180, 44, "View", font, P_SKY, P_SKY_L);
                        btn[1].initPill(CONTENT_X + 196, 360, 180, 44, "< Back", font, sf::Color(200, 198, 194), sf::Color(180, 178, 174), P_INK);
                        bc = 2;ic = 1;
                    }
                }

                else if (gScreen == Screen::DOC_MARK) {
                    if (btn[1].hit(pt)) { goBack(); }
                    else if (btn[0].hit(pt)) {
                        bool isNS = (inp[1].buf[0] == '1');
                        int aid = Validator::parseInt(inp[0].buf);
                        time_t now = time(nullptr); struct tm* lt = localtime(&now);
                        char tod[12]; strftime(tod, 12, "%d-%m-%Y", lt);
                        Appointment* a = nullptr;
                        for (int i = 0;i < gA.size();++i)
                            if (gA[i].getAppointmentId() == aid && gA[i].getDoctorId() == gUid &&
                                gA[i].getStatus() == Appointment::PENDING && Person::strEqual(gA[i].getDate(), tod)) {
                                a = &gA[i];break;
                            }
                        if (!a) { showMsg("Invalid ID or not pending today.", Screen::DOCTOR_MENU); }
                        else if (isNS) {
                            a->setStatus(Appointment::NOSHOW); FileHandler::updateAppointment(*a);
                            for (int i = 0;i < gB.size();++i) if (gB[i].getAppointmentId() == aid) { gB[i].setStatus(Bill::CANCELLED);FileHandler::updateBill(gB[i]);break; }
                            showMsg("Appointment marked as no-show.", Screen::DOCTOR_MENU);
                        }
                        else {
                            a->setStatus(Appointment::COMPLETED); FileHandler::updateAppointment(*a);
                            showMsg("Appointment marked as completed.", Screen::DOCTOR_MENU);
                        }
                    }
                }

                else if (gScreen == Screen::DOC_PRESCRIPTION) {
                    if (btn[1].hit(pt)) { goBack(); }
                    else if (btn[0].hit(pt)) {
                        int aid = Validator::parseInt(inp[0].buf);
                        Appointment* a = nullptr;
                        for (int i = 0;i < gA.size();++i)
                            if (gA[i].getAppointmentId() == aid && gA[i].getDoctorId() == gUid && gA[i].getStatus() == Appointment::COMPLETED) { a = &gA[i];break; }
                        if (!a) { showMsg("Invalid ID or not completed.", Screen::DOCTOR_MENU); }
                        else {
                            bool ex = false;
                            for (int i = 0;i < gRx.size();++i) if (gRx[i].getAppointmentId() == aid) { ex = true;break; }
                            if (ex) showMsg("Prescription already written.", Screen::DOCTOR_MENU);
                            else {
                                int nid = gRx.maxId() + 1;
                                Prescription np(nid, aid, a->getPatientId(), gUid, a->getDate(), inp[1].buf, inp[2].buf);
                                gRx.add(np); FileHandler::appendPrescription(np);
                                showMsg("Prescription saved.", Screen::DOCTOR_MENU);
                            }
                        }
                    }
                }

               
                else if (gScreen == Screen::DOC_HISTORY) {
                    if (btn[1].hit(pt)) { goBack(); }
                    else if (btn[0].hit(pt)) {
                        int pid = Validator::parseInt(inp[0].buf);
                        Patient* pat = gP.findById(pid);
                        bool access = false;
                        for (int i = 0;i < gA.size();++i)
                            if (gA[i].getPatientId() == pid && gA[i].getDoctorId() == gUid && gA[i].getStatus() == Appointment::COMPLETED) { access = true;break; }
                        if (!pat || !access) { showMsg("Access denied.", Screen::DOCTOR_MENU); }
                        else {
                            lClear(); lAdd("Date       | Medicines\n");
                            lAdd("-----------+--------------------------------------------\n");
                            bool any = false;
                            for (int i = 0;i < gRx.size();++i) {
                                if (gRx[i].getPatientId() != pid || gRx[i].getDoctorId() != gUid)continue;
                                any = true; char ln[512];ln[0] = '\0';
                                sapp(ln, gRx[i].getDate(), 512);sapp(ln, " | ", 512);
                                sapp(ln, gRx[i].getMedicines(), 512);sapp(ln, "\n", 512);
                                lAdd(ln);
                            }
                            if (!any)lAdd("No records found.\n");
                            gScreen = Screen::VIEW_RECORDS;resetUI();buildSidebarDoctor();
                            btn[0].initPill(CONTENT_X, 710, 180, 40, "< Back", font, sf::Color(200, 198, 194), sf::Color(180, 178, 174), P_INK);bc = 1;
                        }
                    }
                }

             
                else if (gScreen == Screen::ADMIN_MENU) {
                    if (btn[9].hit(pt)) {
                        gUid = -1;gRole = Role::NONE;gScreen = Screen::ROLE_SELECT;resetUI();
                        btn[0].initPill(WIN_W / 2 - 160, 310, 310, 54, "  Patient  —  Outpatient Portal", font, P_VIOLET, P_VIOLET_L);
                        btn[1].initPill(WIN_W / 2 - 160, 378, 310, 54, "  Doctor   —  Clinical Dashboard", font, P_SKY, P_SKY_L);
                        btn[2].initPill(WIN_W / 2 - 160, 446, 310, 54, "  Admin    —  Control Panel", font, P_GOLD, P_GOLD_L);
                        btn[3].initPill(WIN_W / 2 - 160, 530, 310, 40, "Exit", font, sf::Color(200, 198, 194), sf::Color(180, 178, 174), P_INK);
                        bc = 4;
                    }
                    else if (btn[0].hit(pt)) { 
                        gScreen = Screen::ADM_ADD_DOC;resetUI();buildSidebarAdmin();
                        inp[0].init(CONTENT_X, 155, CONTENT_W / 2, "Name", font);
                        inp[1].init(CONTENT_X, 240, CONTENT_W / 2, "Specialization", font);
                        inp[2].init(CONTENT_X, 325, CONTENT_W / 2, "Contact  (11 digits)", font);
                        inp[3].init(CONTENT_X, 410, CONTENT_W / 2, "Password  (min 6 chars)", font, true);
                        inp[4].init(CONTENT_X, 495, CONTENT_W / 2, "Fee (PKR)", font);
                        inp[5].init(10, 760, 10, "", font); inp[5].buf[0] = '0';inp[5].buf[1] = '\0';inp[5].len = 1;
                        btn[0].initPill(CONTENT_X, 590, 200, 44, "Add Doctor", font, P_GOLD, P_GOLD_L, P_INK);
                        btn[1].initPill(CONTENT_X + 216, 590, 180, 44, "< Back", font, sf::Color(200, 198, 194), sf::Color(180, 178, 174), P_INK);
                        bc = 2;ic = 6;
                    }
                    else if (btn[1].hit(pt)) { 
                        buildAllDoctors(); gScreen = Screen::ADM_ADD_DOC;resetUI();buildSidebarAdmin();
                        inp[0].init(CONTENT_X, 540, 300, "Doctor ID to remove", font);
                        inp[5].init(10, 760, 10, "", font); inp[5].buf[0] = 'R';inp[5].buf[1] = '\0';inp[5].len = 1;
                        btn[0].initPill(CONTENT_X, 600, 180, 44, "Remove", font, P_ROSE, P_ROSE_L);
                        btn[1].initPill(CONTENT_X + 196, 600, 180, 44, "< Back", font, sf::Color(200, 198, 194), sf::Color(180, 178, 174), P_INK);
                        bc = 2;ic = 2;
                    }
                    else if (btn[2].hit(pt)) { buildAllPatients();gScreen = Screen::ADM_VIEW_ALL;resetUI();buildSidebarAdmin();btn[0].initPill(CONTENT_X, 710, 180, 40, "< Back", font, sf::Color(200, 198, 194), sf::Color(180, 178, 174), P_INK);bc = 1; }
                    else if (btn[3].hit(pt)) { buildAllDoctors(); gScreen = Screen::ADM_VIEW_ALL;resetUI();buildSidebarAdmin();btn[0].initPill(CONTENT_X, 710, 180, 40, "< Back", font, sf::Color(200, 198, 194), sf::Color(180, 178, 174), P_INK);bc = 1; }
                    else if (btn[4].hit(pt)) { buildAllAppts();   gScreen = Screen::ADM_VIEW_ALL;resetUI();buildSidebarAdmin();btn[0].initPill(CONTENT_X, 710, 180, 40, "< Back", font, sf::Color(200, 198, 194), sf::Color(180, 178, 174), P_INK);bc = 1; }
                    else if (btn[5].hit(pt)) { buildUnpaidBills();gScreen = Screen::ADM_VIEW_ALL;resetUI();buildSidebarAdmin();btn[0].initPill(CONTENT_X, 710, 180, 40, "< Back", font, sf::Color(200, 198, 194), sf::Color(180, 178, 174), P_INK);bc = 1; }
                    else if (btn[6].hit(pt)) { 
                        gScreen = Screen::ADM_DISCHARGE;resetUI();buildSidebarAdmin();
                        inp[0].init(CONTENT_X, 300, 300, "Patient ID", font);
                        btn[0].initPill(CONTENT_X, 370, 200, 44, "Discharge", font, P_ROSE, P_ROSE_L);
                        btn[1].initPill(CONTENT_X + 216, 370, 180, 44, "< Back", font, sf::Color(200, 198, 194), sf::Color(180, 178, 174), P_INK);
                        bc = 2;ic = 1;
                    }
                    else if (btn[7].hit(pt)) {
                        FileHandler::readSecurityLog(gList, 10240);
                        if (gList[0] == '\0')Person::strCopy(gList, "No security events logged.\n", 10240);
                        gScreen = Screen::ADM_SECURITY;resetUI();buildSidebarAdmin();
                        btn[0].initPill(CONTENT_X, 710, 180, 40, "< Back", font, sf::Color(200, 198, 194), sf::Color(180, 178, 174), P_INK);bc = 1;
                    }
                    else if (btn[8].hit(pt)) { buildDailyReport();gScreen = Screen::ADM_REPORT;resetUI();buildSidebarAdmin();btn[0].initPill(CONTENT_X, 710, 180, 40, "< Back", font, sf::Color(200, 198, 194), sf::Color(180, 178, 174), P_INK);bc = 1; }
                }

            
                else if (gScreen == Screen::ADM_ADD_DOC) {
                    if (btn[1].hit(pt)) { goBack(); }
                    else if (btn[0].hit(pt)) {
                        bool rm = (inp[5].buf[0] == 'R');
                        if (rm) {
                            int did = Validator::parseInt(inp[0].buf);
                            Doctor* doc = gD.findById(did);
                            if (!doc) { showMsg("Doctor not found.", Screen::ADMIN_MENU); }
                            else {
                                bool pend = false;
                                for (int i = 0;i < gA.size();++i) if (gA[i].getDoctorId() == did && gA[i].getStatus() == Appointment::PENDING) { pend = true;break; }
                                if (pend) showMsg("Cannot remove: pending appointments exist.", Screen::ADMIN_MENU);
                                else { gD.removeById(did); FileHandler::deleteDoctor(did); showMsg("Doctor removed.", Screen::ADMIN_MENU); }
                            }
                        }
                        else {
                            if (!Validator::isValidContact(inp[2].buf)) showMsg("Contact must be 11 digits.", Screen::ADM_ADD_DOC);
                            else if (!Validator::isValidPassword(inp[3].buf)) showMsg("Password min 6 characters.", Screen::ADM_ADD_DOC);
                            else if (!Validator::isPositiveFloat(inp[4].buf)) showMsg("Fee must be positive.", Screen::ADM_ADD_DOC);
                            else {
                                int nid = gD.maxId() + 1;
                                Doctor nd(nid, inp[0].buf, inp[1].buf, inp[2].buf, inp[3].buf, Validator::parseFloat(inp[4].buf));
                                gD.add(nd); FileHandler::appendDoctor(nd);
                                char msg[64] = "Doctor added.  ID: "; char ids[8]; itosBuf(nid, ids); sapp(msg, ids, 64);
                                showMsg(msg, Screen::ADMIN_MENU);
                            }
                        }
                    }
                }

                else if (gScreen == Screen::ADM_DISCHARGE) {
                    if (btn[1].hit(pt)) { goBack(); }
                    else if (btn[0].hit(pt)) {
                        int pid = Validator::parseInt(inp[0].buf);
                        Patient* pat = gP.findById(pid);
                        if (!pat) { showMsg("Patient not found.", Screen::ADMIN_MENU); }
                        else {
                            bool hu = false, hp = false;
                            for (int i = 0;i < gB.size();++i) if (gB[i].getPatientId() == pid && gB[i].getStatus() == Bill::UNPAID)hu = true;
                            for (int i = 0;i < gA.size();++i) if (gA[i].getPatientId() == pid && gA[i].getStatus() == Appointment::PENDING)hp = true;
                            if (hu)  showMsg("Cannot discharge: unpaid bills.", Screen::ADMIN_MENU);
                            else if (hp) showMsg("Cannot discharge: pending appointments.", Screen::ADMIN_MENU);
                            else { FileHandler::archivePatient(*pat, gA, gB, gRx); gP.removeById(pid); showMsg("Patient discharged and archived.", Screen::ADMIN_MENU); }
                        }
                    }
                }

                else if (gScreen == Screen::VIEW_APPTS || gScreen == Screen::VIEW_RECORDS ||
                    gScreen == Screen::VIEW_BILLS || gScreen == Screen::DOC_TODAY ||
                    gScreen == Screen::ADM_VIEW_ALL || gScreen == Screen::ADM_SECURITY ||
                    gScreen == Screen::ADM_REPORT)
                {
                    if (bc > 0 && btn[0].hit(pt)) goBack();
                }

            } 
        } 
        window.clear(P_BG);

        bool showSidebar = (gScreen != Screen::ROLE_SELECT &&
            gScreen != Screen::LOGIN &&
            gScreen != Screen::MSG_BOX);

        
        if (showSidebar) {
            sf::RectangleShape sidebar(sf::Vector2f(SIDEBAR_W, WIN_H));
            sidebar.setFillColor(P_SIDEBAR);
            window.draw(sidebar);

            sf::RectangleShape logoBar(sf::Vector2f(SIDEBAR_W, 68));
            logoBar.setFillColor(sf::Color(20, 22, 40));
            window.draw(logoBar);

            dTxt(window, "MediCore", 18, 18, 22, sf::Color(255, 255, 255));
            dTxt(window, "HMS", 18, 42, 11, sf::Color(99, 91, 255));

            const char* roleLabel = (gRole == Role::PATIENT) ? "PATIENT PORTAL" :
                (gRole == Role::DOCTOR) ? "CLINICAL DASHBOARD" : "ADMIN PANEL";
            sf::Color roleCol = (gRole == Role::PATIENT) ? P_VIOLET : (gRole == Role::DOCTOR) ? P_SKY : P_GOLD;
            dTxt(window, roleLabel, 18, 72, 9, roleCol);

            sf::RectangleShape sep(sf::Vector2f(SIDEBAR_W - 36, 1));
            sep.setPosition(18, 100); sep.setFillColor(sf::Color(60, 63, 90));
            window.draw(sep);

            for (int i = 0;i < sideNavCount;++i) sideNav[i].draw(window);

            if (gUid >= 0) {
                sf::RectangleShape bottomBar(sf::Vector2f(SIDEBAR_W, 60));
                bottomBar.setPosition(0, WIN_H - 60); bottomBar.setFillColor(sf::Color(20, 22, 40));
                window.draw(bottomBar);
                if (gRole == Role::PATIENT) {
                    Patient* p = gP.findById(gUid);
                    if (p) {
                        dTxt(window, p->getName(), 12, WIN_H - 50, 12, sf::Color(210, 212, 230));
                        char bal[32] = "PKR "; char b[16]; ftosBuf(p->getBalance(), b); sapp(bal, b, 32);
                        dTxt(window, bal, 12, WIN_H - 32, 10, roleCol);
                    }
                }
                else if (gRole == Role::DOCTOR) {
                    Doctor* d = gD.findById(gUid);
                    if (d) {
                        char nm[128] = "Dr. "; sapp(nm, d->getName(), 128);
                        dTxt(window, nm, 12, WIN_H - 50, 12, sf::Color(210, 212, 230));
                        dTxt(window, d->getSpecialization(), 12, WIN_H - 32, 10, roleCol);
                    }
                }
                else {
                    dTxt(window, "Administrator", 12, WIN_H - 50, 12, sf::Color(210, 212, 230));
                    dTxt(window, "Full access", 12, WIN_H - 32, 10, roleCol);
                }
            }
        }

       
        switch (gScreen) {

        case Screen::ROLE_SELECT: {
            // Centered welcome card
            dCard(window, WIN_W / 2 - 200, 160, 400, 430);
            // Violet top strip
            sf::RectangleShape strip(sf::Vector2f(400, 5));
            strip.setPosition(WIN_W / 2 - 200, 160); strip.setFillColor(P_VIOLET);
            window.draw(strip);
            sf::Text hd; hd.setFont(font); hd.setString("MediCore");
            hd.setCharacterSize(32); hd.setFillColor(P_INK);
            sf::FloatRect lb = hd.getLocalBounds();
            hd.setPosition(WIN_W / 2 - lb.width / 2, 186); window.draw(hd);
            dTxt(window, "Hospital Management System", WIN_W / 2 - 132, 226, 12, P_SUB);
            dRule(window, 266); // not sidebar, use manual rule
            sf::RectangleShape hr(sf::Vector2f(360, 1));
            hr.setPosition(WIN_W / 2 - 180, 274); hr.setFillColor(P_RULE);
            window.draw(hr);
            dTxt(window, "Select your role to continue", WIN_W / 2 - 118, 284, 12, P_SUB);
            break;
        }

        case Screen::LOGIN: {
            dCard(window, WIN_W / 2 - 240, 140, 480, 420);
            sf::RectangleShape strip(sf::Vector2f(480, 4));
            sf::Color sc = (gRole == Role::PATIENT) ? P_VIOLET : (gRole == Role::DOCTOR) ? P_SKY : P_GOLD;
            strip.setPosition(WIN_W / 2 - 240, 140); strip.setFillColor(sc);
            window.draw(strip);
            const char* rt = (gRole == Role::PATIENT) ? "Welcome Back" : "Good to see you";
            sf::Text hd; hd.setFont(font); hd.setString(rt);
            hd.setCharacterSize(24); hd.setFillColor(P_INK);
            sf::FloatRect lb = hd.getLocalBounds();
            hd.setPosition(WIN_W / 2 - lb.width / 2, 162); window.draw(hd);
            const char* sub = (gRole == Role::PATIENT) ? "Patient Portal" :
                (gRole == Role::DOCTOR) ? "Clinical Dashboard" : "Admin Control Panel";
            sf::Text sh; sh.setFont(font); sh.setString(sub);
            sh.setCharacterSize(12); sh.setFillColor(sc);
            sf::FloatRect sl = sh.getLocalBounds();
            sh.setPosition(WIN_W / 2 - sl.width / 2, 194); window.draw(sh);
            for (int i = 0;i < ic;++i) inp[i].draw(window, 400);
            break;
        }

        case Screen::PATIENT_MENU:
            dHeading(window, "Patient Dashboard", 80);
            dRule(window, 118);
            dTxt(window, "What would you like to do?", CONTENT_X, 132, 12, P_SUB);
            dBadge(window, "Patient", CONTENT_X, 142, sf::Color(235, 234, 255), P_VIOLET);
            break;

        case Screen::DOCTOR_MENU:
            dHeading(window, "Clinical Dashboard", 80);
            dRule(window, 118);
            dTxt(window, "Select an action from below or the sidebar", CONTENT_X, 132, 12, P_SUB);
            break;

        case Screen::ADMIN_MENU:
            dHeading(window, "Admin Control Panel", 80);
            dRule(window, 118);
            dTxt(window, "Full system access", CONTENT_X, 132, 12, P_SUB);
            break;

        case Screen::MSG_BOX: {
            // Full-screen overlay
            sf::RectangleShape ov(sf::Vector2f(WIN_W, WIN_H));
            ov.setFillColor(P_OVERLAY); window.draw(ov);
            dCard(window, WIN_W / 2 - 280, 280, 560, 240);
            sf::RectangleShape strip(sf::Vector2f(560, 4));
            strip.setPosition(WIN_W / 2 - 280, 280); strip.setFillColor(P_VIOLET);
            window.draw(strip);
            dTxt(window, "Notice", WIN_W / 2 - 260, 300, 13, P_SUB);
            // Wrap long messages across lines manually
            const char* msg = gMsg;
            float my = 330; int charsPerLine = 52;
            while (*msg) {
                char line[64]; int li = 0;
                while (*msg && li < charsPerLine) { line[li++] = *msg++; }
                // don't break mid-word
                if (*msg && *(msg - 1) != ' ') {
                    while (li > 0 && line[li - 1] != ' ') { --li;--msg; }
                }
                line[li] = '\0';
                dTxt(window, line, WIN_W / 2 - 260, my, 15, P_INK);
                my += 24;
            }
            break;
        }

        case Screen::BOOK_APPT:
            dHeading(window, "Book an Appointment", 80);
            dRule(window, 118);
            dTxt(window, "Slots:  09:00  10:00  11:00  12:00  13:00  14:00  15:00  16:00",
                CONTENT_X, 535, 11, P_SUB);
            break;

        case Screen::CANCEL_APPT:
            dHeading(window, "Cancel Appointment", 80);
            dRule(window, 118);
            dScrollList(window, CONTENT_X, 130, CONTENT_W, 380);
            dTxt(window, "Scroll: mouse wheel", CONTENT_X, 518, 10, P_SUB);
            break;

        case Screen::VIEW_APPTS:
            dHeading(window, "My Appointments", 80);
            dRule(window, 118);
            dScrollList(window, CONTENT_X, 130, CONTENT_W, 548);
            dTxt(window, "Scroll: mouse wheel", CONTENT_X, 686, 10, P_SUB);
            break;

        case Screen::VIEW_RECORDS:
            dHeading(window, "Medical Records", 80);
            dRule(window, 118);
            dScrollList(window, CONTENT_X, 130, CONTENT_W, 548);
            dTxt(window, "Scroll: mouse wheel", CONTENT_X, 686, 10, P_SUB);
            break;

        case Screen::VIEW_BILLS:
            dHeading(window, "My Bills", 80);
            dRule(window, 118);
            dScrollList(window, CONTENT_X, 130, CONTENT_W, 548);
            dTxt(window, "Scroll: mouse wheel", CONTENT_X, 686, 10, P_SUB);
            break;

        case Screen::PAY_BILL:
            dHeading(window, "Pay a Bill", 80);
            dRule(window, 118);
            dScrollList(window, CONTENT_X, 130, CONTENT_W, 380);
            dTxt(window, "Scroll: mouse wheel", CONTENT_X, 518, 10, P_SUB);
            break;

        case Screen::TOPUP:
            dHeading(window, "Top Up Balance", 80);
            dRule(window, 118);
            dCard(window, CONTENT_X, 240, 420, 200);
            break;

        case Screen::DOC_TODAY:
            dHeading(window, "Today's Appointments", 80);
            dRule(window, 118);
            dScrollList(window, CONTENT_X, 130, CONTENT_W, 548);
            dTxt(window, "Scroll: mouse wheel", CONTENT_X, 686, 10, P_SUB);
            break;

        case Screen::DOC_MARK: {
            bool isNS = (ic >= 2 && inp[1].buf[0] == '1');
            dHeading(window, isNS ? "Mark Appointment No-Show" : "Mark Appointment Complete", 80);
            dRule(window, 118);
            dCard(window, CONTENT_X, 220, 440, 200);
            dTxt(window, isNS ? "Enter appointment ID and confirm no-show" :
                "Enter appointment ID and confirm completion",
                CONTENT_X + 12, 236, 12, P_SUB);
            break;
        }

        case Screen::DOC_PRESCRIPTION:
            dHeading(window, "Write Prescription", 80);
            dRule(window, 118);
            break;

        case Screen::DOC_HISTORY:
            dHeading(window, "Patient Medical History", 80);
            dRule(window, 118);
            dCard(window, CONTENT_X, 220, 400, 180);
            dTxt(window, "Enter the patient ID to view their prescription history", CONTENT_X + 12, 236, 12, P_SUB);
            break;

        case Screen::ADM_ADD_DOC:
            dHeading(window, (ic >= 6 && inp[5].buf[0] == 'R') ? "Remove Doctor" : "Add New Doctor", 80);
            dRule(window, 118);
            if (ic >= 2 && inp[5].buf[0] == 'R')
                dScrollList(window, CONTENT_X, 130, CONTENT_W, 380);
            break;

        case Screen::ADM_VIEW_ALL:
            dHeading(window, "Directory", 80);
            dRule(window, 118);
            dScrollList(window, CONTENT_X, 130, CONTENT_W, 548);
            dTxt(window, "Scroll: mouse wheel", CONTENT_X, 686, 10, P_SUB);
            break;

        case Screen::ADM_DISCHARGE:
            dHeading(window, "Discharge Patient", 80);
            dRule(window, 118);
            dCard(window, CONTENT_X, 220, 420, 200);
            dTxt(window, "Patient must have no unpaid bills or pending appointments", CONTENT_X + 12, 236, 12, P_SUB);
            break;

        case Screen::ADM_SECURITY:
            dHeading(window, "Security Log", 80);
            dRule(window, 118);
            dScrollList(window, CONTENT_X, 130, CONTENT_W, 548);
            dTxt(window, "Scroll: mouse wheel", CONTENT_X, 686, 10, P_SUB);
            break;

        case Screen::ADM_REPORT:
            dHeading(window, "Daily Report", 80);
            dRule(window, 118);
            dScrollList(window, CONTENT_X, 130, CONTENT_W, 548);
            dTxt(window, "Scroll: mouse wheel", CONTENT_X, 686, 10, P_SUB);
            break;
        }

        if (gScreen != Screen::LOGIN)
            for (int i = 0;i < ic;++i) inp[i].draw(window, CONTENT_W / 2 - 20);

    
        for (int i = 0;i < bc;++i) btn[i].draw(window);

        window.display();
    }

    return 0;
}
