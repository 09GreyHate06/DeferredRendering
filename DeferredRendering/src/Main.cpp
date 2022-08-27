#include <GDX11.h>

#include "DeferredRendering.h"

int main()
{
	try
	{
		DeferredRendering().Run();
	}
	catch (const GDX11::GDX11Exception& e)
	{
		MessageBoxA(nullptr, e.what(), e.GetType(), MB_OK | MB_ICONERROR);
	}
	catch (const std::exception& e)
	{
		MessageBoxA(nullptr, e.what(), "Standard Exception", MB_OK | MB_ICONERROR);
	}
	catch (...)
	{
		MessageBoxA(nullptr, "No details available", "Unknown Exception", MB_OK | MB_ICONERROR);
	}

	return 0;
}