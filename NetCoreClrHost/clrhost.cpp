#include "clrhost.h"
using namespace std;
#define CORECLR_DIR "C:\\Program Files (x86)\\dotnet\\shared\\Microsoft.NETCore.App\\5.0.8"
//TODO: Change how we find the clr

/// <summary>
/// This will load a managed assembly from native. The assembly entrypoint should have any arguments.
/// </summary>
/// <param name="filePath"></param>
/// <param name="appDomainId"></param>
/// <param name="entryNamespace"></param>
/// <param name="entryClassName"></param>
/// <param name="entrySubroutine"></param>
/// <returns></returns>
bool clrhost::LoadManagedAssembly(const char* filePath, const char* appDomainId, const char* managedAssemblyName,
	const char* entryClassName, const char* entrySubroutine) {

	char runtimeDirectory[MAX_PATH];
	GetFullPathNameA(filePath, MAX_PATH, runtimeDirectory, NULL);

	string coreClrAssembly(CORECLR_DIR);
	coreClrAssembly.append("\\coreclr.dll");

	cout << "Full Assembly Name: " << managedAssemblyName << endl;
	cout << "Loading Directory: " << runtimeDirectory << endl;
	cout << "Loading Runtime: " << coreClrAssembly << endl;

	_coreClr = LoadLibraryExA(coreClrAssembly.c_str(), NULL, 0);

	if (_coreClr == NULL) {
		cout << "There was an error loading coreclr.dll!" << endl;
		return false;
	}

	cout << "Found and loaded coreclr.dll" << endl;

	auto initializeCoreClr = (coreclr_initialize_ptr)GetProcAddress(_coreClr, "coreclr_initialize");

	if (initializeCoreClr == NULL) {
		cout << "Could not find 'coreclr_initialize' subroutine in coreclr.dll!" << endl;
		return false;
	}

	cout << "Building Trusted Platform Assemblies list..." << endl;

	string tpaList;
	BuildTpaList(CORECLR_DIR, tpaList);
	BuildTpaList(runtimeDirectory, tpaList);

	cout << "Trusted Platform Assemblies list complete." << endl;

	const char* propertyKeys[] = { "TRUSTED_PLATFORM_ASSEMBLIES", "APP_CONTEXT_BASE_DIRECTORY" };
	const char* propertyValues[] = { tpaList.c_str(), runtimeDirectory };

	int hResponse = initializeCoreClr(
		CORECLR_DIR,
		appDomainId,
		sizeof(propertyKeys) / sizeof(char*),
		propertyKeys,
		propertyValues,
		&_hostHandle,
		&_domainId);

	if (hResponse >= 0) {
		cout << "CoreClr has started!" << endl;
		_managedDirectMethod = CreateManagedDelegate(managedAssemblyName, entryClassName, entrySubroutine);
		_managedDirectMethod();
		return true;
	}

	cout << "CoreClr 'initializeCoreClr' has failed with response code: " << hResponse << endl;
	return true;
}


void clrhost::BuildTpaList(const char* directory, string& tpaList) {
	string searchPath(directory);
	searchPath.append("\\*.dll");

	WIN32_FIND_DATAA findData;
	HANDLE fileHandle = FindFirstFileA(searchPath.c_str(), &findData);

	if (fileHandle != INVALID_HANDLE_VALUE)
	{
		do
		{
			tpaList.append(directory);
			tpaList.append("\\");
			tpaList.append(findData.cFileName);
			tpaList.append(";");
		} while (FindNextFileA(fileHandle, &findData));

		FindClose(fileHandle);
	}
}

managed_direct_method_ptr clrhost::CreateManagedDelegate(const char* managedAssemblyName, const char* managedClassName, const char* managedsubroutineName) {
	auto createManagedDelegate = (coreclr_create_delegate_ptr)GetProcAddress(_coreClr, "coreclr_create_delegate");

	if (createManagedDelegate == NULL) {
		cout << "'coreclr_create_delegate' could not be found within coreclr.dll!" << endl;
		return NULL;
	}

	// The CLR finds the assembly from the TPA list we created it with.
	managed_direct_method_ptr managedSubroutine;
	int hResponse = createManagedDelegate(_hostHandle, _domainId,
		managedAssemblyName,
		managedClassName,
		managedsubroutineName,
		(void**)&managedSubroutine);

	if (hResponse >= 0) {
		cout << "Created Managed Delegate to: '" << managedClassName << "::" << managedsubroutineName << "'" << endl;
		return managedSubroutine;
	}

	cout << "'coreclr_create_delegate' failed to create the delegate with error code: ";
	printf("%x\n", hResponse);
	return NULL;
}