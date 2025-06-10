    #include <iostream>
    #include <iomanip>
    #include <string>
    #include <conio.h>
    #include <fstream>
    #include <chrono> 
    using namespace std;

    const int MAX_PRODUK = 100;
    const int MAX_USER = 100;

    struct Produk {
        int id;
        string nama;
        int stok;
        int harga;
        int hargaFlashSale;
    time_t flashSaleMulai;
    time_t flashSaleSelesai;
    };

    struct NodeKeranjang {
        Produk* produk;
        int jumlah;
        NodeKeranjang* next;
    };

    struct NodeRiwayat {
        double total;
        NodeKeranjang* detail;
        NodeRiwayat* next;
        NodeRiwayat* prev;
    };

    struct User {
        string username;
        string password;
        double saldo = 0;
    };
    
    bool sedangFlashSale(const Produk& p) {
        if (p.hargaFlashSale <= 0 || p.flashSaleMulai == 0 || p.flashSaleSelesai == 0) {
            return false; // Jika tidak ada data flash sale, pasti tidak sedang flash sale
        }
        // Ambil waktu saat ini
        time_t sekarang = chrono::system_clock::to_time_t(chrono::system_clock::now());
        
        // Cek apakah waktu sekarang berada dalam rentang flash sale
        return sekarang >= p.flashSaleMulai && sekarang <= p.flashSaleSelesai;
    }
    
    int getHargaSekarang(const Produk& p) {
        if (sedangFlashSale(p)) {
            return p.hargaFlashSale;
        }
        return p.harga;
    }

    Produk daftarProduk[MAX_PRODUK];
    int jumlahProduk = 0;

    NodeKeranjang* headKeranjang = nullptr;
    NodeRiwayat* headRiwayat = nullptr;
    NodeRiwayat* tailRiwayat = nullptr;

    User daftarUser[MAX_USER];
    int jumlahUser = 0;
    User* currentUser = nullptr;

    void clearInput() {
        cin.clear();
        cin.ignore(10000, '\n');
    }

    int inputInt(string prompt) {
        int value;
        while (true) {
            cout << prompt;
            cin >> value;
            if (!cin.fail()) {
                cin.ignore(10000, '\n');
                break;
            }
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
            if (!cin.fail()) {
                cin.ignore(10000, '\n');
                break;
            }

            cout << "Input harus berupa angka desimal (mis. 12000.50)!\n";
            clearInput();
        }
        return value;
    }

    void simpanProdukKeCSV() {
        ofstream file("produk.csv");
        if (!file.is_open()) {
            cout << "Gagal membuka file untuk menyimpan produk!\n";
            return;
        }
    
        file << "ID,Nama,Stok,Harga,HargaFlashSale,FlashSaleMulai,FlashSaleSelesai\n";
        for (int i = 0; i < jumlahProduk; i++) {
            file << daftarProduk[i].id << ','
                 << daftarProduk[i].nama << ','
                 << daftarProduk[i].stok << ','
                 << daftarProduk[i].harga << ','
                 << daftarProduk[i].hargaFlashSale << ','
                 << daftarProduk[i].flashSaleMulai << ','
                 << daftarProduk[i].flashSaleSelesai << '\n';
        }
    
        file.close();
    }
    
    void muatProdukDariCSV() {
        ifstream file("produk.csv");
        if (!file.is_open()) {
            return;
        }
    
        string header;
        getline(file, header);
    
        jumlahProduk = 0;
        string line;
        while (getline(file, line) && jumlahProduk < MAX_PRODUK) {
            if (line.empty()) continue;
    
            stringstream ss(line);
            string item;
            Produk p = {0, "", 0, 0, 0, 0, 0};

            getline(ss, item, ','); p.id = stoi(item);
            getline(ss, p.nama, ',');
            getline(ss, item, ','); p.stok = stoi(item);
            getline(ss, item, ','); p.harga = stoi(item);
            getline(ss, item, ','); p.hargaFlashSale = stoi(item);
            getline(ss, item, ','); p.flashSaleMulai = stoll(item);
            getline(ss, item, ','); p.flashSaleSelesai = stoll(item);
    
            if (!ss.fail()) {
                daftarProduk[jumlahProduk++] = p;
            }
        }
    
        file.close();
    }


    void tambahProduk(int id, string nama, int stok, int harga) {
        daftarProduk[jumlahProduk++] = {id, nama, stok, harga, 0, 0, 0}; 
        simpanProdukKeCSV();
    }

    void tampilkanProduk() {
    if (jumlahProduk == 0) {
        cout << "\nBelum ada produk yang tersedia.\n";
        return;
    }
    cout << "\nDaftar Produk:\n";
    cout << "------------------------------------------------------------------\n";
    for (int i = 0; i < jumlahProduk; i++) {
        cout << "[" << daftarProduk[i].id << "] " << daftarProduk[i].nama
             << " | Stok: " << daftarProduk[i].stok;
        
        if (sedangFlashSale(daftarProduk[i])) {
            cout << " | \x1B[91mFLASH SALE!\033[0m" 
                 << " | Harga: \x1B[9m" << "Rp " << daftarProduk[i].harga << "\033[0m" 
                 << " -> Rp " << getHargaSekarang(daftarProduk[i]) << endl;
        } else {
            cout << " | Harga: Rp " << fixed << setprecision(0) << daftarProduk[i].harga << endl;
        }
    }
    cout << "------------------------------------------------------------------\n";
}

    Produk* cariProdukById(int id) {
        for (int i = 0; i < jumlahProduk; i++) {
            if (daftarProduk[i].id == id) {
                return &daftarProduk[i];
            }
        }
        return nullptr;
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
        if (stok < 0) {
            cout << "Stok tidak boleh kurang dari 0!\n";
            return;
        }

        int harga = inputInt("Harga: ");
        if (harga < 0) {
            cout << "Harga tidak boleh kurang dari 0!\n";
            return;
        }

        tambahProduk(id, nama, stok, harga);
        cout << "Produk berhasil ditambahkan!\n";

    }

    void tambahKeKeranjang(Produk* produk, int jumlah) {
        if (produk->stok < jumlah) {
            cout << "Stok tidak cukup!\n";
            return;
        }

        produk->stok -= jumlah;
        NodeKeranjang* newNode = new NodeKeranjang{produk, jumlah, nullptr};

        if (headKeranjang == nullptr) {
            headKeranjang = newNode;
        } else {
            NodeKeranjang* temp = headKeranjang;
            while (temp->next != nullptr)
                temp = temp->next;
            temp->next = newNode;
        }

        cout << "Produk ditambahkan ke keranjang.\n";
    }

    int prosesTransaksi() {
        int total = 0;
        cout << "\nStruk Transaksi:\n";
        cout << "---------------------------\n";
    
        NodeKeranjang* temp = headKeranjang;
        while (temp != nullptr) {
            int hargaSatuan = getHargaSekarang(*(temp->produk));
            int subTotal = temp->jumlah * hargaSatuan;
    
            total += subTotal;
            cout << temp->produk->nama << " x" << temp->jumlah << " = Rp " << subTotal << endl;
            temp = temp->next;
        }
        cout << "---------------------------\n";
        cout << "Total Bayar: Rp " << total << endl;
        cout << "Saldo Anda: Rp " << currentUser->saldo << endl;
    
        return total;
    }

    void hapusKeranjang() {
        while (headKeranjang != nullptr) {
            NodeKeranjang* hapus = headKeranjang;
            headKeranjang = headKeranjang->next;
            delete hapus;
        }
    }

    void simpanRiwayat(int total) {
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
        NodeRiwayat* temp = headRiwayat;
        int no = 1;

        while (temp != nullptr) {
            cout << "Transaksi #" << no++ << " - Total: Rp " << temp->total << endl;
            NodeKeranjang* detail = temp->detail;
            while (detail != nullptr) {
                cout << "  - " << detail->produk->nama << " x" << detail->jumlah << endl;
                detail = detail->next;
            }
            temp = temp->next;
            cout << "-------------------\n";
        }
    }

    string inputPassword(const string& prompt) {
        string password;
        char ch;
        cout << prompt;
        while ((ch = _getch()) != '\r') {  // '\r' adalah Enter
            if (ch == '\b') { // Jika backspace
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
        cout << "Akun berhasil dibuat!\n";
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
                cout << "Login berhasil! Selamat datang, " << username << ".\n";
                return;
            }
        }

        cout << "Username atau password salah.\n";
    }

    void menuAuth() {
        int pilih;
        do {
            cout << "\n=== MENU AUTH ===\n";
            cout << "1. Login\n";
            cout << "2. Register\n";
            cout << "0. Keluar\n";
            pilih = inputInt("Pilih: ");

            if (pilih == 1) {
                login();
            } else if (pilih == 2) {
                registerUser();
            }

        } while (currentUser == nullptr && pilih != 0);
    }

    void topUpSaldo() {
        double nominal = inputDouble("Masukkan jumlah topup: Rp ");
        if (nominal < 0) {
            cout << "Nominal tidak boleh negatif.\n";
            return;
        }
        currentUser->saldo += nominal;
        cout << "Topup berhasil! Saldo sekarang: Rp " << currentUser->saldo << endl;
    }

    void aturFlashSale() {
        cout << "\n=== Atur Flash Sale ===\n";
        tampilkanProduk();
        int id = inputInt("Masukkan ID produk yang akan di-flash sale: ");
    
        Produk* produk = cariProdukById(id);
        if (produk == nullptr) {
            cout << "Produk tidak ditemukan!\n";
            return;
        }
    
        int hargaFS = inputInt("Masukkan harga flash sale: ");
        if (hargaFS >= produk->harga) {
            cout << "Harga flash sale harus lebih rendah dari harga normal!\n";
            return;
        }
    
        int durasiJam = inputInt("Masukkan durasi flash sale (dalam jam, mis. 24): ");

        auto sekarang = chrono::system_clock::now();
        auto waktuSelesai = sekarang + chrono::hours(durasiJam);
    
        produk->hargaFlashSale = hargaFS;
        produk->flashSaleMulai = chrono::system_clock::to_time_t(sekarang);
        produk->flashSaleSelesai = chrono::system_clock::to_time_t(waktuSelesai);
    
        simpanProdukKeCSV();
        cout << "Flash sale untuk produk '" << produk->nama << "' berhasil diatur selama " << durasiJam << " jam!\n";
    }

    void clearScreen() {
        #ifdef _WIN32
            system("cls");
        #else
            system("clear");
        #endif
    }

    int main() {
        muatProdukDariCSV();

        menuAuth();

        if (currentUser != nullptr) {
            int pilihan;
            do {
                
                cout << "\n=== MENU E-Klontong ===\n";
                cout << "1. Tampilkan Produk\n";
                cout << "2. Tambah ke Keranjang\n";
                cout << "3. Checkout\n";
                cout << "4. Lihat Riwayat\n";
                cout << "5. Tambah Produk Baru\n";
                cout << "6. Topup Saldo\n";
                cout << "7. Atur Flash Sale\n";
                cout << "0. Keluar\n";

                pilihan = inputInt("Pilih: ");

                if (pilihan == 1) {
                    clearScreen();
                    tampilkanProduk();
                } else if (pilihan == 2) {
                    tampilkanProduk();
                    int id = inputInt("Masukkan ID produk: ");
                    int jumlah = inputInt("Jumlah: ");

                    Produk* produk = cariProdukById(id);
                    if (produk != nullptr) {
                        tambahKeKeranjang(produk, jumlah);
                    } else {
                        cout << "Produk tidak ditemukan.\n";
                    }
                } else if (pilihan == 3) {
                    clearScreen();
                    if (headKeranjang == nullptr) {
                        cout << "Keranjang kosong.\n";
                    } else {
                        int total = prosesTransaksi();
                        if (total > currentUser->saldo) {
                            cout << "Saldo tidak mencukupi untuk melakukan transaksi!\n";
                        } else {
                            string konfirmasi;
                            cout << "Lanjutkan checkout? (y/n): ";
                            cin >> konfirmasi;
                            if (konfirmasi == "y" || konfirmasi == "Y") {
                                currentUser->saldo -= total;
                                simpanRiwayat(total);
                                hapusKeranjang();
                                cout << "Checkout berhasil. Sisa saldo: Rp " << currentUser->saldo << endl;
                            } else {
                                cout << "Checkout dibatalkan.\n";
                            }
                        }
                    }
                } else if (pilihan == 4) {
                    clearScreen();
                    tampilkanRiwayat();
                } else if (pilihan == 5) {
                    clearScreen();
                    inputProdukBaru();
                } else if (pilihan == 6) {
                    clearScreen();
                    topUpSaldo();
                } else if (pilihan == 7) {
                    clearScreen();
                    aturFlashSale();
                }

            } while (pilihan != 0);
        }

        cout << "Terima kasih telah menggunakan aplikasi E-Klontong.\n";
        return 0;
    }
