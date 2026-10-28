// SuperMarket Billing System - Windows GUI (Dashboard) version.
// Uses the same database.txt and bills.txt files as the terminal version.
//
// Build:
//   g++ "Billing System GUI.cpp" -o billing_gui.exe -mwindows -static -lcomctl32 -lgdi32

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#endif

#include <windows.h>
#include <commctrl.h>
#include <string>
#include <vector>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <ctime>

using namespace std;

// ============================================================
//                         DATA
// ============================================================

struct Product
{
    int code;
    string name;
    float price;
    float dis;
};

struct CartItem
{
    int code;
    int qty;
};

const char *DB_FILE = "database.txt";
const char *BILLS_FILE = "bills.txt";
const string ADMIN_EMAIL = "lakshmidevi@gmail.com";
const string ADMIN_PASSWORD = "Ld@2026";

vector<Product> products;
vector<CartItem> cart;

void loadProducts()
{
    products.clear();
    ifstream data(DB_FILE);
    Product p;
    while (data >> p.code >> p.name >> p.price >> p.dis)
    {
        products.push_back(p);
    }
}

// Writes products in the same format the terminal version uses.
void saveProducts()
{
    ofstream data(DB_FILE, ios::trunc);
    for (const Product &p : products)
    {
        data << " " << p.code << " " << p.name << " " << p.price << " " << p.dis << " \n";
    }
}

Product *findProduct(int code)
{
    for (Product &p : products)
    {
        if (p.code == code)
            return &p;
    }
    return NULL;
}

float finalPrice(const Product &p)
{
    return p.price - p.price * p.dis / 100;
}

string trim(const string &s)
{
    size_t start = 0, end = s.size();
    while (start < end && isspace((unsigned char)s[start]))
        start++;
    while (end > start && isspace((unsigned char)s[end - 1]))
        end--;
    return s.substr(start, end - start);
}

string toLower(string s)
{
    for (size_t i = 0; i < s.size(); i++)
        s[i] = tolower((unsigned char)s[i]);
    return s;
}

// Names are stored as one word (e.g. Basmati_Rice) and shown with spaces.
string displayName(string n)
{
    for (size_t i = 0; i < n.size(); i++)
        if (n[i] == '_')
            n[i] = ' ';
    return n;
}

string storedName(const string &s)
{
    // Drop control characters (Ctrl+Backspace types one into a text box)
    string n;
    for (size_t i = 0; i < s.size(); i++)
        if (!iscntrl((unsigned char)s[i]) || isspace((unsigned char)s[i]))
            n += s[i];
    n = trim(n);
    for (size_t i = 0; i < n.size(); i++)
        if (isspace((unsigned char)n[i]))
            n[i] = '_';
    return n;
}

string money(double v)
{
    char buf[32];
    snprintf(buf, sizeof(buf), "%.2f", v);
    return buf;
}

string number(double v)
{
    char buf[32];
    snprintf(buf, sizeof(buf), "%g", v);
    return buf;
}

bool parseInt(const string &s, int &out)
{
    string t = trim(s);
    if (t.empty())
        return false;
    char *end;
    long v = strtol(t.c_str(), &end, 10);
    if (*end != '\0')
        return false;
    out = (int)v;
    return true;
}

bool parseFloat(const string &s, float &out)
{
    string t = trim(s);
    if (t.empty())
        return false;
    char *end;
    double v = strtod(t.c_str(), &end);
    if (*end != '\0')
        return false;
    out = (float)v;
    return true;
}

// Counts receipts and adds up "Total Amount" lines in bills.txt.
void readBillStats(int &count, double &sales)
{
    count = 0;
    sales = 0;
    ifstream bills(BILLS_FILE);
    string line;
    const string totalTag = "Total Amount: Rs.";
    while (getline(bills, line))
    {
        if (line.find("RECEIPT") != string::npos)
            count++;
        size_t pos = line.find(totalTag);
        if (pos != string::npos)
            sales += atof(line.c_str() + pos + totalTag.size());
    }
}

// ============================================================
//                       UI GLOBALS
// ============================================================

enum Page
{
    PAGE_LOGIN,
    PAGE_ADMIN,
    PAGE_BUYER
};

enum ControlId
{
    ID_EMAIL = 100,
    ID_PASS,
    ID_LOGIN,
    ID_BUYER,
    ID_LOGOUT,
    ID_ADMIN_LIST = 200,
    ID_CODE,
    ID_NAME,
    ID_PRICE,
    ID_DIS,
    ID_ADD,
    ID_UPDATE,
    ID_DELETE,
    ID_CLEAR,
    ID_SEARCH = 300,
    ID_PRODUCTS,
    ID_QTY,
    ID_ADDCART,
    ID_CART,
    ID_REMOVE,
    ID_CLEARCART,
    ID_BILL
};

const COLORREF C_BG = RGB(241, 245, 249);
const COLORREF C_HEADER = RGB(30, 41, 59);
const COLORREF C_WHITE = RGB(255, 255, 255);
const COLORREF C_BORDER = RGB(226, 232, 240);
const COLORREF C_TEXT = RGB(15, 23, 42);
const COLORREF C_MUTED = RGB(100, 116, 139);
const COLORREF C_BLUE = RGB(37, 99, 235);
const COLORREF C_GREEN = RGB(22, 163, 74);
const COLORREF C_RED = RGB(220, 38, 38);
const COLORREF C_ORANGE = RGB(234, 88, 12);
const COLORREF C_GRAY = RGB(100, 116, 139);
const COLORREF C_PURPLE = RGB(124, 58, 237);

HINSTANCE hInst;
HWND hMain;
Page page = PAGE_LOGIN;
int dpi = 96;

HFONT fTitle, fHeading, fNormal, fSmall, fBig, fButton, fMono;

vector<HWND> loginCtl, adminCtl, buyerCtl, sharedCtl;
HWND hEmail, hPass, hLogout;
HWND hAdminList, hCode, hName, hPrice, hDis;
HWND hSearch, hProducts, hQty, hCart;

int selectedCode = -1; // product being edited on the admin page
int billCount = 0;
double totalSales = 0;
int cartItems = 0;
double cartTotal = 0, cartSaved = 0;
string receiptText;

// Scales a 96-DPI layout value to the screen's DPI.
int S(int v)
{
    return MulDiv(v, dpi, 96);
}

HFONT makeFont(int size, int weight, const char *face = "Segoe UI")
{
    return CreateFontA(-S(size), 0, 0, 0, weight, 0, 0, 0, DEFAULT_CHARSET, 0, 0,
                       CLEARTYPE_QUALITY, 0, face);
}

void showError(const string &msg)
{
    MessageBoxA(GetActiveWindow(), msg.c_str(), "Error", MB_OK | MB_ICONWARNING);
}

void showInfo(const string &msg)
{
    MessageBoxA(hMain, msg.c_str(), "Success", MB_OK | MB_ICONINFORMATION);
}

string getText(HWND h)
{
    int n = GetWindowTextLengthA(h);
    string s(n, '\0');
    GetWindowTextA(h, &s[0], n + 1);
    return s;
}

// ============================================================
//                    CONTROL HELPERS
// ============================================================

HWND makeControl(const char *cls, const char *text, DWORD style, DWORD exStyle, int id,
                 int x, int y, int w, int h, vector<HWND> &group)
{
    HWND ctl = CreateWindowExA(exStyle, cls, text, WS_CHILD | style, S(x), S(y), S(w), S(h),
                               hMain, (HMENU)(INT_PTR)id, hInst, NULL);
    SendMessage(ctl, WM_SETFONT, (WPARAM)fNormal, TRUE);
    group.push_back(ctl);
    return ctl;
}

HWND makeEdit(int id, int x, int y, int w, int h, vector<HWND> &group, DWORD extra = 0)
{
    return makeControl("EDIT", "", WS_TABSTOP | ES_AUTOHSCROLL | extra, WS_EX_CLIENTEDGE, id,
                       x, y, w, h, group);
}

HWND makeButton(const char *text, int id, int x, int y, int w, int h, COLORREF color,
                vector<HWND> &group)
{
    HWND btn = makeControl("BUTTON", text, WS_TABSTOP | BS_OWNERDRAW, 0, id, x, y, w, h, group);
    SetWindowLongPtr(btn, GWLP_USERDATA, (LONG_PTR)color);
    SendMessage(btn, WM_SETFONT, (WPARAM)fButton, TRUE);
    return btn;
}

HWND makeList(int id, int x, int y, int w, int h, const vector<pair<string, int>> &cols,
              vector<HWND> &group)
{
    HWND lv = makeControl(WC_LISTVIEWA, "",
                          WS_TABSTOP | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
                          WS_EX_CLIENTEDGE, id, x, y, w, h, group);
    ListView_SetExtendedListViewStyle(lv, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES |
                                              LVS_EX_DOUBLEBUFFER);
    for (size_t i = 0; i < cols.size(); i++)
    {
        LVCOLUMNA col = {};
        col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
        col.fmt = i < 2 ? LVCFMT_LEFT : LVCFMT_RIGHT;
        col.cx = S(cols[i].second);
        col.pszText = (char *)cols[i].first.c_str();
        ListView_InsertColumn(lv, i, &col);
    }
    return lv;
}

void addRow(HWND lv, const vector<string> &cells, int code)
{
    LVITEMA item = {};
    item.mask = LVIF_TEXT | LVIF_PARAM;
    item.iItem = ListView_GetItemCount(lv);
    item.pszText = (char *)cells[0].c_str();
    item.lParam = code;
    int row = ListView_InsertItem(lv, &item);
    for (size_t i = 1; i < cells.size(); i++)
    {
        ListView_SetItemText(lv, row, i, (char *)cells[i].c_str());
    }
}

int selectedRow(HWND lv)
{
    return ListView_GetNextItem(lv, -1, LVNI_SELECTED);
}

int rowCode(HWND lv, int row)
{
    LVITEMA item = {};
    item.mask = LVIF_PARAM;
    item.iItem = row;
    ListView_GetItem(lv, &item);
    return (int)item.lParam;
}

// Flat coloured button, slightly darker while pressed.
void drawButton(DRAWITEMSTRUCT *d)
{
    COLORREF c = (COLORREF)GetWindowLongPtr(d->hwndItem, GWLP_USERDATA);
    if (d->itemState & ODS_SELECTED)
        c = RGB(GetRValue(c) * 8 / 10, GetGValue(c) * 8 / 10, GetBValue(c) * 8 / 10);

    HBRUSH brush = CreateSolidBrush(c);
    FillRect(d->hDC, &d->rcItem, brush);
    DeleteObject(brush);

    char text[64];
    GetWindowTextA(d->hwndItem, text, sizeof(text));
    SetBkMode(d->hDC, TRANSPARENT);
    SetTextColor(d->hDC, C_WHITE);
    DrawTextA(d->hDC, text, -1, &d->rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    if (d->itemState & ODS_FOCUS)
    {
        RECT r = d->rcItem;
        InflateRect(&r, -S(3), -S(3));
        DrawFocusRect(d->hDC, &r);
    }
}

// ============================================================
//                    PAINT HELPERS
// ============================================================

void fillBox(HDC dc, int x, int y, int w, int h, COLORREF color)
{
    RECT r = {S(x), S(y), S(x + w), S(y + h)};
    HBRUSH brush = CreateSolidBrush(color);
    FillRect(dc, &r, brush);
    DeleteObject(brush);
}

void card(HDC dc, int x, int y, int w, int h)
{
    HBRUSH brush = CreateSolidBrush(C_WHITE);
    HPEN pen = CreatePen(PS_SOLID, 1, C_BORDER);
    HGDIOBJ oldBrush = SelectObject(dc, brush);
    HGDIOBJ oldPen = SelectObject(dc, pen);
    RoundRect(dc, S(x), S(y), S(x + w), S(y + h), S(12), S(12));
    SelectObject(dc, oldBrush);
    SelectObject(dc, oldPen);
    DeleteObject(brush);
    DeleteObject(pen);
}

void text(HDC dc, const string &s, int x, int y, int w, int h, HFONT font, COLORREF color,
          UINT format = DT_LEFT | DT_VCENTER | DT_SINGLELINE)
{
    RECT r = {S(x), S(y), S(x + w), S(y + h)};
    SelectObject(dc, font);
    SetTextColor(dc, color);
    SetBkMode(dc, TRANSPARENT);
    DrawTextA(dc, s.c_str(), -1, &r, format | DT_NOPREFIX);
}

void statCard(HDC dc, int x, const string &label, const string &value, COLORREF accent)
{
    card(dc, x, 90, 250, 95);
    fillBox(dc, x, 104, 5, 67, accent);
    text(dc, label, x + 22, 100, 220, 26, fSmall, C_MUTED);
    text(dc, value, x + 22, 126, 220, 46, fBig, C_TEXT);
}

void paintLogin(HDC dc)
{
    card(dc, 330, 120, 440, 420);
    text(dc, "Welcome Back", 330, 145, 440, 40, fHeading, C_TEXT, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    text(dc, "Login as administrator, or continue as a buyer", 330, 185, 440, 24, fSmall, C_MUTED,
         DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    text(dc, "Email", 370, 238, 360, 22, fSmall, C_TEXT);
    text(dc, "Password", 370, 306, 360, 22, fSmall, C_TEXT);
    text(dc, "-  or  -", 330, 432, 440, 34, fSmall, C_MUTED, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

void paintAdmin(HDC dc)
{
    double avg = 0;
    for (const Product &p : products)
        avg += p.price;
    if (!products.empty())
        avg /= products.size();

    statCard(dc, 20, "Total Products", to_string(products.size()), C_BLUE);
    statCard(dc, 290, "Average Price", "Rs. " + money(avg), C_PURPLE);
    statCard(dc, 560, "Bills Generated", to_string(billCount), C_ORANGE);
    statCard(dc, 830, "Total Sales", "Rs. " + money(totalSales), C_GREEN);

    text(dc, "All Products", 20, 198, 680, 32, fHeading, C_TEXT);

    card(dc, 720, 200, 360, 460);
    text(dc, "Product Details", 740, 212, 320, 34, fHeading, C_TEXT);
    text(dc, "Product Code", 740, 255, 320, 22, fSmall, C_TEXT);
    text(dc, "Product Name", 740, 317, 320, 22, fSmall, C_TEXT);
    text(dc, "Price (Rs.)", 740, 379, 320, 22, fSmall, C_TEXT);
    text(dc, "Discount (0-100 %)", 740, 441, 320, 22, fSmall, C_TEXT);
    text(dc, "Tip: click a product in the list to load it here, then Update or Delete.",
         740, 612, 320, 40, fSmall, C_MUTED, DT_LEFT | DT_WORDBREAK);
}

void paintBuyer(HDC dc)
{
    text(dc, "Products", 20, 82, 440, 34, fHeading, C_TEXT);
    text(dc, "Your Cart", 480, 82, 600, 34, fHeading, C_TEXT);
    text(dc, "Qty", 20, 561, 40, 36, fNormal, C_TEXT);
    text(dc, "Tip: double-click a product to add it to the cart.", 20, 612, 440, 40, fSmall, C_MUTED);

    card(dc, 480, 558, 600, 44);
    text(dc, "Items: " + to_string(cartItems), 498, 558, 140, 44, fNormal, C_TEXT);
    text(dc, "You Save: Rs. " + money(cartSaved), 640, 558, 200, 44, fNormal, C_GREEN);
    text(dc, "Total: Rs. " + money(cartTotal), 840, 558, 225, 44, fHeading, C_TEXT,
         DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
}

void paint(HDC dc, const RECT &client)
{
    HBRUSH bg = CreateSolidBrush(C_BG);
    FillRect(dc, &client, bg);
    DeleteObject(bg);

    RECT header = {0, 0, client.right, S(72)};
    HBRUSH hb = CreateSolidBrush(C_HEADER);
    FillRect(dc, &header, hb);
    DeleteObject(hb);
    text(dc, "SuperMarket Billing System", 24, 0, 500, 72, fTitle, C_WHITE);

    if (page == PAGE_ADMIN)
        text(dc, "Administrator: Lakshmi Devi", 520, 0, 430, 72, fNormal, RGB(203, 213, 225),
             DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
    else if (page == PAGE_BUYER)
        text(dc, "Welcome, Buyer", 520, 0, 430, 72, fNormal, RGB(203, 213, 225),
             DT_RIGHT | DT_VCENTER | DT_SINGLELINE);

    if (page == PAGE_LOGIN)
        paintLogin(dc);
    else if (page == PAGE_ADMIN)
        paintAdmin(dc);
    else
        paintBuyer(dc);
}

// ============================================================
//                    ADMINISTRATOR PAGE
// ============================================================

void clearForm()
{
    selectedCode = -1;
    SetWindowTextA(hCode, "");
    SetWindowTextA(hName, "");
    SetWindowTextA(hPrice, "");
    SetWindowTextA(hDis, "");
    ListView_SetItemState(hAdminList, -1, 0, LVIS_SELECTED);
}

void refreshAdmin()
{
    loadProducts();
    readBillStats(billCount, totalSales);

    ListView_DeleteAllItems(hAdminList);
    for (const Product &p : products)
    {
        addRow(hAdminList, {to_string(p.code), displayName(p.name), money(p.price),
                            number(p.dis) + " %", money(finalPrice(p))},
               p.code);
    }
    InvalidateRect(hMain, NULL, FALSE);
}

void fillForm(int code)
{
    Product *p = findProduct(code);
    if (!p)
        return;
    selectedCode = code;
    SetWindowTextA(hCode, to_string(p->code).c_str());
    SetWindowTextA(hName, displayName(p->name).c_str());
    SetWindowTextA(hPrice, number(p->price).c_str());
    SetWindowTextA(hDis, number(p->dis).c_str());
}

// Validates the form. Shows an error and returns false on bad input.
bool readForm(Product &p)
{
    if (!parseInt(getText(hCode), p.code) || p.code <= 0)
    {
        showError("Please enter a valid Product Code (a positive whole number).");
        SetFocus(hCode);
        return false;
    }
    p.name = storedName(getText(hName));
    if (p.name.empty())
    {
        showError("Please enter the Product Name.");
        SetFocus(hName);
        return false;
    }
    if (!parseFloat(getText(hPrice), p.price) || p.price < 0.01 || p.price > 1000000)
    {
        showError("Price must be a number between 0.01 and 1000000.");
        SetFocus(hPrice);
        return false;
    }
    if (!parseFloat(getText(hDis), p.dis) || p.dis < 0 || p.dis > 100)
    {
        showError("Discount must be a number between 0 and 100.");
        SetFocus(hDis);
        return false;
    }
    return true;
}

void addProduct()
{
    Product p;
    if (!readForm(p))
        return;
    loadProducts();
    if (findProduct(p.code))
    {
        showError("Product Code already exists! Please use a different code.");
        SetFocus(hCode);
        return;
    }
    products.push_back(p);
    saveProducts();
    refreshAdmin();
    clearForm();
    showInfo("Product \"" + displayName(p.name) + "\" added successfully!");
}

void updateProduct()
{
    if (selectedCode == -1)
    {
        showError("Please select a product from the list first.");
        return;
    }
    Product p;
    if (!readForm(p))
        return;
    loadProducts();
    if (p.code != selectedCode && findProduct(p.code))
    {
        showError("Another product already uses code " + to_string(p.code) + ".");
        SetFocus(hCode);
        return;
    }
    Product *old = findProduct(selectedCode);
    if (!old)
    {
        showError("Record not found! It may have been deleted.");
        refreshAdmin();
        clearForm();
        return;
    }
    *old = p;
    saveProducts();
    refreshAdmin();
    clearForm();
    showInfo("Product updated successfully!");
}

void deleteProduct()
{
    if (selectedCode == -1)
    {
        showError("Please select a product from the list first.");
        return;
    }
    loadProducts();
    Product *p = findProduct(selectedCode);
    if (!p)
    {
        showError("Record not found! It may have already been deleted.");
        refreshAdmin();
        clearForm();
        return;
    }
    string msg = "Delete " + displayName(p->name) + " (Code " + to_string(p->code) + ")?";
    if (MessageBoxA(hMain, msg.c_str(), "Confirm Delete", MB_YESNO | MB_ICONWARNING) != IDYES)
        return;

    products.erase(products.begin() + (p - &products[0]));
    saveProducts();
    refreshAdmin();
    clearForm();
    showInfo("Product deleted successfully!");
}

// ============================================================
//                       BUYER PAGE
// ============================================================

void refreshProducts()
{
    string query = toLower(trim(getText(hSearch)));
    ListView_DeleteAllItems(hProducts);
    for (const Product &p : products)
    {
        string code = to_string(p.code);
        if (query.empty() || toLower(displayName(p.name)).find(query) != string::npos ||
            code.find(query) == 0)
        {
            addRow(hProducts, {code, displayName(p.name), money(p.price), number(p.dis) + " %"},
                   p.code);
        }
    }
}

void refreshCart()
{
    ListView_DeleteAllItems(hCart);
    cartItems = 0;
    cartTotal = 0;
    cartSaved = 0;
    for (const CartItem &item : cart)
    {
        Product *p = findProduct(item.code);
        if (!p)
            continue;
        double amount = p->price * item.qty;
        double net = amount - amount * p->dis / 100;
        cartItems += item.qty;
        cartTotal += net;
        cartSaved += amount - net;
        addRow(hCart, {to_string(p->code), displayName(p->name), to_string(item.qty),
                       money(p->price), money(amount), money(net)},
               p->code);
    }
    InvalidateRect(hMain, NULL, FALSE);
}

void addToCart()
{
    int row = selectedRow(hProducts);
    if (row < 0)
    {
        showError("Please select a product from the list first.");
        return;
    }
    int qty;
    if (!parseInt(getText(hQty), qty) || qty <= 0 || qty > 10000)
    {
        showError("Quantity must be a whole number between 1 and 10000.");
        SetFocus(hQty);
        return;
    }
    int code = rowCode(hProducts, row);

    bool found = false;
    for (CartItem &item : cart)
    {
        if (item.code == code)
        {
            item.qty += qty;
            found = true;
        }
    }
    if (!found)
        cart.push_back({code, qty});

    SetWindowTextA(hQty, "1");
    refreshCart();
}

void removeFromCart()
{
    int row = selectedRow(hCart);
    if (row < 0)
    {
        showError("Please select an item in your cart first.");
        return;
    }
    int code = rowCode(hCart, row);
    for (size_t i = 0; i < cart.size(); i++)
    {
        if (cart[i].code == code)
        {
            cart.erase(cart.begin() + i);
            break;
        }
    }
    refreshCart();
}

string centered(const string &s, int width)
{
    int pad = (width - (int)s.size()) / 2;
    return string(pad > 0 ? pad : 0, ' ') + s + "\n";
}

string buildReceipt()
{
    const int W = 60;
    string line(W, '-'), dline(W, '=');

    time_t now = time(0);
    char date[32];
    strftime(date, sizeof(date), "%d-%m-%Y %H:%M:%S", localtime(&now));

    string bill;
    bill += dline + "\n";
    bill += centered("SUPERMARKET BILLING SYSTEM", W);
    bill += centered("RECEIPT", W);
    bill += dline + "\n";
    bill += "Date: " + string(date) + "\n";
    bill += line + "\n";

    char row[200];
    snprintf(row, sizeof(row), "%-6s%-18s%5s%10s%10s%11s\n",
             "Code", "Name", "Qty", "Price", "Amount", "Net Amt");
    bill += row;
    bill += line + "\n";

    double total = 0, saved = 0;
    for (const CartItem &item : cart)
    {
        Product *p = findProduct(item.code);
        if (!p)
            continue;
        double amount = p->price * item.qty;
        double net = amount - amount * p->dis / 100;
        total += net;
        saved += amount - net;
        snprintf(row, sizeof(row), "%-6d%-18.17s%5d%10.2f%10.2f%11.2f\n",
                 p->code, displayName(p->name).c_str(), item.qty, p->price, amount, net);
        bill += row;
    }

    bill += line + "\n";
    snprintf(row, sizeof(row), "You Saved:    Rs. %.2f\nTotal Amount: Rs. %.2f\n", saved, total);
    bill += row;
    bill += dline + "\n";
    bill += centered("Thank you for shopping with us!", W);
    return bill;
}

void showReceipt(const string &bill)
{
    receiptText.clear();
    for (char c : bill)
    {
        if (c == '\n')
            receiptText += "\r\n";
        else
            receiptText += c;
    }

    DWORD style = WS_CAPTION | WS_SYSMENU;
    RECT r = {0, 0, S(560), S(560)};
    AdjustWindowRect(&r, style, FALSE);
    RECT m;
    GetWindowRect(hMain, &m);
    int w = r.right - r.left, h = r.bottom - r.top;
    int x = m.left + (m.right - m.left - w) / 2;
    int y = m.top + (m.bottom - m.top - h) / 2;
    CreateWindowExA(0, "ReceiptWnd", "Receipt", style | WS_VISIBLE, x, y, w, h, hMain, NULL,
                    hInst, NULL);
}

void generateBill()
{
    if (cart.empty())
    {
        showError("Your cart is empty. Please add some products first.");
        return;
    }
    loadProducts();
    string bill = buildReceipt();

    ofstream bills(BILLS_FILE, ios::app);
    bills << "\n" << bill << "________________________________\n";
    bills.close();

    cart.clear();
    refreshProducts();
    refreshCart();
    showReceipt(bill);
}

// ============================================================
//                   PAGES & WINDOWS
// ============================================================

void showPage(Page p)
{
    page = p;
    vector<HWND> *groups[] = {&loginCtl, &adminCtl, &buyerCtl, &sharedCtl};
    for (vector<HWND> *g : groups)
        for (HWND h : *g)
            ShowWindow(h, SW_HIDE);

    vector<HWND> &active = p == PAGE_LOGIN ? loginCtl : p == PAGE_ADMIN ? adminCtl : buyerCtl;
    for (HWND h : active)
        ShowWindow(h, SW_SHOW);
    if (p != PAGE_LOGIN)
        ShowWindow(hLogout, SW_SHOW);

    if (p == PAGE_LOGIN)
    {
        SetWindowTextA(hPass, "");
        SetFocus(hEmail);
    }
    else if (p == PAGE_ADMIN)
    {
        refreshAdmin();
        clearForm();
        SetFocus(hAdminList);
    }
    else
    {
        loadProducts();
        cart.clear();
        SetWindowTextA(hSearch, "");
        SetWindowTextA(hQty, "1");
        refreshProducts();
        refreshCart();
        SetFocus(hSearch);
    }
    InvalidateRect(hMain, NULL, FALSE);
}

void doLogin()
{
    string email = trim(getText(hEmail));
    string password = getText(hPass);
    if (email == ADMIN_EMAIL && password == ADMIN_PASSWORD)
    {
        showPage(PAGE_ADMIN);
    }
    else
    {
        showError("Invalid Email/Password!");
        SetFocus(hPass);
    }
}

void createControls()
{
    // Login page
    hEmail = makeEdit(ID_EMAIL, 370, 262, 360, 32, loginCtl);
    hPass = makeEdit(ID_PASS, 370, 330, 360, 32, loginCtl, ES_PASSWORD);
    SendMessageW(hEmail, EM_SETCUEBANNER, TRUE, (LPARAM)L"Enter your email");
    SendMessageW(hPass, EM_SETCUEBANNER, TRUE, (LPARAM)L"Enter your password");
    makeButton("Login as Administrator", ID_LOGIN, 370, 385, 360, 42, C_BLUE, loginCtl);
    makeButton("Continue as Buyer", ID_BUYER, 370, 468, 360, 42, C_GREEN, loginCtl);

    hLogout = makeButton("Logout", ID_LOGOUT, 970, 18, 110, 36, C_RED, sharedCtl);

    // Administrator page
    hAdminList = makeList(ID_ADMIN_LIST, 20, 235, 680, 425,
                          {{"Code", 70}, {"Product Name", 210}, {"Price (Rs.)", 115},
                           {"Discount", 95}, {"Final Price (Rs.)", 140}},
                          adminCtl);
    hCode = makeEdit(ID_CODE, 740, 277, 320, 30, adminCtl, ES_NUMBER);
    hName = makeEdit(ID_NAME, 740, 339, 320, 30, adminCtl);
    hPrice = makeEdit(ID_PRICE, 740, 401, 320, 30, adminCtl);
    hDis = makeEdit(ID_DIS, 740, 463, 320, 30, adminCtl);
    makeButton("Add Product", ID_ADD, 740, 513, 155, 40, C_GREEN, adminCtl);
    makeButton("Update", ID_UPDATE, 905, 513, 155, 40, C_BLUE, adminCtl);
    makeButton("Delete", ID_DELETE, 740, 563, 155, 40, C_RED, adminCtl);
    makeButton("Clear", ID_CLEAR, 905, 563, 155, 40, C_GRAY, adminCtl);

    // Buyer page
    hSearch = makeEdit(ID_SEARCH, 20, 120, 440, 30, buyerCtl);
    SendMessageW(hSearch, EM_SETCUEBANNER, TRUE, (LPARAM)L"Search by product name or code...");
    hProducts = makeList(ID_PRODUCTS, 20, 160, 440, 390,
                         {{"Code", 55}, {"Name", 145}, {"Price (Rs.)", 105}, {"Discount", 85}},
                         buyerCtl);
    hQty = makeEdit(ID_QTY, 60, 563, 70, 32, buyerCtl, ES_NUMBER);
    makeButton("Add to Cart", ID_ADDCART, 145, 561, 315, 36, C_BLUE, buyerCtl);
    hCart = makeList(ID_CART, 480, 120, 600, 430,
                     {{"Code", 55}, {"Name", 150}, {"Qty", 55}, {"Price", 90},
                      {"Amount", 100}, {"Net Amount", 115}},
                     buyerCtl);
    makeButton("Remove Item", ID_REMOVE, 480, 612, 180, 42, C_ORANGE, buyerCtl);
    makeButton("Clear Cart", ID_CLEARCART, 670, 612, 180, 42, C_GRAY, buyerCtl);
    makeButton("Generate Bill", ID_BILL, 860, 612, 220, 42, C_GREEN, buyerCtl);
}

LRESULT CALLBACK ReceiptProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
    case WM_CREATE:
    {
        EnableWindow(hMain, FALSE);
        HWND edit = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", receiptText.c_str(),
                                    WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE |
                                        ES_READONLY | ES_AUTOVSCROLL,
                                    S(16), S(16), S(528), S(470), hwnd, NULL, hInst, NULL);
        SendMessage(edit, WM_SETFONT, (WPARAM)fMono, TRUE);
        HWND btn = CreateWindowExA(0, "BUTTON", "Close", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_OWNERDRAW,
                                   S(16), S(500), S(528), S(44), hwnd, (HMENU)IDOK, hInst, NULL);
        SetWindowLongPtr(btn, GWLP_USERDATA, (LONG_PTR)C_BLUE);
        SendMessage(btn, WM_SETFONT, (WPARAM)fButton, TRUE);
        SetFocus(btn);
        return 0;
    }
    case WM_CTLCOLORSTATIC:
        SetBkColor((HDC)wp, C_WHITE);
        SetTextColor((HDC)wp, C_TEXT);
        return (LRESULT)GetStockObject(WHITE_BRUSH);
    case WM_DRAWITEM:
        drawButton((DRAWITEMSTRUCT *)lp);
        return TRUE;
    case WM_COMMAND:
        if (LOWORD(wp) == IDOK || LOWORD(wp) == IDCANCEL)
            SendMessage(hwnd, WM_CLOSE, 0, 0);
        return 0;
    case WM_CLOSE:
        // Re-enable the main window first so focus returns to it
        EnableWindow(hMain, TRUE);
        DestroyWindow(hwnd);
        SetForegroundWindow(hMain);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wp, lp);
}

LRESULT CALLBACK MainProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
    case WM_CREATE:
        hMain = hwnd;
        createControls();
        showPage(PAGE_LOGIN);
        return 0;

    case WM_ERASEBKGND:
        return 1;

    case WM_PAINT:
    {
        // Draw into an off-screen bitmap first to avoid flicker
        PAINTSTRUCT ps;
        HDC dc = BeginPaint(hwnd, &ps);
        RECT client;
        GetClientRect(hwnd, &client);
        HDC mem = CreateCompatibleDC(dc);
        HBITMAP bmp = CreateCompatibleBitmap(dc, client.right, client.bottom);
        HGDIOBJ old = SelectObject(mem, bmp);
        paint(mem, client);
        BitBlt(dc, 0, 0, client.right, client.bottom, mem, 0, 0, SRCCOPY);
        SelectObject(mem, old);
        DeleteObject(bmp);
        DeleteDC(mem);
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_DRAWITEM:
        drawButton((DRAWITEMSTRUCT *)lp);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wp))
        {
        case ID_LOGIN:
            doLogin();
            break;
        case ID_BUYER:
            showPage(PAGE_BUYER);
            break;
        case ID_LOGOUT:
            if (page == PAGE_BUYER && !cart.empty() &&
                MessageBoxA(hwnd, "Your cart is not empty. Logout anyway?", "Logout",
                            MB_YESNO | MB_ICONQUESTION) != IDYES)
                break;
            showPage(PAGE_LOGIN);
            break;
        case ID_ADD:
            addProduct();
            break;
        case ID_UPDATE:
            updateProduct();
            break;
        case ID_DELETE:
            deleteProduct();
            break;
        case ID_CLEAR:
            clearForm();
            SetFocus(hCode);
            break;
        case ID_SEARCH:
            if (HIWORD(wp) == EN_CHANGE)
                refreshProducts();
            break;
        case ID_ADDCART:
            addToCart();
            break;
        case ID_REMOVE:
            removeFromCart();
            break;
        case ID_CLEARCART:
            cart.clear();
            refreshCart();
            break;
        case ID_BILL:
            generateBill();
            break;
        }
        return 0;

    case WM_NOTIFY:
    {
        NMHDR *n = (NMHDR *)lp;
        if (n->hwndFrom == hAdminList && n->code == LVN_ITEMCHANGED)
        {
            NMLISTVIEW *lv = (NMLISTVIEW *)lp;
            if ((lv->uNewState & LVIS_SELECTED) && !(lv->uOldState & LVIS_SELECTED))
                fillForm(rowCode(hAdminList, lv->iItem));
        }
        else if (n->hwndFrom == hProducts && n->code == NM_DBLCLK)
        {
            addToCart();
        }
        return 0;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wp, lp);
}

// Turns on modern (Windows 10/11) control styles without a manifest file.
void enableVisualStyles()
{
    char dir[MAX_PATH];
    GetSystemDirectoryA(dir, MAX_PATH);
    string path = string(dir) + "\\shell32.dll";
    ACTCTXA ctx = {};
    ctx.cbSize = sizeof(ctx);
    ctx.dwFlags = ACTCTX_FLAG_RESOURCE_NAME_VALID | ACTCTX_FLAG_ASSEMBLY_DIRECTORY_VALID;
    ctx.lpSource = path.c_str();
    ctx.lpAssemblyDirectory = dir;
    ctx.lpResourceName = MAKEINTRESOURCEA(124);
    HANDLE h = CreateActCtxA(&ctx);
    ULONG_PTR cookie;
    if (h != INVALID_HANDLE_VALUE)
        ActivateActCtx(h, &cookie);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    hInst = hInstance;

    // Run from the exe's folder so database.txt / bills.txt are found
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    string folder = exePath;
    folder = folder.substr(0, folder.find_last_of("\\/"));
    SetCurrentDirectoryA(folder.c_str());

    SetProcessDPIAware();
    HDC screen = GetDC(NULL);
    dpi = GetDeviceCaps(screen, LOGPIXELSY);
    ReleaseDC(NULL, screen);

    enableVisualStyles();
    INITCOMMONCONTROLSEX icc = {sizeof(icc), ICC_LISTVIEW_CLASSES | ICC_STANDARD_CLASSES};
    InitCommonControlsEx(&icc);

    fTitle = makeFont(22, FW_BOLD);
    fHeading = makeFont(18, FW_SEMIBOLD);
    fNormal = makeFont(15, FW_NORMAL);
    fSmall = makeFont(13, FW_NORMAL);
    fBig = makeFont(26, FW_BOLD);
    fButton = makeFont(15, FW_SEMIBOLD);
    fMono = makeFont(15, FW_NORMAL, "Consolas");

    WNDCLASSA wc = {};
    wc.lpfnWndProc = MainProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.lpszClassName = "SuperMarketWnd";
    RegisterClassA(&wc);

    WNDCLASSA rc = {};
    rc.lpfnWndProc = ReceiptProc;
    rc.hInstance = hInstance;
    rc.hCursor = LoadCursor(NULL, IDC_ARROW);
    rc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    rc.hbrBackground = CreateSolidBrush(C_BG);
    rc.lpszClassName = "ReceiptWnd";
    RegisterClassA(&rc);

    DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN;
    RECT r = {0, 0, S(1100), S(680)};
    AdjustWindowRect(&r, style, FALSE);
    int w = r.right - r.left, h = r.bottom - r.top;
    int x = (GetSystemMetrics(SM_CXSCREEN) - w) / 2;
    int y = (GetSystemMetrics(SM_CYSCREEN) - h) / 2;
    if (y < 0)
        y = 0;

    HWND hwnd = CreateWindowExA(0, "SuperMarketWnd", "SuperMarket Billing System", style,
                                x, y, w, h, NULL, NULL, hInstance, NULL);
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        // Enter key shortcuts
        if (msg.message == WM_KEYDOWN && msg.wParam == VK_RETURN)
        {
            if (msg.hwnd == hEmail || msg.hwnd == hPass)
            {
                doLogin();
                continue;
            }
            if (msg.hwnd == hQty)
            {
                addToCart();
                continue;
            }
        }
        // Tab / Shift+Tab moves between fields
        HWND root = GetAncestor(msg.hwnd, GA_ROOT);
        if (root && IsDialogMessage(root, &msg))
            continue;
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
