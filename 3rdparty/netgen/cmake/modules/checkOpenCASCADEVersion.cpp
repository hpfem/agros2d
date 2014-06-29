/* ----------------------------------------------
 Simple C++ Code to be used by CMake to 
 extracts the version string from the 
 OpenCASCADE header file "Standard_Version.hxx"
-------------------------------------------------*/

#include "Standard_Version.hxx"
#include <iostream>

using namespace std;

int main(void)
{
#ifdef OCC_VERSION_MAINTENANCE
	cout << OCC_VERSION_MAJOR << "." << OCC_VERSION_MINOR << "." << OCC_VERSION_MAINTENANCE << endl;
#else
	cout << OCC_VERSION_STRING << endl;
#endif
	
	return 0;
}
   