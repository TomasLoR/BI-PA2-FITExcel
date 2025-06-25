## Přehled projektu

Tento projekt je aplikace v jazyce C++ pro zpracování a vyhodnocování matematických výrazů. Je postaven pomocí CMake a obsahuje modulární komponenty pro parsování a vyhodnocování výrazů.

### Klíčové komponenty

1. **`all_in_one.cpp`**:
   - Hlavní implementační soubor obsahující jádro logiky aplikace.
   - Zahrnuje standardní knihovny C++ a implementuje funkce pro zpracování výrazů.

2. **`expression.h`**:
   - Definuje abstraktní třídu `CExprBuilder` s virtuálními metodami pro matematické operace, jako je sčítání, odčítání, násobení, dělení, negace a porovnání.
   - Poskytuje modulární návrh pro rozšíření a implementaci vlastních builderů výrazů.

3. **`CMakeLists.txt`**:
   - Konfiguruje build systém pomocí CMake.
   - Specifikuje použití C++20, kompilátorové příznaky pro varování, ladění a sanitizaci adres.
   - Linkuje aplikaci s knihovnou `expression_parser` umístěnou v adresáři `x86_64-linux-gnu`.

### Jak sestavit a spustit

1. Pomocí CMake:
   ```bash
   cmake -S . -B build
   cmake --build build
   ./build/semester
   ```

2. Přímé použití g++:
   ```bash
   g++ -std=c++20 -Wall -pedantic -g -o FITexcel -fsanitize=address all_in_one.cpp -L./x86_64-linux-gnu -lexpression_parser
   ./FITexcel
   ```

### Další soubory a adresáře

- **Adresáře pro různé platformy**:
  - `arm64-darwin23-clang/`, `arm64-darwin23-g++/`, `x86_64-darwin23-clang/`, `x86_64-darwin23-g++/`, `x86_64-linux-gnu/`, `i686-w64-mingw32/`:
    - Obsahují knihovny a konfigurace specifické pro různé platformy a architektury.

- **`cmake-build-debug/`**:
  - Adresář generovaný během procesu sestavení pomocí CMake.

### Obsah hlavních souborů

1. **`all_in_one.cpp`**:
   - Obsahuje hlavní implementaci aplikace.
   - Používá standardní knihovny C++ jako `<iostream>`, `<vector>`, `<map>` a další pro zpracování matematických výrazů.

2. **`expression.h`**:
   - Definuje abstraktní třídu `CExprBuilder` s virtuálními metodami pro operace jako sčítání, odčítání, násobení, dělení, mocnění, negace a porovnání.
   - Slouží jako základ pro rozšíření a implementaci vlastních builderů výrazů.

3. **`CMakeLists.txt`**:
   - Nastavuje minimální požadovanou verzi CMake (3.24) a standard C++ (C++20).
   - Přidává kompilátorové příznaky pro varování, ladění a sanitizaci adres.
   - Linkuje aplikaci s knihovnou `expression_parser` umístěnou v adresáři `x86_64-linux-gnu`.
