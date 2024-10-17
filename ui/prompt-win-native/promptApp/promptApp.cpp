#include <windows.h>
#include <iostream>
#include <string>
#include "Resource.h"

static std::wstring defaultPromptText;
// Dialog procedure
INT_PTR CALLBACK DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_INITDIALOG: {
        // Center the dialog on the screen
        RECT rcDlg, rcDesktop;
        GetWindowRect(hwnd, &rcDlg);
        SystemParametersInfo(SPI_GETWORKAREA, 0, &rcDesktop, 0);
        int x = (rcDesktop.right - rcDesktop.left - (rcDlg.right - rcDlg.left)) / 2;
        int y = (rcDesktop.bottom - rcDesktop.top - (rcDlg.bottom - rcDlg.top)) / 2;
        SetWindowPos(hwnd, NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

        // Get the default text from command-line arguments (-p)
        LPWSTR* argv;
        int argc;
        defaultPromptText.erase();
        argv = CommandLineToArgvW(GetCommandLineW(), &argc);
        if (argv != NULL && argc > 2) {
            std::wstring arg = argv[1];
            if (arg == L"-p" && argc >= 3) {
                defaultPromptText = argv[2];
            }
        }
        LocalFree(argv);

        // Set the default text in the text entry box
        SetDlgItemTextW(hwnd, IDC_PROMPT, defaultPromptText.c_str());
        return TRUE;
    }
    case WM_COMMAND: {
        switch (LOWORD(wParam)) {
        case IDOK: {
            WCHAR buffer[256];
            GetDlgItemTextW(hwnd, IDC_PROMPT, buffer, 256);
            std::wcout << buffer << std::endl;
            EndDialog(hwnd, IDOK);
            return TRUE;
        }
        case IDCANCEL:
            // write out default text
            std::wcout << defaultPromptText << std::endl;
            EndDialog(hwnd, IDCANCEL);
            return TRUE;
        }
        break;
    }
    case WM_CLOSE:
        // write out default text
        std::wcout << defaultPromptText << std::endl;
        EndDialog(hwnd, IDCANCEL);
        return TRUE;
    }
    return FALSE;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    // Create the dialog box
    int ret = DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG), NULL, DialogProc);
    if (ret == -1) {
        MessageBox(NULL, L"Failed to create dialog box!", L"Error", MB_ICONERROR);
        return 1;
    }

    return 0;
}
