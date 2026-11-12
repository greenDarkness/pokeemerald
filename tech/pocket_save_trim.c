/*
 * Pocket Save Trim - fixes "Error in framework - File ID [257] too large" on the Analogue
 * Pocket's Spiritualized GBA core (https://github.com/spiritualized1997/openFPGA-GBA/issues/74).
 *
 * The core only accepts saves that are exactly a real cartridge save size. mGBA and other
 * emulators append RTC data to the .sav (e.g. 131072 + 16 bytes for Emerald). This cuts the
 * file back to the largest valid GBA save size; the game data is untouched and the save still
 * loads in mGBA. Every checked save (before trimming) is copied to BACKUP_DIR under its own
 * name, overwriting any older copy there.
 *
 * Afterwards it syncs BAK_SAVE and PROJECT_SAVE (the newer one overwrites the older one) and
 * copies ROM_SOURCE into ROM_DEST_DIR, overwriting any copy already there.
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

#define BACKUP_DIR   L"D:\\projects\\bak"
#define BAK_SAVE     BACKUP_DIR L"\\pokeemerald.sav"
#define PROJECT_SAVE L"D:\\projects\\pokeemerald\\pokeemerald.sav"
#define ROM_SOURCE   L"D:\\projects\\pokeemerald\\pokeemerald.gba"
#define ROM_DEST_DIR L"E:\\Assets\\gba\\common"

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

static BOOL SamePath(const wchar_t *a, const wchar_t *b)
{
    wchar_t fullA[MAX_PATH * 2], fullB[MAX_PATH * 2];
    if (!GetFullPathNameW(a, MAX_PATH * 2, fullA, NULL) || !GetFullPathNameW(b, MAX_PATH * 2, fullB, NULL))
        return FALSE;
    return _wcsicmp(fullA, fullB) == 0;
}

static BOOL BackupSave(const wchar_t *path, const wchar_t *name)
{
    wchar_t backup[MAX_PATH * 2];

    _snwprintf(backup, sizeof(backup) / sizeof(backup[0]), L"%ls\\%ls", BACKUP_DIR, name);
    backup[sizeof(backup) / sizeof(backup[0]) - 1] = 0;
    if (SamePath(path, backup))
        return TRUE;

    CreateDirectoryW(BACKUP_DIR, NULL);
    if (!CopyFileW(path, backup, FALSE))
    {
        PrintError(L"can't write backup to " BACKUP_DIR L", left unchanged", name);
        return FALSE;
    }
    Print(L"  BACKUP   %ls -> %ls\n", name, backup);
    return TRUE;
}

static void TrimSave(const wchar_t *path)
{
    const wchar_t *name = FileName(path);
    WIN32_FILE_ATTRIBUTE_DATA info;
    LARGE_INTEGER size, target;
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

    if (target.QuadPart == 0)
    {
        Print(L"  SKIPPED  %ls - only %lld bytes, too small to be a GBA save\n", name, size.QuadPart);
        sProblems++;
        return;
    }

    if (!BackupSave(path, name))
        return;

    if (target.QuadPart == size.QuadPart)
    {
        Print(L"  ok       %ls (%lld bytes, nothing to trim)\n", name, size.QuadPart);
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
    sTrimmed++;
}

static void SyncNewestSave(void)
{
    WIN32_FILE_ATTRIBUTE_DATA bak, project;
    BOOL hasBak = GetFileAttributesExW(BAK_SAVE, GetFileExInfoStandard, &bak);
    BOOL hasProject = GetFileAttributesExW(PROJECT_SAVE, GetFileExInfoStandard, &project);
    const wchar_t *newer, *older;
    LONG cmp;

    Print(L"\nSyncing newest save:\n");
    if (!hasBak && !hasProject)
    {
        Print(L"  SKIPPED  neither %ls nor %ls exists\n", BAK_SAVE, PROJECT_SAVE);
        return;
    }

    cmp = !hasProject ? 1 : !hasBak ? -1 : CompareFileTime(&bak.ftLastWriteTime, &project.ftLastWriteTime);
    if (cmp == 0)
    {
        Print(L"  ok       both saves have the same timestamp, nothing to copy\n");
        return;
    }
    newer = cmp > 0 ? BAK_SAVE : PROJECT_SAVE;
    older = cmp > 0 ? PROJECT_SAVE : BAK_SAVE;

    if (!CopyFileW(newer, older, FALSE))
    {
        PrintError(L"can't overwrite older save", older);
        return;
    }
    Print(L"  COPIED   %ls (newer)\n        -> %ls\n", newer, older);
}

static void CopyRom(void)
{
    wchar_t dest[MAX_PATH * 2];

    _snwprintf(dest, sizeof(dest) / sizeof(dest[0]), L"%ls\\%ls", ROM_DEST_DIR, FileName(ROM_SOURCE));
    dest[sizeof(dest) / sizeof(dest[0]) - 1] = 0;
    Print(L"\nCopying ROM:\n");
    if (!CopyFileW(ROM_SOURCE, dest, FALSE))
    {
        PrintError(L"can't copy ROM", dest);
        return;
    }
    Print(L"  COPIED   %ls\n        -> %ls\n", ROM_SOURCE, dest);
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

    if (sChecked == 0 && sProblems == 0)
        Print(L"\nNo .sav files found. Drag a save file onto this program to trim it.\n");
    else
        Print(L"\n%d save(s) checked, %d trimmed.\n", sChecked, sTrimmed);

    SyncNewestSave();
    CopyRom();

    Print(L"\n%d problem(s).\n", sProblems);

    Print(L"\nPress Enter to close.");
    getwchar();
    return sProblems ? 1 : 0;
}
