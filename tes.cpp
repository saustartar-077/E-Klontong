#include <iostream>
#include <iomanip>
#include <string>
#include <conio.h> // Non-standard, specific to Windows
#include <fstream>
#include <vector>
#include <map>
#include <sstream> // Added missing header

using namespace std;

// Constants for array sizes
const int MAX_PRODUK = 100;
const int MAX_USER = 100;

// NEW: Predefined list of categories
const vector<string> daftarKategoriTetap = {
    "Elektronik", "Makanan & Minuman", "Kesehatan & Kecantikan",
    "Peralatan Rumah Tangga", "Alat Tulis & Kantor", "Aksesoris & Fashion",
    "Otomotif", "Mainan & Hobi", "Buku & Edukasi", "Lainnya"
};

// Struct definition for a Product
struct Produk {
    int id;
    string nama;
    int stok;
    int harga;
    string kategori; // NEW: Added category field
};

// Node for the shopping cart (Linked List)
struct NodeKeranjang {
    Produk* produk;
    int jumlah;
    NodeKeranjang* next;
};

// Node for transaction history (Doubly Linked List)
struct NodeRiwayat {
    double total;
    NodeKeranjang* detail; // The cart items at the time of transaction
    NodeRiwayat* next;
    NodeRiwayat* prev;
};

// Struct definition for a User
struct User {
    string username;
    string password;
    double saldo = 0;
};


// --- Global Variables ---
// Product data
Produk daftarProduk[MAX_PRODUK];
int jumlahProduk = 0;

// Shopping Cart
NodeKeranjang* headKeranjang = nullptr;

// Transaction History
NodeRiwayat* headRiwayat = nullptr;
NodeRiwayat* tailRiwayat = nullptr;

// User data
User daftarUser[MAX_USER];
int jumlahUser = 0;
User* currentUser = nullptr;


// --- Utility Functions ---

void clearInput() {
    cin.clear();
    cin.ignore(10000, '\n');
}

int inputInt(string prompt) {
    int value;
    while (true) {
        cout << prompt;
        cin >> value;
        if (!cin.fail()) break;
        cout << "Input harus berupa angka bulat!\n";
        clearInput();
    }
    return value;
}

double inputDouble(string prompt) {
    double value;
    while (true) {
        cout << prompt;
        cin >> value;
        if (!cin.fail()) break;
        cout << "Input harus berupa angka desimal (mis. 12000.50)!\n";
        clearInput();
    }
    return value;
}


// --- Product and Category Management ---

Produk* cariProdukById(int id) {
    for (int i = 0; i < jumlahProduk; i++) {
        if (daftarProduk[i].id == id) {
            return &daftarProduk[i];
        }
    }
    return nullptr;
}

// NEW: Function to select a category from the predefined list
string pilihKategori() {
    cout << "\nPilih Kategori Produk:\n";
    for (size_t i = 0; i < daftarKategoriTetap.size(); ++i) {
        cout << i + 1 << ". " << daftarKategoriTetap[i] << endl;
    }
    int pilihan = 0;
    while (true) {
        pilihan = inputInt("Pilih nomor kategori: ");
        if (pilihan > 0 && pilihan <= daftarKategoriTetap.size()) {
            return daftarKategoriTetap[pilihan - 1];
        }
        cout << "Pilihan tidak valid. Silakan coba lagi.\n";
    }
}

// MODIFIED: Save and load now include the category field
void simpanProdukKeCSV() {
    ofstream file("produk.csv");
    if (!file.is_open()) {
        cout << "Gagal membuka file untuk menyimpan produk!\n";
        return;
    }
    file << "ID,Nama,Stok,Harga,Kategori\n"; // Added Kategori header
    for (int i = 0; i < jumlahProduk; i++) {
        file << daftarProduk[i].id << ",\""
             << daftarProduk[i].nama << "\","
             << daftarProduk[i].stok << ","
             << daftarProduk[i].harga << ",\""
             << daftarProduk[i].kategori << "\"\n"; // Save category
    }
    file.close();
}

void muatProdukDariCSV() {
    ifstream file("produk.csv");
    if (!file.is_open()) return;

    string line;
    getline(file, line); // Skip header

    jumlahProduk = 0;
    while (getline(file, line) && jumlahProduk < MAX_PRODUK) {
        if (line.empty()) continue;

        stringstream ss(line);
        string id_str, nama, stok_str, harga_str, kategori;

        getline(ss, id_str, ',');
        
        // Read name enclosed in quotes
        ss.ignore(); getline(ss, nama, '"'); ss.ignore();

        getline(ss, stok_str, ',');
        getline(ss, harga_str, ',');

        // Read category enclosed in quotes
        ss.ignore(); getline(ss, kategori, '"');

        try {
            Produk& p = daftarProduk[jumlahProduk];
            p.id = stoi(id_str);
            p.nama = nama;
            p.stok = stoi(stok_str);
            p.harga = stoi(harga_str);
            p.kategori = kategori;
            jumlahProduk++;
        } catch (const std::invalid_argument& e) {
            // Silently ignore malformed lines
        }
    }
    file.close();
}

// MODIFIED: Product input now uses category selection
void inputProdukBaru() {
    if (jumlahProduk >= MAX_PRODUK) {
        cout << "Database produk penuh!\n";
        return;
    }

    cout << "\n=== Tambah Produk Baru ===\n";
    int id = inputInt("ID Produk: ");
    if (cariProdukById(id) != nullptr) {
        cout << "ID sudah digunakan!\n";
        return;
    }

    cin.ignore();
    string nama;
    cout << "Nama Produk: ";
    getline(cin, nama);

    int stok = inputInt("Stok: ");
    if (stok < 0) {
        cout << "Stok tidak boleh kurang dari 0!\n";
        return;
    }
    int harga = inputInt("Harga: ");
    if (harga < 0) {
        cout << "Harga tidak boleh kurang dari 0!\n";
        return;
    }

    string kategori = pilihKategori();

    Produk& produkBaru = daftarProduk[jumlahProduk];
    produkBaru = {id, nama, stok, harga, kategori};
    
    jumlahProduk++;
    simpanProdukKeCSV();
    cout << "Produk berhasil ditambahkan!\n";
}

void tampilkanProduk() {
    if (jumlahProduk == 0) {
        cout << "\nBelum ada produk yang tersedia.\n";
        return;
    }
    cout << "\nDaftar Semua Produk:\n";
    cout << "-------------------------------------------------------------------\n";
    cout << left << setw(5) << "ID" << setw(25) << "Nama" << setw(20) << "Kategori" << setw(10) << "Stok" << "Harga\n";
    cout << "-------------------------------------------------------------------\n";
    for (int i = 0; i < jumlahProduk; i++) {
        cout << left << setw(5) << daftarProduk[i].id
             << setw(25) << daftarProduk[i].nama
             << setw(20) << daftarProduk[i].kategori
             << setw(10) << daftarProduk[i].stok
             << "Rp " << fixed << setprecision(0) << daftarProduk[i].harga << endl;
    }
    cout << "-------------------------------------------------------------------\n";
}

// NEW: Function to display products by a chosen category
void tampilkanProdukByKategori() {
    cout << "\n=== Tampilkan Produk Berdasarkan Kategori ===\n";
    string kategoriDipilih = pilihKategori();
    
    cout << "\nMenampilkan produk untuk kategori: " << kategoriDipilih << endl;
    cout << "-------------------------------------------------------------------\n";
    cout << left << setw(5) << "ID" << setw(25) << "Nama" << setw(20) << "Kategori" << setw(10) << "Stok" << "Harga\n";
    cout << "-------------------------------------------------------------------\n";

    bool found = false;
    for(int i = 0; i < jumlahProduk; ++i) {
        if(daftarProduk[i].kategori == kategoriDipilih) {
            cout << left << setw(5) << daftarProduk[i].id
                 << setw(25) << daftarProduk[i].nama
                 << setw(20) << daftarProduk[i].kategori
                 << setw(10) << daftarProduk[i].stok
                 << "Rp " << fixed << setprecision(0) << daftarProduk[i].harga << endl;
            found = true;
        }
    }

    if (!found) {
        cout << "Tidak ada produk yang ditemukan untuk kategori ini.\n";
    }
    cout << "-------------------------------------------------------------------\n";
}


// --- Cart Management ---

void tambahKeKeranjang(Produk* produk, int jumlah) {
    if (jumlah <= 0) {
        cout << "Jumlah harus lebih dari 0.\n";
        return;
    }
    if (produk->stok < jumlah) {
        cout << "Stok tidak cukup! Sisa stok: " << produk->stok << endl;
        return;
    }

    NodeKeranjang* temp = headKeranjang;
    while(temp != nullptr){
        if(temp->produk->id == produk->id){
            temp->jumlah += jumlah;
            produk->stok -= jumlah;
            cout << "Jumlah produk di keranjang diperbarui.\n";
            return;
        }
        temp = temp->next;
    }

    produk->stok -= jumlah;
    NodeKeranjang* newNode = new NodeKeranjang{produk, jumlah, nullptr};

    if (headKeranjang == nullptr) {
        headKeranjang = newNode;
    } else {
        temp = headKeranjang;
        while (temp->next != nullptr) temp = temp->next;
        temp->next = newNode;
    }
    cout << "Produk berhasil ditambahkan ke keranjang.\n";
}

void kembalikanStokKeranjang() {
    NodeKeranjang* temp = headKeranjang;
    while (temp != nullptr) {
        temp->produk->stok += temp->jumlah;
        temp = temp->next;
    }
}

void hapusKeranjang() {
    while (headKeranjang != nullptr) {
        NodeKeranjang* hapus = headKeranjang;
        headKeranjang = headKeranjang->next;
        delete hapus;
    }
}


// --- Transaction and History ---

double prosesTransaksi() {
    double total = 0;
    cout << "\n--- Struk Transaksi ---\n";
    NodeKeranjang* temp = headKeranjang;
    while (temp != nullptr) {
        double subTotal = static_cast<double>(temp->jumlah) * temp->produk->harga;
        total += subTotal;
        cout << left << setw(20) << temp->produk->nama
             << " x" << setw(3) << temp->jumlah
             << "= Rp " << fixed << setprecision(0) << subTotal << endl;
        temp = temp->next;
    }
    cout << "---------------------------\n";
    cout << "Total Bayar: Rp " << fixed << setprecision(0) << total << endl;
    cout << "Saldo Anda : Rp " << fixed << setprecision(0) << currentUser->saldo << endl;
    return total;
}

void simpanRiwayat(double total) {
    if (headKeranjang == nullptr) return;
    NodeRiwayat* newNode = new NodeRiwayat{total, headKeranjang, nullptr, nullptr};

    if (tailRiwayat == nullptr) {
        headRiwayat = tailRiwayat = newNode;
    } else {
        tailRiwayat->next = newNode;
        newNode->prev = tailRiwayat;
        tailRiwayat = newNode;
    }

    headKeranjang = nullptr;
}

void tampilkanRiwayat() {
    cout << "\n=== Riwayat Transaksi ===\n";
    if(headRiwayat == nullptr){
        cout << "Belum ada riwayat transaksi.\n";
        return;
    }
    NodeRiwayat* temp = headRiwayat;
    int no = 1;

    while (temp != nullptr) {
        cout << "--- Transaksi #" << no++ << " ---\n";
        cout << "Total: Rp " << fixed << setprecision(0) << temp->total << endl;
        cout << "Detail Pembelian:\n";
        NodeKeranjang* detail = temp->detail;
        while (detail != nullptr) {
            cout << "  - " << detail->produk->nama << " (" << detail->produk->kategori << ") x" << detail->jumlah << endl;
            detail = detail->next;
        }
        temp = temp->next;
        cout << "-----------------------\n";
    }
}


// --- Recommendation System (NEW LOGIC) ---

void cetakRekomendasiBerdasarkanKategori(string kategori, int& count, int limit, const map<int, bool>& itemDibeli) {
    if(kategori.empty()) return;
    cout << "\n--- Rekomendasi dari kategori \"" << kategori << "\" ---\n";
    bool found = false;
    for(int i = 0; i < jumlahProduk && count < limit; ++i) {
        // Recommend if category matches AND item was not in the original purchase history
        if(daftarProduk[i].kategori == kategori && itemDibeli.find(daftarProduk[i].id) == itemDibeli.end()) {
            cout << "- " << daftarProduk[i].nama << " (Rp " << daftarProduk[i].harga << ")\n";
            count++;
            found = true;
        }
    }
    if(!found){
        cout << "Tidak ada produk lain di kategori ini.\n";
    }
}

void tampilkanRekomendasi() {
    cout << "\n=== Rekomendasi Produk Untuk Anda ===\n";
    if(tailRiwayat == nullptr) {
        cout << "Anda belum memiliki riwayat transaksi untuk kami berikan rekomendasi.\n";
        return;
    }

    string kategoriTerakhir = "";
    string kategoriKeduaTerakhir = "";
    map<int, bool> itemSudahDibeli; // To avoid recommending items user has already bought

    // Get last purchase category
    NodeRiwayat* lastPurchase = tailRiwayat;
    if(lastPurchase && lastPurchase->detail) {
        kategoriTerakhir = lastPurchase->detail->produk->kategori;
        NodeKeranjang* temp = lastPurchase->detail;
        while(temp) {
            itemSudahDibeli[temp->produk->id] = true;
            temp = temp->next;
        }
    }

    // Get second to last purchase category
    NodeRiwayat* secondLastPurchase = tailRiwayat->prev;
    if(secondLastPurchase && secondLastPurchase->detail) {
        kategoriKeduaTerakhir = secondLastPurchase->detail->produk->kategori;
        NodeKeranjang* temp = secondLastPurchase->detail;
        while(temp) {
            itemSudahDibeli[temp->produk->id] = true;
            temp = temp->next;
        }
    }
    
    int count = 0;
    // 5 recommendations from last purchase category
    cetakRekomendasiBerdasarkanKategori(kategoriTerakhir, count, 5, itemSudahDibeli);

    // 5 recommendations from second to last purchase category (if different)
    if (kategoriKeduaTerakhir != "" && kategoriKeduaTerakhir != kategoriTerakhir) {
       cetakRekomendasiBerdasarkanKategori(kategoriKeduaTerakhir, count, 10, itemSudahDibeli);
    }

    if (count == 0) {
        cout << "Tidak dapat menemukan rekomendasi yang cocok saat ini.\n";
    }
}


// --- User Authentication ---

string inputPassword(const string& prompt) {
    string password;
    char ch;
    cout << prompt;
    while ((ch = _getch()) != '\r') {
        if (ch == '\b') {
            if (!password.empty()) {
                password.pop_back();
                cout << "\b \b";
            }
        } else {
            password += ch;
            cout << '*';
        }
    }
    cout << endl;
    return password;
}

void registerUser() {
    if (jumlahUser >= MAX_USER) {
        cout << "Registrasi penuh, tidak bisa menambah user baru.\n";
        return;
    }
    cout << "\n=== Register ===\n";
    string username, password;

    cout << "Username: ";
    cin >> username;

    for (int i = 0; i < jumlahUser; i++) {
        if (daftarUser[i].username == username) {
            cout << "Username sudah digunakan!\n";
            return;
        }
    }

    password = inputPassword("Password (min 8 karakter): ");
    if (password.length() < 8) {
        cout << "Password harus minimal 8 karakter!\n";
        return;
    }

    daftarUser[jumlahUser++] = {username, password, 0};
    cout << "Akun berhasil dibuat! Silakan login.\n";
}

void login() {
    cout << "\n=== Login ===\n";
    string username, password;

    cout << "Username: ";
    cin >> username;
    password = inputPassword("Password: ");

    for (int i = 0; i < jumlahUser; i++) {
        if (daftarUser[i].username == username && daftarUser[i].password == password) {
            currentUser = &daftarUser[i];
            cout << "\nLogin berhasil! Selamat datang, " << username << ".\n";
            cout << "Saldo Anda saat ini: Rp " << fixed << setprecision(0) << currentUser->saldo << endl;
            return;
        }
    }

    cout << "Username atau password salah.\n";
    currentUser = nullptr;
}

void logout() {
    cout << "Anda telah logout. Terima kasih, " << currentUser->username << ".\n";
    currentUser = nullptr;
    hapusKeranjang();
}

void menuAuth() {
    int pilih;
    do {
        cout << "\n=== SELAMAT DATANG DI E-KLONTONG ===\n";
        cout << "1. Login\n";
        cout << "2. Register\n";
        cout << "0. Keluar Program\n";
        pilih = inputInt("Pilih: ");

        if (pilih == 1) {
            login();
        } else if (pilih == 2) {
            registerUser();
        } else if (pilih == 0) {
            return;
        }
        
    } while (currentUser == nullptr);
}

void topUpSaldo() {
    double nominal = inputDouble("Masukkan jumlah topup: Rp ");
    if (nominal < 0) {
        cout << "Nominal tidak boleh negatif.\n";
        return;
    }
    currentUser->saldo += nominal;
    cout << "Topup berhasil! Saldo sekarang: Rp " << fixed << setprecision(0) << currentUser->saldo << endl;
}

// --- Main Program Loop ---

int main() {
    daftarUser[jumlahUser++] = {"admin", "password123", 1000000};
    daftarUser[jumlahUser++] = {"user", "password123", 50000};
    
    muatProdukDariCSV();
    
    while(true) {
        if(currentUser == nullptr) {
            menuAuth();
            if(currentUser == nullptr) {
                break;
            }
        }

        cout << "\n=== MENU E-KLONTONG ===\n";
        cout << "Saldo: Rp " << fixed << setprecision(0) << currentUser->saldo << endl;
        cout << "------------------------\n";
        cout << "1. Tampilkan Semua Produk\n";
        cout << "2. Lihat Produk Berdasarkan Kategori\n";
        cout << "3. Tambah ke Keranjang\n";
        cout << "4. Checkout\n";
        cout << "5. Lihat Riwayat Transaksi\n";
        cout << "6. Lihat Rekomendasi Produk\n";
        cout << "7. Topup Saldo\n";
        cout << "--- Menu Admin ---\n";
        cout << "8. Tambah Produk Baru\n";
        cout << "------------------\n";
        cout << "0. Logout\n";

        int pilihan = inputInt("Pilih: ");

        if (pilihan == 1) {
            tampilkanProduk();
        } else if (pilihan == 2) {
            tampilkanProdukByKategori();
        } else if (pilihan == 3) {
            tampilkanProduk();
            int id = inputInt("Masukkan ID produk: ");
            Produk* produk = cariProdukById(id);
            if (produk != nullptr) {
                int jumlah = inputInt("Jumlah: ");
                tambahKeKeranjang(produk, jumlah);
            } else {
                cout << "Produk tidak ditemukan.\n";
            }
        } else if (pilihan == 4) {
            if (headKeranjang == nullptr) {
                cout << "Keranjang kosong.\n";
            } else {
                double total = prosesTransaksi();
                if (total > currentUser->saldo) {
                    cout << "Saldo tidak mencukupi untuk melakukan transaksi!\n";
                } else {
                    string konfirmasi;
                    cout << "Lanjutkan checkout? (y/n): ";
                    cin >> konfirmasi;
                    if (konfirmasi == "y" || konfirmasi == "Y") {
                        currentUser->saldo -= total;
                        simpanRiwayat(total);
                        simpanProdukKeCSV();
                        cout << "Checkout berhasil. Sisa saldo: Rp " << fixed << setprecision(0) << currentUser->saldo << endl;
                    } else {
                        kembalikanStokKeranjang();
                        cout << "Checkout dibatalkan. Stok produk telah dikembalikan.\n";
                    }
                }
            }
        } else if (pilihan == 5) {
            tampilkanRiwayat();
        } else if (pilihan == 6) {
             tampilkanRekomendasi();
        } else if (pilihan == 7) {
            topUpSaldo();
        } else if (pilihan == 8) {
            inputProdukBaru();
        } else if (pilihan == 0) {
            logout();
        }
    }

    cout << "\nTerima kasih telah menggunakan aplikasi E-Klontong.\n";
    return 0;
}