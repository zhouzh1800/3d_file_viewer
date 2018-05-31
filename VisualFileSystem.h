#ifndef VISUAL_FILESYSTEM_H
#define VISUAL_FILESYSTEM_H

#define   _WIN32_WINNT           0x0400

#if (_WIN32_WINNT >= 0x0400) || (_WIN32_WINDOWS > 0x0400)
#define WM_MOUSEWHEEL                   0x020A				// use WM_MOUSEWHEEL message
#endif


// include some header file
#include "d3dUtility.h"
#include "fileSystem.h"


#define ITEM_HEIGHT  0.2f
#define SPACE_HEIGHT 0.05f
//
// Vertex
//
struct Vertex
{
	//construction
	Vertex(){}

	Vertex(float x, float y, float z, float nx, float ny, float nz)
	{
		_x  = x;  _y  = y;	_z  = z;
		_nx = nx; _ny = ny; _nz = nz;
		_tx = _ty = 0;
	}

	Vertex(float x, float y, float z, float nx, float ny, float nz, float tx, float ty )
	{
		_x  = x;  _y  = y;	_z  = z;
		_nx = nx; _ny = ny; _nz = nz;
		_tx = tx; _ty = ty;
	}

	float  _x,  _y,  _z;		// position
	float _nx, _ny, _nz;		// normal
	
	float _tx, _ty;				// texture coordinate

	static const DWORD FVF;		// Vertex format
};




//
// Device
//
class Device
{
public:
	IDirect3DDevice9* m_device;				// interface of d3d device

	DWORD m_width;							// width of window
	DWORD m_height;							// height of window

	DWORD m_numOfTaperSector;				// the number of sectors of taper	
	float m_ratio;							// scaling ratio
	
	float m_itemsTextureWidth;				// the width of the texture of items
	float m_itemsTextureHeigth;				// the height of the texture of items

	IDirect3DVertexBuffer9* m_taper;		// the buffer of vertex of taper
	IDirect3DVertexBuffer9* m_rectangle;	// the buffer of vertex of rectangle

	
	VisualDirectory m_vslDirectory;			// visual directory
	

	bool m_isMoving;
	bool m_isRightButton;
	POINT m_firstPos;
	POINT m_secondPos;
	VisualDirectory* m_pickingVslDir;
	HWND m_oldHwnd;

	Device();		// construct

	void InitVslDirectory( void );		// initual visual directory
	bool InitD3D							
		(
		HINSTANCE hInstance,		// Application instance.
		int width, int height,		// Backbuffer dimensions.
		bool windowed,				// Windowed (true)or full screen (false).
		D3DDEVTYPE deviceType		// HAL or REF
		);

	bool Setup( DWORD numOfTaperSector, float ratio ); 	// setup parameters, render state, transform matrixes and meshes, and so on
	bool Display( void );			// display the file system graph
	void Cleanup( void );			// clean up resource

	
	void InitTaperModel( void );	// initual taper model
	void InitRectangleModel( void );// initual rectangle model

	void InitRenderState( void );	// initial render state
	void InitTransformMatrix( void );		// initial transform matrixes
	
	//just as its name implies
	void CalculateDirectoryGraph( VisualDirectory* pVisualDirectory, D3DXVECTOR3* pos );	
	//just as its name implies
	void CalculateItemGraph( VisualDirectory* pVisualDirectory );
	
	//just as its name implies
	void DrawDirectory( VisualDirectory* pVisualDirectory );
	//just as its name implies
	void DeviceDrawItem( VisualItem* pVisualItem );
	
	//just as its name implies
	void DrawTaper( VisualDirectory* pVisualDirectory );
	//just as its name implies
	void DrawRectangle( VisualItem* pVisualItem );
	
	// pick test
	bool TaperPickTest( VisualDirectory* pVisualDirectory, d3d::Ray* ray, bool* isDir, void** vp, float* k );
	bool RectanglePickTest( VisualItem* pVisualItem, d3d::Ray* ray , bool* isDir, void** vp, float* k );
	
	// calculate the picking ray
	d3d::Ray CalcPickingRay( float x, float y );
	
	// transform ray using T matrix
	void TransformRay( d3d::Ray* ray, D3DXMATRIX* T );
	
	// return the distance of rays
	float CalculateRayDistance( d3d::Ray* ray0, d3d::Ray* ray1 );

	// directory revolution, rotation and file rotation
	void DirectoryRevolution( VisualDirectory* vd, D3DXVECTOR3* center, float angle );
	void DirectoryRotation( VisualDirectory* vd, float angle );
	void ItemRevolution( VisualItem* vi, D3DXVECTOR3* center, float angle );
	

	// window process
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK WndProcEx(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
};



#endif