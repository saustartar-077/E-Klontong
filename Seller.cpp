#include <iostream>
#include <iomanip>
#include <string>
#include <conio.h> // Non-standard, specific to Windows
#include <fstream>
#include <vector>
#include <map>
#include <sstream> // Added missing header
#include <stack>   // NEW: Include for std::stack

using namespace std;

// Constants for array sizes
const int MAX_PRODUK = 100;
const int MAX_USER = 100;

// Predefined list of categories
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
    string kategori;
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
    NodeKeranjang* detail;
    NodeRiwayat* next;
    NodeRiwayat* prev;
};

// MODIFIED: Struct definition for a User now includes a role
struct User {
    string username;
    string password;
    double saldo = 0;
    string role; // Can be "Penjual" or "Pembeli"
};

// Struct to store cart actions for the Undo feature
struct AksiKeranjang {
    Produk* produk;
    int jumlah;
};


// --- Global Variables ---
Produk daftarProduk[MAX_PRODUK];
int jumlahProduk = 0;
NodeKeranjang* headKeranjang = nullptr;
NodeRiwayat* headRiwayat = nullptr;
NodeRiwayat* tailRiwayat = nullptr;
User daftarUser[MAX_USER];
int jumlahUser = 0;
User* currentUser = nullptr;
stack<AksiKeranjang> undoStack;

// NEW: Graph data structure for recommendations
map<int, map<int, int>> grafRekomendasi;


// --- Forward Declarations ---
void simpanUserKeCSV();

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

void simpanProdukKeCSV() {
    ofstream file("produk.csv");
    if (!file.is_open()) {
        cout << "Gagal membuka file produk!\n";
        return;
    }
    file << "ID,Nama,Stok,Harga,Kategori\n";
    for (int i = 0; i < jumlahProduk; i++) {
        file << daftarProduk[i].id << ",\""
             << daftarProduk[i].nama << "\","
             << daftarProduk[i].stok << ","
             << daftarProduk[i].harga << ",\""
             << daftarProduk[i].kategori << "\"\n";
    }
    file.close();
}

void muatProdukDariCSV() {
    ifstream file("produk.csv");
    if (!file.is_open()) return;

    string line;
    getline(file, line);

    jumlahProduk = 0;
    while (getline(file, line) && jumlahProduk < MAX_PRODUK) {
        if (line.empty()) continue;

        stringstream ss(line);
        string id_str, nama, stok_str, harga_str, kategori;

        getline(ss, id_str, ',');
        ss.ignore(); getline(ss, nama, '"'); ss.ignore();
        getline(ss, stok_str, ',');
        getline(ss, harga_str, ',');
        ss.ignore(); getline(ss, kategori, '"');

        try {
            Produk& p = daftarProduk[jumlahProduk];
            p.id = stoi(id_str);
            p.nama = nama;
            p.stok = stoi(stok_str);
            p.harga = stoi(harga_str);
            p.kategori = kategori;
            jumlahProduk++;
        } catch (const std::invalid_argument& e) {}
    }
    file.close();
}

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
    int harga = inputInt("Harga: ");
    string kategori = pilihKategori();
    
    daftarProduk[jumlahProduk] = {id, nama, stok, harga, kategori};
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
    cout << "-------------------------------------------------------------------------\n";
    // FIXED: Adjusted column widths for better alignment
    cout << left << setw(5) << "ID" << setw(25) << "Nama" << setw(30) << "Kategori" << setw(10) << "Stok" << "Harga\n";
    cout << "-------------------------------------------------------------------------\n";
    for (int i = 0; i < jumlahProduk; i++) {
        cout << left << setw(5) << daftarProduk[i].id
             << setw(25) << daftarProduk[i].nama
             << setw(30) << daftarProduk[i].kategori
             << setw(10) << daftarProduk[i].stok
             << "Rp " << fixed << setprecision(0) << daftarProduk[i].harga << endl;
    }
    cout << "-------------------------------------------------------------------------\n";
}

void tampilkanProdukByKategori() {
    cout << "\n=== Tampilkan Produk Berdasarkan Kategori ===\n";
    string kategoriDipilih = pilihKategori();
    
    cout << "\nMenampilkan produk untuk kategori: " << kategoriDipilih << endl;
    cout << "-------------------------------------------------------------------------\n";
    // FIXED: Adjusted column widths for better alignment
    cout << left << setw(5) << "ID" << setw(25) << "Nama" << setw(30) << "Kategori" << setw(10) << "Stok" << "Harga\n";
    cout << "-------------------------------------------------------------------------\n";

    bool found = false;
    for(int i = 0; i < jumlahProduk; ++i) {
        if(daftarProduk[i].kategori == kategoriDipilih) {
            cout << left << setw(5) << daftarProduk[i].id
                 << setw(25) << daftarProduk[i].nama
                 << setw(30) << daftarProduk[i].kategori
                 << setw(10) << daftarProduk[i].stok
                 << "Rp " << fixed << setprecision(0) << daftarProduk[i].harga << endl;
            found = true;
        }
    }

    if (!found) {
        cout << "Tidak ada produk yang ditemukan untuk kategori ini.\n";
    }
    cout << "-------------------------------------------------------------------------\n";
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

    undoStack.push({produk, jumlah});
    produk->stok -= jumlah;

    NodeKeranjang* temp = headKeranjang;
    while(temp != nullptr){
        if(temp->produk->id == produk->id){
            temp->jumlah += jumlah;
            cout << "Jumlah produk di keranjang diperbarui.\n";
            return;
        }
        temp = temp->next;
    }

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

void undoAksiTerakhir() {
    if (undoStack.empty()) {
        cout << "Tidak ada aksi yang bisa di-undo.\n";
        return;
    }

    AksiKeranjang aksi = undoStack.top();
    undoStack.pop();

    NodeKeranjang* temp = headKeranjang;
    NodeKeranjang* prev = nullptr;
    while (temp != nullptr) {
        if (temp->produk->id == aksi.produk->id) {
            temp->produk->stok += aksi.jumlah;
            temp->jumlah -= aksi.jumlah;
            cout << "Aksi terakhir (menambah " << aksi.jumlah << " " << aksi.produk->nama << ") telah dibatalkan.\n";
            if (temp->jumlah <= 0) {
                if (prev == nullptr) {
                    headKeranjang = temp->next;
                } else {
                    prev->next = temp->next;
                }
                delete temp;
            }
            return;
        }
        prev = temp;
        temp = temp->next;
    }
}


void hapusKeranjang() {
    while (headKeranjang != nullptr) {
        NodeKeranjang* hapus = headKeranjang;
        headKeranjang = headKeranjang->next;
        delete hapus;
    }
    while(!undoStack.empty()) undoStack.pop();
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

// NEW: Function to update the recommendation graph after a purchase
void updateGrafRekomendasi(NodeKeranjang* keranjang) {
    if (keranjang == nullptr) return;

    vector<int> idProduk;
    NodeKeranjang* temp = keranjang;
    while (temp != nullptr) {
        idProduk.push_back(temp->produk->id);
        temp = temp->next;
    }
    
    // For every pair of items in the cart, increment their co-purchase score
    for (size_t i = 0; i < idProduk.size(); i++) {
        for (size_t j = i + 1; j < idProduk.size(); j++) {
            grafRekomendasi[idProduk[i]][idProduk[j]]++;
            grafRekomendasi[idProduk[j]][idProduk[i]]++;
        }
    }
}


void simpanRiwayat(double total) {
    if (headKeranjang == nullptr) return;

    // The graph must be updated BEFORE the cart is moved to history
    updateGrafRekomendasi(headKeranjang);

    NodeRiwayat* newNode = new NodeRiwayat{total, headKeranjang, nullptr, nullptr};

    if (tailRiwayat == nullptr) {
        headRiwayat = tailRiwayat = newNode;
    } else {
        tailRiwayat->next = newNode;
        newNode->prev = tailRiwayat;
        tailRiwayat = newNode;
    }

    headKeranjang = nullptr;
    while(!undoStack.empty()) undoStack.pop();
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


// --- Recommendation System (Graph-based) ---

void tampilkanRekomendasi() {
    cout << "\n=== Rekomendasi Produk Berdasarkan Pembelian Bersama ===\n";
    int idProduk = inputInt("Masukkan ID produk untuk melihat rekomendasi: ");
    
    Produk* p = cariProdukById(idProduk);
    if (!p) {
        cout << "Produk dengan ID " << idProduk << " tidak ditemukan.\n";
        return;
    }

    cout << "\nProduk yang sering dibeli bersama \"" << p->nama << "\":\n";

    if (grafRekomendasi.find(idProduk) == grafRekomendasi.end() || grafRekomendasi[idProduk].empty()) {
        cout << "Belum ada data rekomendasi untuk produk ini.\n";
        return;
    }

    // The map automatically stores related products for the given ID
    for (auto const& [relatedId, score] : grafRekomendasi[idProduk]) {
        Produk* recProduk = cariProdukById(relatedId);
        if (recProduk) {
            cout << "- " << recProduk->nama << " (Skor: " << score << ")\n";
        }
    }
}


// --- User Authentication & Profile ---

void simpanUserKeCSV() {
    ofstream file("users.csv");
    if (!file.is_open()) {
        cout << "Gagal membuka file users!\n";
        return;
    }
    file << "Username,Password,Saldo,Role\n"; // Added Role column
    for (int i = 0; i < jumlahUser; i++) {
        file << daftarUser[i].username << ","
             << daftarUser[i].password << ","
             << fixed << setprecision(2) << daftarUser[i].saldo << ","
             << daftarUser[i].role << "\n";
    }
    file.close();
}

void muatUserDariCSV() {
    ifstream file("users.csv");
    if (!file.is_open()) return;

    string line;
    getline(file, line);

    jumlahUser = 0;
    while (getline(file, line) && jumlahUser < MAX_USER) {
        if (line.empty()) continue;
        
        stringstream ss(line);
        string username, password, saldo_str, role;
        
        getline(ss, username, ',');
        getline(ss, password, ',');
        getline(ss, saldo_str, ',');
        getline(ss, role); // Read role
        
        try {
            daftarUser[jumlahUser] = {username, password, stod(saldo_str), role};
            jumlahUser++;
        } catch (const std::invalid_argument& e) {}
    }
    file.close();
}


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
        cout << "Registrasi penuh.\n";
        return;
    }
    cout << "\n=== Register ===\n";
    string username, password, role;

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

    // MODIFIED: Role selection during registration
    cout << "\nDaftar sebagai:\n1. Penjual\n2. Pembeli\n";
    int roleChoice;
    do {
        roleChoice = inputInt("Pilih peran (1-2): ");
        if(roleChoice == 1) role = "Penjual";
        else if (roleChoice == 2) role = "Pembeli";
        else cout << "Pilihan tidak valid.\n";
    } while (roleChoice != 1 && roleChoice != 2);


    daftarUser[jumlahUser++] = {username, password, 0, role};
    simpanUserKeCSV();
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
    simpanUserKeCSV();
    cout << "Topup berhasil! Saldo sekarang: Rp " << fixed << setprecision(0) << currentUser->saldo << endl;
}

// NEW: Functions for profile management
void gantiUsername() {
    cout << "\n--- Ganti Username ---\n";
    cout << "Masukkan username baru: ";
    string newUsername;
    cin >> newUsername;

    // Check for duplicates
    for(int i = 0; i < jumlahUser; ++i) {
        // Make sure we are not comparing the user to themselves
        if(&daftarUser[i] != currentUser && daftarUser[i].username == newUsername) {
            cout << "Username sudah digunakan. Silakan pilih yang lain.\n";
            return;
        }
    }

    currentUser->username = newUsername;
    simpanUserKeCSV();
    cout << "Username berhasil diubah menjadi: " << newUsername << endl;
}

void gantiPassword() {
    cout << "\n--- Ganti Password ---\n";
    string oldPassword = inputPassword("Masukkan password lama Anda: ");

    if (oldPassword != currentUser->password) {
        cout << "Password lama salah!\n";
        return;
    }

    string newPassword = inputPassword("Masukkan password baru (min 8 karakter): ");
    if(newPassword.length() < 8) {
        cout << "Password baru harus minimal 8 karakter!\n";
        return;
    }
    
    string confirmPassword = inputPassword("Konfirmasi password baru: ");
    if(newPassword != confirmPassword) {
        cout << "Konfirmasi password tidak cocok!\n";
        return;
    }

    currentUser->password = newPassword;
    simpanUserKeCSV();
    cout << "Password berhasil diubah.\n";
}


void menuProfil() {
    int pilihan;
    do {
        cout << "\n=== PROFIL ANDA ===\n";
        cout << "Username: " << currentUser->username << endl;
        cout << "Peran   : " << currentUser->role << endl;
        cout << "Saldo   : Rp " << fixed << setprecision(0) << currentUser->saldo << endl;
        cout << "---------------------\n";
        cout << "1. Ganti Username\n";
        cout << "2. Ganti Password\n";
        cout << "0. Kembali ke Menu Utama\n";
        pilihan = inputInt("Pilih: ");

        if (pilihan == 1) {
            gantiUsername();
        } else if (pilihan == 2) {
            gantiPassword();
        }

    } while (pilihan != 0);
}


// --- Main Program Loop ---

void mainLoop() {
    while(currentUser != nullptr) {
        // SHARED ACTION LOGIC
        auto handleBuyerActions = [&](int pilihan) {
            if (pilihan == 1) tampilkanProduk();
            else if (pilihan == 2) tampilkanProdukByKategori();
            else if (pilihan == 3) {
                tampilkanProduk();
                int id = inputInt("Masukkan ID produk: ");
                Produk* produk = cariProdukById(id);
                if (produk != nullptr) {
                    int jumlah = inputInt("Jumlah: ");
                    tambahKeKeranjang(produk, jumlah);
                } else {
                    cout << "Produk tidak ditemukan.\n";
                }
            } 
            else if (pilihan == 4) undoAksiTerakhir();
            else if (pilihan == 5) {
                if (headKeranjang == nullptr) {
                    cout << "Keranjang kosong.\n";
                } else {
                    double total = prosesTransaksi();
                    if (total > currentUser->saldo) {
                        cout << "Saldo tidak mencukupi!\n";
                    } else {
                        string konfirmasi;
                        cout << "Lanjutkan checkout? (y/n): ";
                        cin >> konfirmasi;
                        if (konfirmasi == "y" || konfirmasi == "Y") {
                            currentUser->saldo -= total;
                            simpanRiwayat(total); // This now also updates the graph
                            simpanProdukKeCSV();
                            simpanUserKeCSV();
                            cout << "Checkout berhasil. Sisa saldo: Rp " << fixed << setprecision(0) << currentUser->saldo << endl;
                        } else {
                            kembalikanStokKeranjang();
                            cout << "Checkout dibatalkan. Stok produk telah dikembalikan.\n";
                        }
                    }
                }
            }
            else if (pilihan == 6) tampilkanRiwayat();
            else if (pilihan == 7) tampilkanRekomendasi();
            else if (pilihan == 8) topUpSaldo();
        };

        if(currentUser->role == "Penjual") {
            cout << "\n=== MENU PENJUAL ===\n";
            cout << "Saldo: Rp " << fixed << setprecision(0) << currentUser->saldo << endl;
            cout << "------------------------\n";
            cout << "1. Tampilkan Semua Produk\n";
            cout << "2. Lihat Produk Berdasarkan Kategori\n";
            cout << "3. Tambah ke Keranjang\n";
            cout << "4. Undo Tambah ke Keranjang\n";
            cout << "5. Checkout\n";
            cout << "6. Lihat Riwayat Transaksi\n";
            cout << "7. Lihat Rekomendasi Produk\n";
            cout << "8. Topup Saldo\n";
            cout << "9. Lihat Profil\n";
            cout << "--- Fitur Penjual ---\n";
            cout << "10. Tambah Produk Baru\n";
            cout << "---------------------\n";
            cout << "0. Logout\n";

            int pilihan = inputInt("Pilih: ");
            if (pilihan >= 1 && pilihan <= 8) {
                handleBuyerActions(pilihan);
            } else if (pilihan == 9) {
                menuProfil();
            } else if (pilihan == 10) {
                inputProdukBaru();
            } else if (pilihan == 0) {
                logout();
                break;
            }

        } else { // Role is "Pembeli"
            cout << "\n=== MENU PEMBELI ===\n";
            cout << "Saldo: Rp " << fixed << setprecision(0) << currentUser->saldo << endl;
            cout << "------------------------\n";
            cout << "1. Tampilkan Semua Produk\n";
            cout << "2. Lihat Produk Berdasarkan Kategori\n";
            cout << "3. Tambah ke Keranjang\n";
            cout << "4. Undo Tambah ke Keranjang\n";
            cout << "5. Checkout\n";
            cout << "6. Lihat Riwayat Transaksi\n";
            cout << "7. Lihat Rekomendasi Produk\n";
            cout << "8. Topup Saldo\n";
            cout << "9. Lihat Profil\n";
            cout << "0. Logout\n";

            int pilihan = inputInt("Pilih: ");
            if (pilihan >= 1 && pilihan <= 8) {
                handleBuyerActions(pilihan);
            } else if (pilihan == 9) {
                menuProfil();
            } else if (pilihan == 0) {
                logout();
                break;
            }
        }
    }
}


int main() {
    muatProdukDariCSV();
    muatUserDariCSV();
    
    while(true) {
        if(currentUser == nullptr) {
            menuAuth();
            if(currentUser == nullptr) {
                break; // User chose to exit program from auth menu
            }
        }
        mainLoop(); // Enter the main application loop based on role
    }

    cout << "\nTerima kasih telah menggunakan aplikasi E-Klontong.\n";
    return 0;
}
