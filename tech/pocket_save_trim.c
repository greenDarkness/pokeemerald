/*
 * Pocket Save Trim - fixes "Error in framework - File ID [257] too large" on the Analogue
 * Pocket's Spiritualized GBA core (https://github.com/spiritualized1997/openFPGA-GBA/issues/74).
 *
 * The core only accepts saves that are exactly a real cartridge save size. mGBA and other
 * emulators append RTC data to the .sav (e.g. 131072 + 16 bytes for Emerald). This cuts the
 * file back to the largest valid GBA save size; the game data is untouched and the save still
 * loads in mGBA. The original is kept next to it as "<name>.sav.bak".
 *
 * Use: drag .sav files (or folders) onto the exe, or double-click it to trim every .sav in the
 * exe's own folder.
 *
 * Build (one standalone exe, no DLLs needed):
 *   gcc -O2 -s -static -municode -o "tech/Pocket Save Trim.exe" tech/pocket_save_trim.c
 */
#include <windows.h>
#include <stdarg.h>
#include <stdio.h>
#include <wchar.h>

static const LONGLONG sValidSizes[] = { 512, 8192, 32768, 65536, 131072 };
#define NUM_VALID_SIZES (sizeof(sValidSizes) / sizeof(sValidSizes[0]))
#define MGBA_RTC_SIZE 16

static int sChecked, sTrimmed, sProblems;

static void Print(const wchar_t *fmt, ...)
{
    wchar_t buf[2048];
    DWORD mode, written;
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    va_list args;
    int len;

    va_start(args, fmt);
    len = _vsnwprintf(buf, sizeof(buf) / sizeof(buf[0]) - 1, fmt, args);
    va_end(args);
    if (len < 0)
        len = sizeof(buf) / sizeof(buf[0]) - 1;
    buf[len] = 0;

    if (GetConsoleMode(out, &mode))
    {
        WriteConsoleW(out, buf, len, &written, NULL);
    }
    else
    {
        char utf8[sizeof(buf) * 2];
        int n = WideCharToMultiByte(CP_UTF8, 0, buf, len, utf8, sizeof(utf8), NULL, NULL);
        WriteFile(out, utf8, n, &written, NULL);
    }
}

static void PrintError(const wchar_t *what, const wchar_t *path)
{
    wchar_t msg[512];
    DWORD n = FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
                             GetLastError(), 0, msg, sizeof(msg) / sizeof(msg[0]), NULL);
    while (n > 0 && (msg[n - 1] == L'\r' || msg[n - 1] == L'\n'))
        msg[--n] = 0;
    Print(L"  FAILED   %ls: %ls - %ls\n", path, what, n ? msg : L"unknown error");
    sProblems++;
}

static const wchar_t *FileName(const wchar_t *path)
{
    const wchar_t *name = path;
    for (const wchar_t *p = path; *p; p++)
        if (*p == L'\\' || *p == L'/')
            name = p + 1;
    return name;
}

static BOOL HasSavExtension(const wchar_t *path)
{
    size_t len = wcslen(path);
    return len >= 4 && _wcsicmp(path + len - 4, L".sav") == 0;
}

static void TrimSave(const wchar_t *path)
{
    const wchar_t *name = FileName(path);
    WIN32_FILE_ATTRIBUTE_DATA info;
    LARGE_INTEGER size, target;
    wchar_t backup[MAX_PATH + 8];
    HANDLE file;
    size_t i;

    if (!HasSavExtension(path))
    {
        Print(L"  SKIPPED  %ls - not a .sav file\n", name);
        sProblems++;
        return;
    }
    // macOS "._" metadata files get scattered across SD cards
    if (name[0] == L'.' && name[1] == L'_')
        return;

    sChecked++;
    if (!GetFileAttributesExW(path, GetFileExInfoStandard, &info))
    {
        PrintError(L"can't read size", name);
        return;
    }
    size.LowPart = info.nFileSizeLow;
    size.HighPart = info.nFileSizeHigh;

    target.QuadPart = 0;
    for (i = 0; i < NUM_VALID_SIZES; i++)
        if (sValidSizes[i] <= size.QuadPart)
            target.QuadPart = sValidSizes[i];

    if (target.QuadPart == size.QuadPart)
    {
        Print(L"  ok       %ls (%lld bytes, nothing to trim)\n", name, size.QuadPart);
        return;
    }
    if (target.QuadPart == 0)
    {
        Print(L"  SKIPPED  %ls - only %lld bytes, too small to be a GBA save\n", name, size.QuadPart);
        sProblems++;
        return;
    }

    if (_snwprintf(backup, sizeof(backup) / sizeof(backup[0]), L"%ls.bak", path) < 0
     || !CopyFileW(path, backup, FALSE))
    {
        PrintError(L"can't write backup, left unchanged", name);
        return;
    }

    file = CreateFileW(path, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE)
    {
        PrintError(L"can't open (is the emulator still using it?)", name);
        return;
    }
    if (!SetFilePointerEx(file, target, NULL, FILE_BEGIN) || !SetEndOfFile(file))
    {
        PrintError(L"can't trim", name);
        CloseHandle(file);
        return;
    }
    CloseHandle(file);

    Print(L"  TRIMMED  %ls: %lld -> %lld bytes (removed %lld bytes%ls)\n", name, size.QuadPart,
          target.QuadPart, size.QuadPart - target.QuadPart,
          size.QuadPart - target.QuadPart == MGBA_RTC_SIZE ? L" of mGBA clock data" : L"");
    Print(L"           original kept as %ls\n", FileName(backup));
    sTrimmed++;
}

static void TrimFolder(const wchar_t *dir)
{
    wchar_t pattern[MAX_PATH + 8], path[MAX_PATH * 2];
    WIN32_FIND_DATAW found;
    HANDLE find;

    _snwprintf(pattern, sizeof(pattern) / sizeof(pattern[0]), L"%ls\\*.sav", dir);
    find = FindFirstFileW(pattern, &found);
    if (find == INVALID_HANDLE_VALUE)
        return;
    do
    {
        if (found.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            continue;
        _snwprintf(path, sizeof(path) / sizeof(path[0]), L"%ls\\%ls", dir, found.cFileName);
        TrimSave(path);
    } while (FindNextFileW(find, &found));
    FindClose(find);
}

static void TrimPath(const wchar_t *path)
{
    DWORD attrs = GetFileAttributesW(path);
    if (attrs == INVALID_FILE_ATTRIBUTES)
        PrintError(L"not found", path);
    else if (attrs & FILE_ATTRIBUTE_DIRECTORY)
        TrimFolder(path);
    else
        TrimSave(path);
}

int wmain(int argc, wchar_t **argv)
{
    int i;

    SetConsoleTitleW(L"Pocket Save Trim");
    Print(L"Pocket Save Trim - fixes \"File ID [257] too large\" on the Spiritualized GBA core\n\n");

    if (argc > 1)
    {
        for (i = 1; i < argc; i++)
            TrimPath(argv[i]);
    }
    else
    {
        wchar_t dir[MAX_PATH];
        DWORD len = GetModuleFileNameW(NULL, dir, MAX_PATH);
        while (len > 0 && dir[len - 1] != L'\\')
            len--;
        dir[len > 0 ? len - 1 : 0] = 0;
        Print(L"Checking saves next to this program: %ls\n\n", dir);
        TrimFolder(dir);
    }

    Print(L"\n");
    if (sChecked == 0 && sProblems == 0)
        Print(L"No .sav files found. Drag a save file onto this program to trim it.\n");
    else
        Print(L"%d save(s) checked, %d trimmed, %d problem(s).\n", sChecked, sTrimmed, sProblems);

    Print(L"\nPress Enter to close.");
    getwchar();
    return sProblems ? 1 : 0;
}
