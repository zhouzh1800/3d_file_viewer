//////////////////////////////////////////////////////////////////////////////////////////////////
// 
// File: d3dUtility.h
// 
// Author: Frank Luna (C) All Rights Reserved
//
// System: AMD Athlon 1800+ XP, 512 DDR, Geforce 3, Windows XP, MSVC++ 7.0 
//
// Desc: Provides utility functions for simplifying common tasks.
//          
//////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __d3dUtilityH__
#define __d3dUtilityH__


// include some header file
#include <d3dx9.h>
#include <string>


// namespace
namespace d3d
{
	
	// project the in vertex to YZ plane using the normal n, and save the result to out
	void ProjectionYZ( D3DXVECTOR3* out, D3DXVECTOR3* in, D3DXVECTOR3* n );


	//
	//ray
	//
	struct Ray
	{
		D3DXVECTOR3 _origin;
		D3DXVECTOR3 _direction;
	};
	
	
	//
	// box
	//
	struct Box
	{
		D3DXVECTOR3	m_pos0;
		D3DXVECTOR3 m_pos1;

		bool IsRayInside(Ray* ray );
	};
	

	// release
	template<class T> void Release(T t)
	{
		if( t )
		{
			t->Release();
			t = 0;
		}
	}
		
	//delete
	template<class T> void Delete(T t)
	{
		if( t )
		{
			delete t;
			t = 0;
		}
	}
	
	//
	// colors
	//
	const D3DXCOLOR      GREY( D3DCOLOR_XRGB(64, 64, 64) );
	const D3DXCOLOR      WHITE( D3DCOLOR_XRGB(255, 255, 255) );
	const D3DXCOLOR      BLACK( D3DCOLOR_XRGB(  0,   0,   0) );
	const D3DXCOLOR        RED( D3DCOLOR_XRGB(255,   0,   0) );
	const D3DXCOLOR      GREEN( D3DCOLOR_XRGB(  0, 255,   0) );
	const D3DXCOLOR       BLUE( D3DCOLOR_XRGB(  0,   0, 255) );
	const D3DXCOLOR     YELLOW( D3DCOLOR_XRGB(255, 255,   0) );
	const D3DXCOLOR       CYAN( D3DCOLOR_XRGB(  0, 255, 255) );
	const D3DXCOLOR    MAGENTA( D3DCOLOR_XRGB(255,   0, 255) );
	



	//
	// Lights
	//
	D3DLIGHT9 InitDirectionalLight(D3DXVECTOR3* direction, D3DXCOLOR* color);

	//
	// Materials
	//
	D3DMATERIAL9 InitMtrl(D3DXCOLOR a, D3DXCOLOR d, D3DXCOLOR s, D3DXCOLOR e, float p);
	
	const D3DMATERIAL9 WHITE_MTRL  = InitMtrl(WHITE, WHITE, WHITE, BLACK, 2.0f);
	const D3DMATERIAL9 RED_MTRL    = InitMtrl(RED, RED, RED, BLACK, 2.0f);
	const D3DMATERIAL9 GREEN_MTRL  = InitMtrl(GREEN, GREEN, GREEN, BLACK, 2.0f);
	const D3DMATERIAL9 BLUE_MTRL   = InitMtrl(BLUE, BLUE, BLUE, BLACK, 2.0f);
	const D3DMATERIAL9 YELLOW_MTRL = InitMtrl(YELLOW, YELLOW, YELLOW, BLACK, 2.0f);
}

#endif // __d3dUtilityH__