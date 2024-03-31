#pragma once

// .dll export api
#ifdef RH_EXPORT
// Exports
#ifdef _MSC_VER
#define RHAPI __declspec(dllexport)
#else
#define RHAPI __attribute__((visibility("default")))
#endif
#else
// Imports
#ifdef _MSC_VER
#define RHAPI __declspec(dllimport)
#else
#define RHAPI
#endif
#endif

RHAPI int lib_func(int a);