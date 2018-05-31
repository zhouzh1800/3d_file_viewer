//
// WinMain
//

#include "VisualFileSystem.h"

// WinMain
int WINAPI WinMain(HINSTANCE hinstance,
				   HINSTANCE prevInstance, 
				   PSTR cmdLine,
				   int showCmd)
{
	//create a Device Object, device
	Device device;
	
	// Initialize Visual Directory
	device.InitVslDirectory();
	
	// Initialize Direct3D 
	if(!device.InitD3D(hinstance, 800, 600, true, D3DDEVTYPE_HAL ) )
	{
		::MessageBox(0, "InitD3D() - FAILED", 0, 0);
		return 0;
	}
	
	// setup parameters, render state, transform matrixes and meshes, and so on
	if(! device.Setup( 30, 200 ) )
	{
		::MessageBox(0, "Setup() - FAILED", 0, 0);
		return 0;
	}

	//Display after Initialize
	device.Display();


	//message loop
	MSG msg;
	while(GetMessage(&msg, NULL, 0, 0 ) )
	{
		TranslateMessage( &msg );
		DispatchMessage( &msg );
    }
	
	// clean up resource
	device.Cleanup();

	return 0;
}