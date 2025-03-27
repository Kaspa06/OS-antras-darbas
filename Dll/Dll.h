#pragma once
#ifndef _DLL_H_
#define _DLL_H_

#ifdef DLL_EXPORTS
#define DLL_API __declspec(dllexport)
#else
#define DLL_API __declspec(dllimport)
#endif

extern "C" {
    DLL_API void StartTimer();
    DLL_API void EndTimer();

    DLL_API void PingWithTempFile();
    DLL_API void CreateCustomFolderStructure();
    DLL_API void ComputeTschirnhausen(double F, double x0, double xn, double dx);
    DLL_API void MergeResults(double F);
    DLL_API void DeleteGeneratedFolders();
}

#endif
