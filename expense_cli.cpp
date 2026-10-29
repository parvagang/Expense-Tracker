// =============================================================================
//  EXPENSE TRACKER - Basic OOP Prototype (single file)
//  Concepts used:
//    - Abstraction   : Transaction and Expense are abstract classes
//    - Inheritance   : Expense, Income and Loan inherit from Transaction;
//                      Food, Travel, Bills, Shopping inherit from Expense
//    - Polymorphism  : one list of Transaction* holds all entries, and
//                      show(), getType(), getCategory() and getInterest()
//                      run the right version for each entry
//    - Constructors  : every class sets up its own data
//  Data is kept in memory only (it is lost when the program closes).
// =============================================================================

#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <string>
using namespace std;

const int MAX_ITEMS = 200;

// -----------------------------------------------------------------------------
// HELPER FUNCTIONS
// -----------------------------------------------------------------------------
string formatTime(time_t t) {
    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&t));
    return string(buf);
}

int daysInMonth(int year, int month) {
    int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == 2 && ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0))
        return 29;
    return days[month - 1];
}

string monthName(int month) {
    string names[] = {"January", "February", "March", "April", "May", "June",
                      "July", "August", "September", "October", "November", "December"};
    return names[month - 1];
}

// -----------------------------------------------------------------------------
// ABSTRACT BASE CLASS: every entry (expense, income, loan) is a Transaction
// -----------------------------------------------------------------------------
class Transaction {
protected:
    double amount;
    string description;
    time_t when;     // exact time the entry was added
    int year;
    int month;

public:
    // Constructor: stores the amount and the current date/time automatically
    Transaction(double amt, string desc) {
        amount = amt;
        description = desc;
        when = time(NULL);
        tm* lt = localtime(&when);
        year = lt->tm_year + 1900;
        month = lt->tm_mon + 1;
    }

    virtual ~Transaction() {}

    double getAmount() const { return amount; }
    string getDescription() const { return description; }
    int getYear() const { return year; }
    int getMonth() const { return month; }
    string getTime() const { return formatTime(when); }

    // PURE VIRTUAL: each child type must say what it is
    virtual string getType() const = 0;

    // Default values; child classes replace them when needed
    virtual string getCategory() const { return ""; }
    virtual double getInterest() const { return 0; }

    virtual void show() const {
        cout << "  [" << getTime() << "] " << getType()
             << " | " << description << " | " << amount << endl;
    }
};

// -----------------------------------------------------------------------------
// EXPENSE (abstract, because it has a pure virtual getCategory)
// -----------------------------------------------------------------------------
class Expense : public Transaction {
public:
    Expense(double amt, string desc) : Transaction(amt, desc) {}

    string getType() const { return "Expense"; }

    virtual string getCategory() const = 0;   // abstract

    void show() const {
        cout << "  [" << getTime() << "] Expense (" << getCategory()
             << ") | " << description << " | " << amount << endl;
    }
};

class FoodExpense : public Expense {
public:
    FoodExpense(double amt, string desc) : Expense(amt, desc) {}
    string getCategory() const { return "Food"; }
};

class TravelExpense : public Expense {
public:
    TravelExpense(double amt, string desc) : Expense(amt, desc) {}
    string getCategory() const { return "Travel"; }
};

class BillExpense : public Expense {
public:
    BillExpense(double amt, string desc) : Expense(amt, desc) {}
    string getCategory() const { return "Bills"; }
};

class ShoppingExpense : public Expense {
public:
    ShoppingExpense(double amt, string desc) : Expense(amt, desc) {}
    string getCategory() const { return "Shopping"; }
};

// -----------------------------------------------------------------------------
// INCOME
// -----------------------------------------------------------------------------
class Income : public Transaction {
public:
    Income(double amt, string desc) : Transaction(amt, desc) {}
    string getType() const { return "Income"; }
};

// -----------------------------------------------------------------------------
// LOAN (money borrowed, with monthly interest)
// -----------------------------------------------------------------------------
class Loan : public Transaction {
private:
    double monthlyRate;   // percent per month, e.g. 2 means 2%

public:
    Loan(double amt, string desc, double rate) : Transaction(amt, desc) {
        monthlyRate = rate;
    }

    string getType() const { return "Loan"; }

    double getInterest() const { return amount * monthlyRate / 100.0; }

    void show() const {
        cout << "  [" << getTime() << "] Loan | " << description << " | " << amount
             << " | " << monthlyRate << "% per month (interest "
             << getInterest() << " per month)" << endl;
    }
};
// TRACKER: holds all entries and calculates totals and the monthly status
class Tracker {
private:
    Transaction* items[MAX_ITEMS];
    int count;

public:
    Tracker() {
        count = 0;
    }

    ~Tracker() {
        for (int i = 0; i < count; i++) {
            delete items[i];
        }
    }

    int size() const { return count; }

    bool add(Transaction* t) {
        if (count >= MAX_ITEMS) return false;
        items[count] = t;
        count++;
        return true;
    }

    void showAll() const {
        if (count == 0) {
            cout << "  No entries yet.\n";
            return;
        }
        for (int i = 0; i < count; i++) {
            cout << "  " << (i + 1) << ".";
            items[i]->show();
        }
    }

    bool removeAt(int index) {
        if (index < 0 || index >= count) return false;
        delete items[index];
        for (int i = index; i < count - 1; i++) {
            items[i] = items[i + 1];
        }
        count--;
        return true;
    }

    double totalSpent() const {
        double sum = 0;
        for (int i = 0; i < count; i++) {
            if (items[i]->getType() == "Expense") sum += items[i]->getAmount();
        }
        return sum;
    }

    void showCategoryTotals() const {
        string categories[] = {"Food", "Travel", "Bills", "Shopping"};
        for (int c = 0; c < 4; c++) {
            double sum = 0;
            for (int i = 0; i < count; i++) {
                if (items[i]->getType() == "Expense" &&
                    items[i]->getCategory() == categories[c]) {
                    sum += items[i]->getAmount();
                }
            }
            cout << "  " << categories[c] << ": " << sum << endl;
        }
    }

    // Monthly status: uses today's date to decide which entries belong to this month
    void showStatus() const {
        time_t now = time(NULL);
        tm* lt = localtime(&now);
        int year = lt->tm_year + 1900;
        int month = lt->tm_mon + 1;
        int today = lt->tm_mday;
        int daysLeft = daysInMonth(year, month) - today + 1;

        double income = 0, spent = 0, bills = 0, loans = 0, interest = 0;

        for (int i = 0; i < count; i++) {
            Transaction* t = items[i];
            string type = t->getType();
            bool thisMonth = (t->getYear() == year && t->getMonth() == month);

            if (type == "Loan") {
                loans += t->getAmount();          // loans are counted until repaid
                interest += t->getInterest();
            }
            if (thisMonth && type == "Income") {
                income += t->getAmount();
            }
            if (thisMonth && type == "Expense") {
                spent += t->getAmount();
                if (t->getCategory() == "Bills") bills += t->getAmount();
            }
        }

        double left = income - spent;

        cout << "\n----------- MONTHLY STATUS -----------\n";
        cout << "  Current time       : " << formatTime(now) << "\n";
        cout << "  Month              : " << monthName(month) << " " << year << "\n";
        cout << "  Days remaining     : " << daysLeft << " (including today)\n";
        cout << "  ------------------------------------\n";
        cout << "  Income this month  : " << income << "\n";
        cout << "  Spent this month   : " << spent << "\n";
        cout << "    of which bills   : " << bills << "\n";
        cout << "  Money left         : " << left << "\n";
        cout << "  ------------------------------------\n";
        cout << "  Total loans        : " << loans << "\n";
        cout << "  Interest per month : " << interest << "\n";
        cout << "  Left after interest: " << left - interest << "\n";
        cout << "--------------------------------------\n";
    }
};

// -----------------------------------------------------------------------------
// INPUT HELPERS
// -----------------------------------------------------------------------------
int readInt(const string& prompt) {
    int value = 0;
    cout << prompt;
    while (!(cin >> value)) {
        if (cin.eof()) {
            cout << "\nGoodbye!\n";
            exit(0);
        }
        cin.clear();
        cin.ignore(1000, '\n');
        cout << "  Please enter a whole number: ";
    }
    cin.ignore(1000, '\n');
    return value;
}

// Amount that must be greater than zero
double readAmount(const string& prompt) {
    double value = 0;
    cout << prompt;
    while (!(cin >> value) || value <= 0) {
        if (cin.eof()) {
            cout << "\nGoodbye!\n";
            exit(0);
        }
        cin.clear();
        cin.ignore(1000, '\n');
        cout << "  Please enter a positive amount: ";
    }
    cin.ignore(1000, '\n');
    return value;
}

// Interest rate that may be zero
double readRate(const string& prompt) {
    double value = 0;
    cout << prompt;
    while (!(cin >> value) || value < 0) {
        if (cin.eof()) {
            cout << "\nGoodbye!\n";
            exit(0);
        }
        cin.clear();
        cin.ignore(1000, '\n');
        cout << "  Please enter 0 or a positive rate: ";
    }
    cin.ignore(1000, '\n');
    return value;
}

string readText(const string& prompt) {
    string text;
    cout << prompt;
    getline(cin, text);
    return text;
}

// -----------------------------------------------------------------------------
// MENU ACTIONS
// -----------------------------------------------------------------------------
void addExpense(Tracker& tracker) {
    cout << "\n--- Add Expense ---\n";
    cout << "  1. Food\n  2. Travel\n  3. Bills\n  4. Shopping\n";
    int choice = readInt("Choose category: ");

    Transaction* t = NULL;
    if (choice >= 1 && choice <= 4) {
        double amount = readAmount("Amount: ");
        string desc = readText("Description: ");
        if (choice == 1)      t = new FoodExpense(amount, desc);
        else if (choice == 2) t = new TravelExpense(amount, desc);
        else if (choice == 3) t = new BillExpense(amount, desc);
        else                  t = new ShoppingExpense(amount, desc);
    } else {
        cout << "  Invalid category.\n";
        return;
    }

    if (tracker.add(t)) {
        cout << "  Expense added at " << t->getTime() << ".\n";
    } else {
        cout << "  The list is full.\n";
        delete t;
    }
}

void addIncome(Tracker& tracker) {
    cout << "\n--- Add Income ---\n";
    double amount = readAmount("Amount: ");
    string desc = readText("Source (e.g. salary): ");
    Transaction* t = new Income(amount, desc);
    if (tracker.add(t)) {
        cout << "  Income added at " << t->getTime() << ".\n";
    } else {
        cout << "  The list is full.\n";
        delete t;
    }
}

void addLoan(Tracker& tracker) {
    cout << "\n--- Add Loan ---\n";
    double amount = readAmount("Loan amount: ");
    string desc = readText("Loan from (e.g. bank): ");
    double rate = readRate("Interest rate per month (%): ");
    Transaction* t = new Loan(amount, desc, rate);
    if (tracker.add(t)) {
        cout << "  Loan added at " << t->getTime() << ".\n";
    } else {
        cout << "  The list is full.\n";
        delete t;
    }
}

void deleteEntry(Tracker& tracker) {
    cout << "\n--- Delete Entry ---\n";
    tracker.showAll();
    if (tracker.size() == 0) return;

    int number = readInt("Enter the number to delete (0 to cancel): ");
    if (number == 0) {
        cout << "  Cancelled.\n";
    } else if (tracker.removeAt(number - 1)) {
        cout << "  Entry deleted.\n";
    } else {
        cout << "  Invalid number.\n";
    }
}

void showMenu() {
    cout << "\n===== EXPENSE TRACKER =====\n"
         << "1. Add expense\n"
         << "2. Add income\n"
         << "3. Add loan\n"
         << "4. View all entries\n"
         << "5. Show total spent\n"
         << "6. Show totals by category\n"
         << "7. Monthly status\n"
         << "8. Delete entry\n"
         << "0. Exit\n"
         << "===========================\n";
}

// -----------------------------------------------------------------------------
// MAIN
// -----------------------------------------------------------------------------
int main() {
    cout << fixed << setprecision(2);   // money always shows 2 decimals

    Tracker tracker;
    int choice = -1;

    while (choice != 0) {
        showMenu();
        choice = readInt("Choose an option: ");

        if (choice == 1)      addExpense(tracker);
        else if (choice == 2) addIncome(tracker);
        else if (choice == 3) addLoan(tracker);
        else if (choice == 4) {
            cout << "\n--- All Entries ---\n";
            tracker.showAll();
        }
        else if (choice == 5) cout << "\nTotal spent: " << tracker.totalSpent() << endl;
        else if (choice == 6) {
            cout << "\n--- Totals by Category ---\n";
            tracker.showCategoryTotals();
        }
        else if (choice == 7) tracker.showStatus();
        else if (choice == 8) deleteEntry(tracker);
        else if (choice == 0) cout << "Goodbye!\n";
        else cout << "Invalid option.\n";
    }
    return 0;
}