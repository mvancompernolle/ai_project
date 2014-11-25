// DynamicLibrary.cpp

#include "DynamicLibrary.h"

// ctor that loads shared lib
DynamicLibrary::DynamicLibrary(const char *libName) : libHandle(NULL)
{
	Load(libName);
}

DynamicLibrary::~DynamicLibrary()
{
	Unload();
}

// load and unload member methods
bool DynamicLibrary::Load(const char *libName)
{
	if (libName != NULL && std::strlen(libName) > 0)
		if (libHandle == NULL)
			libHandle = ::dlopen( libName, RTLD_LAZY );

	return (libHandle != NULL);
}

bool DynamicLibrary::Unload()
{
	if (libHandle != NULL)
		::dlclose( libHandle );

	libHandle = NULL;
	cache.clear();
	return true;
}

// cached and uncached access to functions in dynamic lib
DLPROC DynamicLibrary::GetProcAddr(const char *procName) const
{
	DLPROC proc = NULL;
	if (libHandle != NULL)
		proc = (DLPROC) ::dlsym(libHandle, procName);
	return proc;
}

DLPROC DynamicLibrary::GetProcAddrCached(const char *procName, size_t procId)
{
	DLPROC proc = NULL;
	if (libHandle != NULL) {
		if ((procId >= cache.size()) || (cache.size() < 1))
			cache.resize(procId + 1, cache_info());
		cache_info &ci = cache[ procId ];
		if (ci.procAddr == NULL && !ci.testFlag) {
			ci.testFlag = true;
			ci.procAddr = GetProcAddr(procName);
		}
		proc = ci.procAddr;
	}
	return proc;
}