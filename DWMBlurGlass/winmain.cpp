/**
 * FileName: winmain.cpp
 *
 * Copyright (C) 2024 Maplespe
 *
 * This file is part of MToolBox and DWMBlurGlass.
 * DWMBlurGlass is free software: you can redistribute it and/or modify it under the terms of the
 * GNU Lesser General Public License as published by the Free Software Foundation, either version 3
 * of the License, or any later version.
 *
 * DWMBlurGlass is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License along with Foobar.
 * If not, see <https://www.gnu.org/licenses/lgpl-3.0.html>.
*/
#include "framework.h"
#include "Helper/Helper.h"
#include "MHostHelper.h"
#include <Knownfolders.h>
#include <Shlobj.h>

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

#ifndef _DEBUG
	if (!MDWMBlurGlass::IsRunasAdmin())
		return 0;
#endif

	PWSTR shfolder = nullptr;
	if (SHGetKnownFolderPath(FOLDERID_Profile, 0, nullptr, &shfolder) == S_OK)
	{
		std::wstring path = shfolder;
		CoTaskMemFree(shfolder);
		std::wstring curpath = MDWMBlurGlass::Utils::GetCurrentDir();
		if (curpath.length() >= path.length() && curpath.substr(0, path.length()) == path)
		{
			MessageBoxW(nullptr, L"Running in sandbox!", L"DWMBlurGlass: warning", MB_ICONWARNING | MB_TOPMOST);
			return false;
		}
	}

	HRESULT hr = CoInitialize(nullptr);
	if (FAILED(hr))
	{
		MessageBoxW(nullptr, L"CoInitialize failed!", L"DWMBlurGlass", MB_ICONERROR);
		return 0;
	}

	if(lpCmdLine && _wcsicmp(lpCmdLine, L"loaddll") == 0)
	{
		std::wstring err;
		if (!MDWMBlurGlass::LoadDWMExtensionBase(err))
			MessageBoxW(nullptr, err.c_str(), L"DWMBlurGlass: loaddll error", MB_ICONERROR);
		return 0;
	}
	
	if (lpCmdLine && _wcsicmp(lpCmdLine, L"unloaddll") == 0)
	{
		std::wstring err;
		if (!MDWMBlurGlass::ShutdownDWMExtension(err))
			MessageBoxW(nullptr, err.c_str(), L"DWMBlurGlass: unloaddll error", MB_ICONERROR);
		return 0;
	}

	if (lpCmdLine && _wcsicmp(lpCmdLine, L"install") == 0)
	{
		std::wstring errinfo;
		
		if (MDWMBlurGlass::IsInstallTasks())
			return 0;

		if (MDWMBlurGlass::InstallScheduledTasks(errinfo)) {
			std::wstring err;
			bool symbolState = MDWMBlurGlass::MHostGetSymbolState();
			if (symbolState && !MDWMBlurGlass::LoadDWMExtension(err))
			{
				MessageBoxW(nullptr, (L"Failed to load component! Error message: " + err).c_str(), L"DWMBlurGlass: install error", MB_ICONERROR);
				return 0;
			}

			if (symbolState) {
				PostMessageW(FindWindowW(L"Dwm", nullptr), WM_THEMECHANGED, 0, 0);
        		InvalidateRect(nullptr, nullptr, FALSE);
			}

			MessageBoxW(nullptr, 
				symbolState ? L"Install successfully!" : L"Install successfully!" 
				"But you haven't downloaded a valid symbol file yet, download it from the \"Symbols\" page to make DWMBlurGlass work!", 
				L"DWMBlurGlass: install", 
				MB_ICONINFORMATION);
		}
		else
		{
			MessageBoxW(nullptr, (L"Install failed! Error message: " + errinfo).c_str(), L"DWMBlurGlass: install error", MB_ICONERROR);
		}
		return 0;
	}

	if (lpCmdLine && _wcsicmp(lpCmdLine, L"uninstall") == 0)
	{
		std::wstring errinfo;
		MDWMBlurGlass::ShutdownDWMExtension(errinfo);

		if (!MDWMBlurGlass::IsInstallTasks())
			return 0;

		if (MDWMBlurGlass::DeleteScheduledTasks(errinfo)) 
		{
			MessageBoxW(nullptr, L"Uninstall successfully.", L"DWMBlurGlass: uninstall success", MB_ICONINFORMATION);
		} 
		else
		{ 
			MessageBoxW(nullptr, (L"Uninstall failed! Error message: " + errinfo).c_str(), L"DWMBlurGlass: uninstall error", MB_ICONERROR);
		}
		return 0;
	}

	if (lpCmdLine && _wcsicmp(lpCmdLine, L"downloadsym") == 0)
	{
		if (MDWMBlurGlass::MHostGetSymbolState())
			return 0;

		if (!MDWMBlurGlass::MHostDownloadSymbol()) 
		{
			MessageBoxW(nullptr,
				L"Download failed! Unable to download symbol files from \"msdl.microsoft.com\".",
				L"DWMBlurGlass: Download",
				MB_ICONERROR
			);
			return 0;
		}
                        
        if (MDWMBlurGlass::IsInstallTasks())
        {
            std::wstring err;
			if (!MDWMBlurGlass::LoadDWMExtension(err))
			{
				MessageBoxW(nullptr, (L"Failed to load component! Error message: " + err).c_str(), L"Error", MB_ICONERROR);
			}
			else
			{
				PostMessageW(FindWindowW(L"Dwm", nullptr), WM_THEMECHANGED, 0, 0);
				InvalidateRect(nullptr, nullptr, FALSE);
			}
        }
	}

	if (lpCmdLine && _wcsicmp(lpCmdLine, L"refresh") == 0)
	{
		MDWMBlurGlass::MHostNotify(MDWMBlurGlass::MHostNotifyType::Refresh);
		if (MDWMBlurGlass::IsInstallTasks())
		{
			PostMessageW(FindWindowW(L"Dwm", nullptr), WM_THEMECHANGED, 0, 0);
			InvalidateRect(nullptr, nullptr, FALSE);
		}
		return 0;
	}

	/*
	HANDLE hObject = CreateMutexW(nullptr, FALSE, L"_DWMBlurGlass_");
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		CloseHandle(hObject);
		return 0;
	}
	if (hObject)
		ReleaseMutex(hObject);
	*/

	return 0;
}