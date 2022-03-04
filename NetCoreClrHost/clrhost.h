#pragma once
#include <iostream>
#include <string.h>
#include <string>
#include "coreclrhost.h" 
#include <Windows.h>

using namespace std;

typedef char* (*managed_direct_method_ptr)();

class clrhost
{
public:
	bool LoadManagedAssembly(const char* filePath, const char* appDomainId, const char* managedAssemblyName, const char* entryClassName, const char* entrySubroutine);

private:
	HMODULE _coreClr;
	void* _hostHandle;
	unsigned int _domainId;
	managed_direct_method_ptr _managedDirectMethod;

	void BuildTpaList(const char* directory, string& tpaList);
	managed_direct_method_ptr CreateManagedDelegate(const char* managedAssemblyName, const char* managedClassName, const char* managedsubroutineName);
};