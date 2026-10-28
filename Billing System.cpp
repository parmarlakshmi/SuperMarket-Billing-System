#include <iostream>
#include <fstream>
#include <string>
#include <limits>
#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <ctime>

using namespace std;

// Reads a value from the user, re-prompting on invalid input.
// Exits cleanly if input ends (e.g. Ctrl+Z / end of piped input).
template <typename T>
T input()
{
    T value;
    while (!(cin >> value))
    {
        if (cin.eof())
        {
            cout << "\n\nInput ended. Exiting...\n";
            exit(0);
        }
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid input, please try again: ";
    }
    // Discard anything else typed on the same line (e.g. the "." in "1.")
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    return value;
}

// Reads a number between min and max, re-prompting if out of range.
float inputRange(float min, float max)
{
    float value = input<float>();
    while (value < min || value > max)
    {
        cout << "Please enter a value between " << min << " and " << max << ": ";
        value = input<float>();
    }
    return value;
}

string toLower(string s)
{
    for (size_t i = 0; i < s.size(); i++)
    {
        s[i] = tolower((unsigned char)s[i]);
    }
    return s;
}

bool isNumber(const string &s)
{
    if (s.empty())
        return false;
    for (size_t i = 0; i < s.size(); i++)
    {
        if (!isdigit((unsigned char)s[i]))
            return false;
    }
    return true;
}

class shopping
{
private:
    int pcode;
    float price;
    float dis;
    string pname;

public:
    void menu();
    void administrator();
    void buyer();
    void add();
    void edit();
    void rem();
    bool list();
    bool find(const string &key);
    void receipt();
};

void shopping ::menu()
{
m:
    int choice;
    string email;
    string password;

    cout << "____________________________________\n";
    cout << "                                    \n";
    cout << "||     SUPERMARKET MAIN MENU      ||\n";
    cout << "                                    \n";
    cout << "____________________________________\n";
    cout << "                                    \n";
    cout << "|         1. ADMINISTRATOR         |\n";
    cout << "|                                  |\n";
    cout << "|         2. BUYER                 |\n";
    cout << "|                                  |\n";
    cout << "|         3. EXIT                  |\n";
    cout << "|                                  |\n";
    cout << "____________________________________\n";

    cout << "\nSelect Option: ";
    choice = input<int>();

    switch (choice)
    {
    case 1:
        cout << "\n\nPlease Login ! \n\n";
        cout << "\nEnter Email: ";
        email = input<string>();
        cout << "Enter Password: ";
        password = input<string>();
        if (email == "lakshmidevi@gmail.com" && password == "Ld@2026")
        {
            cout << "\nLogin Successful ! Welcome, Lakshmi Devi.\n\n";
            administrator();
        }
        else
        {
            cout << "Invalid Email/Password ! \n";
        }
        break;

    case 2:
        buyer();
        break;

    case 3:
        cout << "\nThank you for visiting ! \n";
        exit(0);

    default:
        cout << "Please select from the given Options ! \n";
    }
    goto m;
}

void shopping ::administrator()
{
m:
    int choice;
    cout << "________________________________\n";
    cout << "\n||     ADMINISTRATOR MENU     ||\n";
    cout << "________________________________\n\n";
    cout << "|      1. VIEW PRODUCTS        |\n";
    cout << "|                              |\n";
    cout << "|      2. ADD PRODUCT          |\n";
    cout << "|                              |\n";
    cout << "|      3. MODIFY PRODUCT       |\n";
    cout << "|                              |\n";
    cout << "|      4. DELETE PRODUCT       |\n";
    cout << "|                              |\n";
    cout << "|      5. BACK TO MENU         |\n";
    cout << "________________________________\n";

    cout << "\n\nPlease Enter your Choice: ";
    choice = input<int>();

    switch (choice)
    {
    case 1:
        cout << "\n\n||       ALL PRODUCTS       ||\n";
        list();
        cout << "\n";
        break;

    case 2:
        add();
        break;

    case 3:
        edit();
        break;

    case 4:
        rem();
        break;

    case 5:
        return;

    default:
        cout << "\nInvalid Choice ! \n";
    }
    goto m;
}

void shopping ::buyer()
{
m:
    int choice;
    cout << endl << endl;
    cout << "________________________________\n";
    cout << "\n||        BUYER MENU          ||\n";
    cout << "________________________________\n\n";
    cout << "|       1. BUY PRODUCT         |\n";
    cout << "|                              |\n";
    cout << "|       2. BACK TO MENU        |\n";
    cout << "________________________________\n";

    cout << "\n\nPlease Enter your Choice: ";
    choice = input<int>();

    switch (choice)
    {
    case 1:
        receipt();
        break;

    case 2:
        return;

    default:
        cout << "\nInvalid Choice ! \n";
    }

    goto m;
}

void shopping ::add()
{
    fstream data;
    int c;
    int token = 0;
    float p;
    float d;
    string n;

    cout << "\n\n||    ADD NEW PRODUCT    ||\n";
    cout << "\nExisting Products:\n";
    list();
    cout << "\nProduct Code of the Product: ";
    pcode = input<int>();

    data.open("database.txt", ios::in);
    if (data)
    {
        while (data >> c >> n >> p >> d)
        {
            if (c == pcode)
            {
                token++;
            }
        }
        data.close();
    }

    if (token > 0)
    {
        cout << "\nProduct Code already exists ! Please use a different code.\n\n";
        return;
    }

    cout << "Name of the Product (one word, e.g. Basmati_Rice): ";
    pname = input<string>();
    cout << "Price of the Product (Rs.): ";
    price = inputRange(0.01, 1000000);
    cout << "Discount on Product (0-100 %): ";
    dis = inputRange(0, 100);

    data.open("database.txt", ios::app | ios::out);
    data << " " << pcode << " " << pname << " " << price << " " << dis << " \n";
    data.close();

    cout << "\nRecord Inserted ! \n\n";
}

void shopping ::edit()
{
    fstream data, data1;
    int pkey;
    int token = 0;
    int c;
    float p;
    float d;
    string n;

    cout << "\n\n||    MODIFY THE PRODUCT    ||\n";
    if (!list())
    {
        return;
    }
    cout << "\nEnter Product Code to Modify: ";
    pkey = input<int>();

    data.open("database.txt", ios::in);
    if (!data)
    {
        cout << "\n\n File doesn't Exists ! \n";
    }
    else
    {

        data1.open("database1.txt", ios::out | ios::trunc);

        while (data >> pcode >> pname >> price >> dis)
        {
            if (pkey == pcode)
            {
                cout << "\nCurrent Details -> Code: " << pcode << ", Name: " << pname
                     << ", Price: " << price << ", Discount: " << dis << "%\n";
                cout << "\nProduct new Code: ";
                c = input<int>();
                cout << "Name of the Product: ";
                n = input<string>();
                cout << "Price of the Product (Rs.): ";
                p = inputRange(0.01, 1000000);
                cout << "Discount (0-100 %): ";
                d = inputRange(0, 100);
                data1 << " " << c << " " << n << " " << p << " " << d << " \n";
                cout << "\nRecord Edited ! \n";
                token++;
            }
            else
            {
                data1 << " " << pcode << " " << pname << " " << price << " " << dis << " \n";
            }
        }
        data.close();
        data1.close();

        remove("database.txt");
        rename("database1.txt", "database.txt");

        if (token == 0)
        {
            cout << "\n\nRecord Not Found ! \n";
        }
    }
}

void shopping::rem()
{
    fstream data, data1;
    int pkey;
    int token = 0;
    cout << "\n\n||    DELETE PRODUCT    ||\n";
    if (!list())
    {
        return;
    }
    cout << "\nEnter Product Code to Delete: ";
    pkey = input<int>();

    // Show the product and ask for confirmation before deleting
    if (!find(to_string(pkey)))
    {
        cout << "\n\nRecord not Found ! \n";
        return;
    }
    cout << "\nDelete " << pname << " (Code " << pcode << ")? Press 'y' to confirm: ";
    char confirm = input<char>();
    if (confirm != 'y' && confirm != 'Y')
    {
        cout << "\nDelete Cancelled ! \n";
        return;
    }

    data.open("database.txt", ios::in);
    if (!data)
    {
        cout << "\n\nFile doesn't Exists ! \n";
    }

    else
    {
        data1.open("database1.txt", ios::out | ios::trunc);
        while (data >> pcode >> pname >> price >> dis)
        {
            if (pcode == pkey)
            {
                cout << "\n\nProduct Deleted Successfully ! \n";
                token++;
            }
            else
            {
                data1 << " " << pcode << " " << pname << " " << price << " " << dis << " \n";
            }
        }
        data.close();
        data1.close();
        remove("database.txt");
        rename("database1.txt", "database.txt");

        if (token == 0)
        {
            cout << "\n\nRecord not Found ! \n";
        }
    }
}

// Prints all products. Returns false if there are none.
bool shopping::list()
{
    fstream data;
    int count = 0;
    data.open("database.txt", ios::in);
    cout << "|________________________________| \n";
    cout << "|P.No   Name    Price  Discount  | \n";
    cout << "|________________________________| \n";
    while (data >> pcode >> pname >> price >> dis)
    {
        cout << "  " << pcode << "\t" << pname << "\t" << price << "\t" << dis << "%\n";
        count++;
    }
    data.close();

    if (count == 0)
    {
        cout << "  No products available.\n";
    }
    return count > 0;
}

// Looks up a product by code or name (case-insensitive) and loads it
// into the member fields. Returns false if not found.
bool shopping::find(const string &key)
{
    fstream data;
    data.open("database.txt", ios::in);
    while (data >> pcode >> pname >> price >> dis)
    {
        if ((isNumber(key) && pcode == stoi(key)) || toLower(pname) == toLower(key))
        {
            data.close();
            return true;
        }
    }
    data.close();
    return false;
}

void shopping::receipt()
{
    const int MAX_ITEMS = 100;
    int arrc[MAX_ITEMS];
    int arrq[MAX_ITEMS];
    char choice;
    int c = 0;
    float amount = 0;
    float discounted = 0;
    float total = 0;
    float saved = 0;

   cout << "\n\n||         PRODUCT LIST         ||\n";
    if (!list())
    {
        return;
    }

    cout << "\n\nPlease Place the Order !\n";
    do
    {
        cout << "\n\nEnter Product Code or Name: ";
        string key = input<string>();

        if (!find(key))
        {
            cout << "\n\nProduct not found, Please Try Again ! \n";
        }
        else
        {
            bool duplicate = false;
            for (int i = 0; i < c; i++)
            {
                if (pcode == arrc[i])
                {
                    duplicate = true;
                }
            }

            if (duplicate)
            {
                cout << "\n\nProduct already in your order, Please Try Again ! \n";
            }
            else
            {
                cout << "Selected: " << pname << " (Rs. " << price << ")\n";
                cout << "\nEnter the Product Quantity: ";
                int qty = input<int>();
                while (qty <= 0)
                {
                    cout << "Quantity must be greater than 0: ";
                    qty = input<int>();
                }
                arrc[c] = pcode;
                arrq[c] = qty;
                c++;
            }
        }

        if (c == MAX_ITEMS)
        {
            cout << "\n\nMaximum number of items reached ! \n";
            break;
        }

        cout << "\n\nDo you want to Buy another Product? If Yes, then Press 'y' else 'n' ! \n";
        choice = input<char>();
    } while (choice == 'y' || choice == 'Y');

    if (c == 0)
    {
        cout << "\n\nNo products ordered. \n";
        return;
    }

    // Build the receipt once and print it to screen and to bills.txt
    time_t now = time(0);
    char date[32];
    strftime(date, sizeof(date), "%d-%m-%Y %H:%M:%S", localtime(&now));

    string bill;
    bill += "\n\n||            RECEIPT           ||\n";
    bill += "Date: " + string(date) + "\n";
    bill += "\nP.No\tName\tQuantity\tPrice\tAmount \t Amount with Discount";

    for (int i = 0; i < c; i++)
    {
        if (find(to_string(arrc[i])))
        {
            amount = price * arrq[i];
            discounted = amount - (amount * dis / 100);
            total = total + discounted;
            saved = saved + (amount - discounted);

            char line[200];
            snprintf(line, sizeof(line), "\n%d\t%s  \t %d \t\t %g \t %g  \t  %g",
                     pcode, pname.c_str(), arrq[i], price, amount, discounted);
            bill += line;
        }
    }

    char totals[100];
    snprintf(totals, sizeof(totals), "\n\nYou Saved: Rs. %g\nTotal Amount: Rs. %g\n", saved, total);
    bill += totals;

    cout << bill;

    fstream bills;
    bills.open("bills.txt", ios::app | ios::out);
    bills << bill << "________________________________\n";
    bills.close();
    cout << "\n(Receipt saved to bills.txt)\n";
}
int main()
{
    shopping s;
    s.menu();
}
