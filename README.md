# Supermarket Billing System (C++)

A supermarket billing program with a **Windows GUI dashboard** (`billing_gui.exe`) and the original **console** version (`billing.exe`). The **administrator** manages the product list, and the **buyer** picks products and gets a bill with discounts applied automatically.

---

## Table of Contents
1. [Quick Start](#1-quick-start)
2. [First-Time Setup (Installing the Compiler)](#2-first-time-setup-installing-the-compiler)
3. [How to Run](#3-how-to-run)
4. [Program Flow](#4-program-flow)
5. [Using the Administrator Section](#5-using-the-administrator-section)
6. [Using the Buyer Section](#6-using-the-buyer-section)
7. [Files in This Project](#7-files-in-this-project)
8. [How the Code Works](#8-how-the-code-works)
9. [Rules & Validations](#9-rules--validations)
10. [Troubleshooting](#10-troubleshooting)

---

## 1. Quick Start

If everything is already set up:

1. Open the project folder.
2. Double-click **`billing.exe`**, or open PowerShell in the folder and type:
   ```
   .\billing.exe
   ```
3. Admin login:

   | Field | Value |
   |---|---|
   | Email | `lakshmidevi@gmail.com` |
   | Password | `Ld@2026` |

### GUI Dashboard Version (recommended)

The project also has a window-based dashboard, so you don't need to use the terminal:

- Double-click **`billing_gui.exe`**.
- **Login screen:** log in as Administrator, or click **Continue as Buyer**.
- **Admin dashboard:** shows Total Products, Average Price, Bills Generated and Total Sales. Use the product table and the form beside it to **Add**, **Update** and **Delete** products. Click a row to load it into the form.
- **Buyer screen:** search products, set the quantity, then click **Add to Cart** (or double-click a product). The cart shows the live total and how much you save. **Generate Bill** opens the receipt and saves it to `bills.txt`.

The GUI uses the same `database.txt` and `bills.txt` files as the terminal version.

To build it from the source code:
```
g++ "Billing System GUI.cpp" -o billing_gui.exe -mwindows -static -lcomctl32 -lgdi32
```

---

## 2. First-Time Setup (Installing the Compiler)

C++ code must be **compiled** (converted into an `.exe` program) before it can run. This needs a compiler called **g++**. You only do this once per computer.

**Step 1 — Check if g++ is already installed.** Open PowerShell and type:
```
g++ --version
```
If you see a version number (e.g. `g++ ... 16.2.0`), skip to step 3.

**Step 2 — Install g++.** In PowerShell, run:
```
winget install BrechtSanders.WinLibs.POSIX.UCRT
```
Then **close PowerShell and open it again** (so it can find the new compiler) and repeat step 1.

**Step 3 — Compile the program.** Go to the project folder and run:
```
cd "C:\Users\Hp\Desktop\SuperMarket-Billing-System-Project-with-Cpp"
g++ -static -o billing.exe "Billing System.cpp"
```
This creates `billing.exe`. No message means it worked.

> **Note:** Recompile (step 3) every time you change `Billing System.cpp`, otherwise you will still be running the old version.

---

## 3. How to Run

```
cd "C:\Users\Hp\Desktop\SuperMarket-Billing-System-Project-with-Cpp"
.\billing.exe
```

**How to type answers:** type **one answer**, then press **Enter**. Wait for the next question before typing again.

To close the program, choose **3. EXIT** from the main menu.

---

## 4. Program Flow

```
                    ┌──────────────────────────┐
                    │     SUPERMARKET MAIN     │
                    │          MENU            │
                    └────────────┬─────────────┘
          ┌──────────────────────┼──────────────────────┐
          ▼                      ▼                      ▼
  1. ADMINISTRATOR           2. BUYER                3. EXIT
          │                      │                (program closes)
          ▼                      ▼
   Enter Email &          ┌──────────────┐
   Password               │  BUYER MENU  │
          │               └──────┬───────┘
   ┌──────┴──────┐         ┌─────┴────────────┐
   ▼             ▼         ▼                  ▼
 Wrong        Correct   1. BUY PRODUCT   2. BACK TO MENU
 (back to        │         │
 main menu)      │         ▼
                 │      Product list shown
                 │         │
                 │         ▼
                 │      Enter code/name + quantity ◄──┐
                 │         │                          │
                 │         ▼                          │
                 │      "Buy another?" ── y ──────────┘
                 │         │ n
                 │         ▼
                 │      Receipt printed + saved to bills.txt
                 ▼
       ┌───────────────────────┐
       │  ADMINISTRATOR MENU   │
       └───────────┬───────────┘
   ┌───────────┬───┴───────┬────────────┬─────────────┐
   ▼           ▼           ▼            ▼             ▼
1. VIEW     2. ADD      3. MODIFY    4. DELETE    5. BACK TO
PRODUCTS    PRODUCT     PRODUCT      PRODUCT      MAIN MENU
```

After each action, the program returns to the same menu, so you can do several things in a row.

---

## 5. Using the Administrator Section

From the main menu, type `1`, then enter the email and password.

### 1. View Products
Shows every product:
```
|P.No   Name    Price  Discount  |
  101   Bread   45      2%
  102   Milk    50      2%
  103   Butter  160     3%
```

### 2. Add Product
The current product list is shown first, so you can choose an unused code.

| Question | Example | Rule |
|---|---|---|
| Product Code | `106` | Must be a number not already in use |
| Name | `Rice` | One word only. Use `_` for spaces, e.g. `Basmati_Rice` |
| Price (Rs.) | `80` | Must be more than 0 |
| Discount (%) | `5` | Between 0 and 100 |

Result: `Record Inserted !`

### 3. Modify Product
1. The product list is shown.
2. Enter the code of the product to change, e.g. `102`.
3. Its current details are shown:
   `Current Details -> Code: 102, Name: Milk, Price: 50, Discount: 2%`
4. Enter the new code, name, price and discount. To keep a value the same, type the old value again.

Result: `Record Edited !`

### 4. Delete Product
1. The product list is shown.
2. Enter the code to delete, e.g. `103`.
3. Confirm: `Delete Butter (Code 103)? Press 'y' to confirm:`
   - `y` → product removed
   - anything else → cancelled, nothing deleted

### 5. Back to Menu
Logs out and returns to the main menu.

---

## 6. Using the Buyer Section

From the main menu, type `2`, then `1` (Buy Product).

**Example of a full order:**
```
Enter Product Code or Name: bread
Selected: Bread (Rs. 45)
Enter the Product Quantity: 2
Do you want to Buy another Product? ... y

Enter Product Code or Name: 102
Selected: Milk (Rs. 50)
Enter the Product Quantity: 1
Do you want to Buy another Product? ... n
```

**Receipt:**
```
||            RECEIPT           ||
Date: 22-10-2026 22:45:07

P.No    Name    Quantity    Price   Amount   Amount with Discount
101     Bread    2           45      90       88.2
102     Milk     1           50      50       49

You Saved: Rs. 2.8
Total Amount: Rs. 137.2

(Receipt saved to bills.txt)
```

**How the bill is calculated (for each product):**
```
Amount               = Price × Quantity                 → 45 × 2  = 90
Discount             = Amount × Discount% ÷ 100         → 90 × 2% = 1.8
Amount with Discount = Amount − Discount                → 90 − 1.8 = 88.2
Total Amount         = sum of all "Amount with Discount"
```

**Tips:**
- You can type the product **code** (`101`) or **name** (`bread`, `BREAD`, `Bread` all work).
- The same product can't be added twice in one order; enter the full quantity the first time.

---

## 7. Files in This Project

| File | What it is |
|---|---|
| `Billing System.cpp` | The source code (the program itself, written in C++) |
| `billing.exe` | The compiled program you run (created by the compile step) |
| `database.txt` | All products are stored here; changes made by the admin are saved here |
| `bills.txt` | Created automatically; every receipt is added here as a sales record |
| `README.md` | This guide |

**Format of `database.txt`** (one product per line: code, name, price, discount):
```
 101 Bread 45 2
 102 Milk 50 2
 103 Butter 160 3
 104 Ghee 700 4
 105 Almonds 1000 10
```
It's safer to change products through the Administrator menu than to edit this file by hand.

---

## 8. How the Code Works

Everything is inside one class called `shopping`. Each menu option is one function:

| Function | What it does |
|---|---|
| `main()` | Starting point; creates a `shopping` object and opens the main menu |
| `menu()` | Main menu; handles the admin login |
| `administrator()` | Admin menu (view / add / modify / delete) |
| `buyer()` | Buyer menu |
| `list()` | Reads `database.txt` and prints all products |
| `find()` | Searches for a product by code or name |
| `add()` | Checks the code is unused, then appends the new product to `database.txt` |
| `edit()` | Copies every product to `database1.txt`, replacing the edited one, then renames it to `database.txt` |
| `rem()` | Same as `edit()`, but skips the deleted product |
| `receipt()` | Takes the order, calculates the bill, prints it and saves it to `bills.txt` |

Helper functions at the top of the file:

| Helper | What it does |
|---|---|
| `input<T>()` | Reads a value safely; if the user types something wrong (e.g. letters for a number), it asks again |
| `inputRange()` | Reads a number and makes sure it's within limits (e.g. discount 0–100) |
| `toLower()` / `isNumber()` | Used by `find()` to match names regardless of capital letters |

**To change the admin login**, open `Billing System.cpp`, find this line in `menu()`, change it, and recompile:
```cpp
if (email == "lakshmidevi@gmail.com" && password == "Ld@2026")
```

---

## 9. Rules & Validations

| Situation | What the program does |
|---|---|
| Letters typed where a number is expected | `Invalid input, please try again:` |
| Wrong email or password | `Invalid Email/Password !`, back to main menu |
| Adding a code that already exists | `Product Code already exists !` |
| Price 0 or less, or discount outside 0–100 | Asks again with the allowed range |
| Product code/name not in the list (buyer) | `Product not found, Please Try Again !` |
| Same product twice in one order | `Product already in your order` |
| Quantity 0 or less | `Quantity must be greater than 0` |
| Modify/delete a code that doesn't exist | `Record not Found !` |

---

## 10. Troubleshooting

| Problem | Solution |
|---|---|
| `g++ is not recognized` | g++ isn't installed or PowerShell wasn't restarted. See [Section 2](#2-first-time-setup-installing-the-compiler). |
| `cannot open output file billing.exe: Permission denied` | The program is still running. Close it (choose **3. EXIT**) and compile again. |
| Login fails with the correct details | You may be running an old `billing.exe`. Recompile (Section 2, step 3). Check there are no extra spaces. |
| "Enter Email" was skipped | Type only one answer per line and press Enter. |
| Product list is empty | Make sure `database.txt` is in the same folder as `billing.exe`, and run the program from that folder. |
| Product name with spaces gets cut off | Names must be one word; use `_` instead of spaces (`Basmati_Rice`). |

---

## Author

Lakshmi Devi
