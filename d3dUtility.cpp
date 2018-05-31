//////////////////////////////////////////////////////////////////////////////////////////////////
// 
// File: d3dUtility.cpp
// 
// Author: Frank Luna (C) All Rights Reserved
//
// System: AMD Athlon 1800+ XP, 512 DDR, Geforce 3, Windows XP, MSVC++ 7.0 
//
// Desc: Provides utility functions for simplifying common tasks.
//          
//////////////////////////////////////////////////////////////////////////////////////////////////


#include <assert.h>
#include "d3dUtility.h"


// project the in vertex to YZ plane using the normal n, and save the result to out
void d3d::ProjectionYZ( D3DXVECTOR3* out, D3DXVECTOR3* in, D3DXVECTOR3* n )
{
	assert( out != NULL && in != NULL && n != NULL );
	float k = -in->x / n->x;
	out->x = 0;
	out->y = in->y + k * n->y;
	out->z = in->z + k * n->z;
}

// if the ray go through current box, return true
bool d3d::Box::IsRayInside( Ray* ray )
{
	assert( ray != NULL );
	
	// because the ray is parallel to XZ plane, we can do current process simply

	// first judgement, ray->_origin.y must satisfym_pos0.y > ray->_origin.y > m_pos1.y
	if( ray->_origin.y > m_pos0.y || ray->_origin.y < m_pos1.y ) return false;
	
	

	D3DXVECTOR3 p0 = ray->_origin, p1 = m_pos0, p2 = m_pos1;
	
	if( ray->_direction.x < 0 )
	{
		float temp;
		temp = p1.z;
		p1.z = p2.z;
		p2.z = temp;
	}

	//compute the projected vertex
	D3DXVECTOR3 n = ray->_direction;
	ProjectionYZ( &p0, &p0, &n );
	ProjectionYZ( &p1, &p1, &n );
	ProjectionYZ( &p2, &p2, &n );

	// if p0.z beteen p1.z and p2.z, return true
	if( ( p0.z - p1.z ) * ( p0.z - p2.z ) < 0 )
	{
		return true;
	}

	return false;
}


D3DLIGHT9 d3d::InitDirectionalLight(D3DXVECTOR3* direction, D3DXCOLOR* color)
{
	D3DLIGHT9 light;
	::ZeroMemory(&light, sizeof(light));

	light.Type      = D3DLIGHT_DIRECTIONAL;
	light.Ambient   = *color * 0.6f;
	light.Diffuse   = *color;
	light.Specular  = *color * 0.6f;
	light.Direction = *direction;

	return light;
}



D3DMATERIAL9 d3d::InitMtrl(D3DXCOLOR a, D3DXCOLOR d, D3DXCOLOR s, D3DXCOLOR e, float p)
{
	D3DMATERIAL9 mtrl;
	mtrl.Ambient  = a;
	mtrl.Diffuse  = d;
	mtrl.Specular = s;
	mtrl.Emissive = e;
	mtrl.Power    = p;
	return mtrl;
}



