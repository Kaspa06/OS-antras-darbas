#include <windows.h>
#include <iostream>
#include <omp.h>

typedef void(*VoidFunc)();
typedef void(*PingFunc)();
typedef void(*CalcFunc)(double, double, double, double);
typedef void(*MergeFunc)(double);

double GautiDxPagalStudID(int studentID, int tikslumoLygis) {
    switch (tikslumoLygis) {
    case 1: return studentID / 1e7;   // labai zemas
    case 2: return studentID / 1e8;   // zemas
    case 3: return studentID / 1e9;   // aukstas (numatytasis)
    case 4: return studentID / 1e10;   // labai aukstas
    case 5: return studentID / 1e12;  // ekstremalus
    default: return studentID / 1e8;
    }
}

int main() {
    HMODULE dll = LoadLibrary(TEXT("Dll.dll"));

    if (!dll) {
        std::cerr << "Nepavyko ikelti DLL." << std::endl;
        return 1;
    }

    auto StartTimer = (VoidFunc)GetProcAddress(dll, "StartTimer");
    auto EndTimer = (VoidFunc)GetProcAddress(dll, "EndTimer");

    auto Ping = (PingFunc)GetProcAddress(dll, "PingWithTempFile");
    auto CreateFolders = (VoidFunc)GetProcAddress(dll, "CreateCustomFolderStructure");
    auto Calc = (CalcFunc)GetProcAddress(dll, "ComputeTschirnhausen");
    auto Merge = (MergeFunc)GetProcAddress(dll, "MergeResults");
    auto Delete = (VoidFunc)GetProcAddress(dll, "DeleteGeneratedFolders");

    if (!StartTimer || !EndTimer || !Ping || !CreateFolders || !Calc || !Merge || !Delete) {
        std::cerr << "Nepavyko rasti vienos ar daugiau funkciju DLL faile." << std::endl;
        FreeLibrary(dll);
        return 2;
    }

    StartTimer();

    Ping();
    CreateFolders();

    int studentID = 2314012;
    double x0 = -(studentID % 19);
    double xn = studentID % 25;

    int tikslumoLygis;
    std::cout << "\nPasirinkite dx tikslumo lygi:\n";
    std::cout << "1 - Labai zemas tikslumas   (dx ~ " << studentID / 1e7 << ")\n";
    std::cout << "2 - Zemas tikslumas         (dx ~ " << studentID / 1e8 << ")\n";
    std::cout << "3 - Aukstas tikslumas       (dx ~ " << studentID / 1e9 << ")\n";
    std::cout << "4 - Labai aukstas tikslumas (dx ~ " << studentID / 1e10 << ")\n";
    std::cout << "5 - Ekstremalus tikslumas   (dx ~ " << studentID / 1e12 << ") [numatytasis eMokymai]\n";
    std::cout << "Iveskite pasirinkima (1-5): ";
    std::cin >> tikslumoLygis;

    if (tikslumoLygis < 1 || tikslumoLygis > 5) {
        std::cout << "Netinkamas pasirinkimas. Naudojamas numatytasis (3 lygis).\n";
        tikslumoLygis = 3;
    }

    double dx = GautiDxPagalStudID(studentID, tikslumoLygis);
    std::cout << "Pasirinktas dx: " << dx << "\n\n";

    for (int F = -2; F <= 2; ++F) {
        Calc(F, x0, xn, dx);
        Merge(F);
    }

    char deleteChoice;
    std::cout << "Ar norite istrinti visus sukurtus aplankus ir failus? (Y/N): ";
    std::cin >> deleteChoice;

    if (deleteChoice == 'Y' || deleteChoice == 'y') {
        Delete();
        std::cout << "Visi duomenys istrinti." << std::endl;
    }
    else {
        std::cout << "Duomenys palikti." << std::endl;
    }

    EndTimer();
    FreeLibrary(dll);

    char closeChoice;
    std::cout << "Ar norite uzdaryti programa? (Y/N): ";
    std::cin >> deleteChoice;
    if (deleteChoice == 'Y' || deleteChoice == 'y') {
        return 0;
        std::cout << "Programa baigia darba." << std::endl;
    }
    else {
        std::cout << "Programa baigia darba." << std::endl;
    }
}
