#include "VisualFileSystem.h"


// veterx format, include position, normal, texture coordinate
const DWORD Vertex::FVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1;


//
// Device
//
Device::Device():
m_isMoving( false ),
m_isRightButton( false ),
m_pickingVslDir( NULL ),
m_oldHwnd( NULL ),
m_device( NULL )
{}

void Device::Cleanup()
{
	d3d::Release<IDirect3DVertexBuffer9*>(m_taper);
	d3d::Release<IDirect3DVertexBuffer9*>(m_rectangle);
	d3d::Release<IDirect3DDevice9*>(m_device);
}

void Device::InitVslDirectory()
{
	m_vslDirectory.InitVslDirectoryUsingSections();
}

bool Device::InitD3D(
						HINSTANCE hInstance,       // [in] Application instance.
						int width, int height,     // [in] Backbuffer dimensions.
						bool windowed,             // [in] Windowed (true)or full screen (false).
						D3DDEVTYPE deviceType     // [in] HAL or REF)
					)
{
	//
	// Create the main application window.
	//
	m_width = width;
	m_height = height;


	WNDCLASS wc;

	wc.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wc.lpfnWndProc   = (WNDPROC)WndProc; 
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInstance;
	wc.hIcon         = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor       = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName  = 0;
	wc.lpszClassName = "VisualFileSystem";

	if( !RegisterClass(&wc) ) 
	{
		::MessageBox(0, "RegisterClass() - FAILED", 0, 0);
		return false;
	}
		
	HWND hwnd = 0;
	hwnd = ::CreateWindow(wc.lpszClassName, wc.lpszClassName, 
		WS_EX_TOPMOST,
		50, 50, width, height,
		0 /*parent hwnd*/, 0 /* menu */, hInstance, 0 /*extra*/); 
	
	SetWindowLong( hwnd, GWL_USERDATA, (long)this );
	if( !hwnd )
	{
		::MessageBox(0, "CreateWindow() - FAILED", 0, 0);
		return false;
	}

	::ShowWindow(hwnd, SW_SHOW);
	::UpdateWindow(hwnd);

	//
	// Init D3D: 
	//

	HRESULT hr = 0;

	// Step 1: Create the IDirect3D9 object.

	IDirect3D9* d3d9 = 0;
    d3d9 = Direct3DCreate9(D3D_SDK_VERSION);

    if( !d3d9 )
	{
		::MessageBox(0, "Direct3DCreate9() - FAILED", 0, 0);
		return false;
	}

	// Step 2: Check for hardware vp.

	D3DCAPS9 caps;
	d3d9->GetDeviceCaps(D3DADAPTER_DEFAULT, deviceType, &caps);

	int vp = 0;
	if( caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT )
		vp = D3DCREATE_HARDWARE_VERTEXPROCESSING;
	else
		vp = D3DCREATE_SOFTWARE_VERTEXPROCESSING;

	// Step 3: Fill out the D3DPRESENT_PARAMETERS structure.
	
	RECT rect;
	GetClientRect(hwnd, &rect);

	D3DPRESENT_PARAMETERS d3dpp;
	d3dpp.BackBufferWidth            = rect.right - rect.left;
	d3dpp.BackBufferHeight           = rect.bottom - rect.top;
	d3dpp.BackBufferFormat           = D3DFMT_A8R8G8B8;
	d3dpp.BackBufferCount            = 1;
	d3dpp.MultiSampleType            = D3DMULTISAMPLE_NONE;
	d3dpp.MultiSampleQuality         = 0;
	d3dpp.SwapEffect                 = D3DSWAPEFFECT_DISCARD; 
	d3dpp.hDeviceWindow              = hwnd;
	d3dpp.Windowed                   = windowed;
	d3dpp.EnableAutoDepthStencil     = true; 
	d3dpp.AutoDepthStencilFormat     = D3DFMT_D24S8;
	d3dpp.Flags                      = 0;
	d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	d3dpp.PresentationInterval       = D3DPRESENT_INTERVAL_IMMEDIATE;

	// Step 4: Create the device.

	hr = d3d9->CreateDevice(
		D3DADAPTER_DEFAULT, // primary adapter
		deviceType,         // device type
		hwnd,               // window associated with device
		vp,                 // vertex processing
	    &d3dpp,             // present parameters
	    &m_device);            // return created device

	if( FAILED(hr) )
	{
		// try again using a 16-bit depth buffer
		d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
		
		hr = d3d9->CreateDevice(
			D3DADAPTER_DEFAULT,
			deviceType,
			hwnd,
			vp,
			&d3dpp,
			&m_device);

		if( FAILED(hr) )
		{
			d3d9->Release(); // done with d3d9 object
			::MessageBox(0, "CreateDevice() - FAILED", 0, 0);
			return false;
		}
	}

	d3d9->Release(); // done with d3d9 object
	
	return true;
}

void Device::InitRectangleModel()
{
	m_device->CreateVertexBuffer
	(
		6 * sizeof( Vertex ),
		D3DUSAGE_WRITEONLY,
		Vertex::FVF,
		D3DPOOL_MANAGED,
		&m_rectangle,
		0
	);

	float edgeLength = 1.0f;

	// rectangle involve two triangle
	Vertex* rv;
	m_rectangle->Lock(0, 0, (void**)&rv, 0);
	
	Vertex angles[4];
	angles[0] = Vertex(  edgeLength,  edgeLength, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f );
	angles[1] = Vertex(	 0,			  edgeLength, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f );
	angles[2] = Vertex(  edgeLength, -edgeLength, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f );
	angles[3] = Vertex(  0,			 -edgeLength, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f );

	// first triangle
	rv[0] = angles[0];
	rv[1] = angles[2];
	rv[2] = angles[1];
	
	// second triangle
	rv[3] = angles[3];
	rv[4] = angles[1];
	rv[5] = angles[2];

	m_rectangle->Unlock();
}

void Device::InitTaperModel()
{
	m_device->CreateVertexBuffer
	(
		6 * m_numOfTaperSector * sizeof(Vertex), 
		D3DUSAGE_WRITEONLY,
		Vertex::FVF,
		D3DPOOL_MANAGED,
		&m_taper,
		0
	);

	//
	// Fill the vertex buffer with taper data.
	//
	float radius = 1.0f;
	float taperHeight = 1.0f;
	D3DXVECTOR3 taperVertex( 0, 0, 0 );
	
	D3DXVECTOR3	buttomCenter( taperHeight, 0, 0 );

	D3DXVECTOR3 firstButtonVertex(taperHeight, radius, 0 );

	D3DXMATRIX Rb;
	
	D3DXMatrixRotationX( &Rb, ( 2 * D3DX_PI ) / m_numOfTaperSector );
	Vertex* v;

	m_taper->Lock(0, 0, (void**)&v, 0);

	D3DXVECTOR3 vt0 = firstButtonVertex;
	D3DXVECTOR3 vt1 = firstButtonVertex;
	
	
	// calculate all triangles for taper
	for( int i = 0; i < m_numOfTaperSector; i++ )
	{
		vt0 = vt1;
		D3DXVec3TransformCoord( &vt1, &vt1, &Rb );
		v[ i * 3 + 0 ] = Vertex( buttomCenter.x, buttomCenter.y, buttomCenter.z, buttomCenter.x, buttomCenter.y, buttomCenter.z );
		v[ i * 3 + 1 ] = Vertex( vt0.x, vt0.y, vt0.z, -vt0.x, -vt0.y, -vt0.z );// -buttomCenter.x, buttomCenter.y, buttomCenter.z );
		v[ i * 3 + 2 ] = Vertex( vt1.x, vt1.y, vt1.z, -vt1.x, -vt1.y, -vt1.z );//-buttomCenter.x, buttomCenter.y, buttomCenter.z );

		D3DXVECTOR3 normalOfSide;
		D3DXVec3Cross( &normalOfSide, &( vt1 - taperVertex ), &( vt0 - taperVertex ) );

		v[ i * 3 + 0 + m_numOfTaperSector * 3 ] = Vertex( taperVertex.x, taperVertex.y, taperVertex.z, normalOfSide.x, normalOfSide.y, normalOfSide.z );
		v[ i * 3 + 2 + m_numOfTaperSector * 3 ] = Vertex( vt0.x, vt0.y, vt0.z, normalOfSide.x, normalOfSide.y, normalOfSide.z );
		v[ i * 3 + 1 + m_numOfTaperSector * 3 ] = Vertex( vt1.x, vt1.y, vt1.z, normalOfSide.x, normalOfSide.y, normalOfSide.z );
	}
	
	m_taper->Unlock();
}

void Device::InitRenderState()
{
	//
	// Turn on lighting.
	//
	m_device->SetRenderState( D3DRS_LIGHTING, true );



	//
	// Create and set the material.
	//
	D3DMATERIAL9 mtrl;
	mtrl.Ambient  = d3d::BLACK;
	mtrl.Diffuse  = d3d::WHITE;
	mtrl.Specular = d3d::BLACK;
	mtrl.Emissive = d3d::BLACK;
	mtrl.Power    = 5.0f;

	m_device->SetMaterial(&mtrl);



	//
	// Setup a directional light.
	//
	D3DLIGHT9 dir0;
	::ZeroMemory(&dir0, sizeof(dir0));
	dir0.Type      = D3DLIGHT_DIRECTIONAL;
	dir0.Diffuse   = d3d::WHITE;
	dir0.Specular  = d3d::WHITE;
	dir0.Ambient   = d3d::WHITE;
	dir0.Direction = D3DXVECTOR3( 0.5f, -0.3f, 1.0f );
	
	D3DLIGHT9 dir1;
	::ZeroMemory(&dir1, sizeof(dir1));
	dir1.Type      = D3DLIGHT_DIRECTIONAL;
	dir1.Diffuse   = d3d::WHITE;
	dir1.Specular  = d3d::WHITE;
	dir1.Ambient   = d3d::WHITE;
	dir1.Direction = D3DXVECTOR3( -1.0f, 0.0f, 0.0f );


	//
	// Set and Enable the light.
	//
	m_device->SetLight(0, &dir0);
	m_device->LightEnable(0, true);

	m_device->SetLight(1, &dir1);
	m_device->LightEnable(1, true);



	//
	// Turn on specular lighting and instruct Direct3D
	// to renormalize normals.
	//
	m_device->SetRenderState( D3DRS_NORMALIZENORMALS, true );
	m_device->SetRenderState( D3DRS_SPECULARENABLE, true );

	m_device->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
}

void Device::InitTransformMatrix()
{
	//
	// initial transform matrixes
	//
	D3DXMATRIX world;
	D3DXMatrixIdentity( &world );
	m_device->SetTransform( D3DTS_WORLD, &world );
	

	float distance = 100.0f;
	float tg = -10.0f / distance;

	D3DXVECTOR3 pos(3.0f, 0.0f, -distance);
	D3DXVECTOR3 target(3.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
	D3DXMATRIX V;
	D3DXMatrixLookAtLH(&V, &pos, &target, &up);
	m_device->SetTransform(D3DTS_VIEW, &V);


	D3DXMATRIX proj(m_ratio / m_width,					0,			   0,				0,
									0, m_ratio / m_height,			   0,				0,
								   tg,				    0, 1.0f/ 1000.0f,				0,
					   -tg * distance,				    0,			   0,				1);
	m_device->SetTransform( D3DTS_PROJECTION, &proj );
}


bool Device::Setup( DWORD numOfTaperSector, float ratio )
{
	// setup parameters, render state, transform matrixes and meshes, and so on

	m_numOfTaperSector = numOfTaperSector;
	m_ratio = ratio;

	m_itemsTextureWidth = 100 / 9.6f;
	m_itemsTextureHeigth = 20;
	
	InitTaperModel();
	InitRectangleModel();

	InitRenderState();
	InitTransformMatrix();
	
	CalculateDirectoryGraph( &m_vslDirectory, &D3DXVECTOR3( 0, 0, 0 ) );
	CalculateItemGraph( &m_vslDirectory );
	
	return true;
}

void Device::CalculateDirectoryGraph( VisualDirectory* pVisualDirectory, D3DXVECTOR3* pos )
{
	assert(pVisualDirectory != NULL && pos != NULL );
	
	int itemNum = pVisualDirectory->m_vslItemQueue.m_num;
	if( itemNum == 0 ) 
	{
		return;
	}
	// calculate member variable

	float itemHeight = ITEM_HEIGHT;
	float spaceHeight = SPACE_HEIGHT;
	float perimeter = itemNum * ( itemHeight + spaceHeight );

	pVisualDirectory->m_graph.m_position = *pos;
	pVisualDirectory->m_graph.m_height = 0.2f + ( itemNum / 50.0f );

	float radius = perimeter / ( 2 * D3DX_PI );
	pVisualDirectory->m_graph.m_radius = radius + 0.01f;
	pVisualDirectory->m_graph.m_angle = 0;
}

void Device::CalculateItemGraph( VisualDirectory* pVisualDirectory )
{
	assert( pVisualDirectory != NULL );

	int itemNum = pVisualDirectory->m_vslItemQueue.m_num;
	if( itemNum == 0 )
	{
		return;
	}

	// calculate member variable
	D3DXMATRIX rx;
	float angle = ( 2 * D3DX_PI ) / itemNum;
	D3DXMatrixRotationX( &rx, angle );
	D3DXVECTOR3 pos( pVisualDirectory->m_graph.m_height , 0, -pVisualDirectory->m_graph.m_radius );
	
	int i = 0;
	ItemNode* temp = pVisualDirectory->m_vslItemQueue.m_head;
	while( temp != pVisualDirectory->m_vslItemQueue.m_tail->m_next )
	{
		DWORD num = strlen( temp->m_vslItem.m_item.GetName());
		temp->m_vslItem.m_graph.m_position = pos + pVisualDirectory->m_graph.m_position;
		
		temp->m_vslItem.m_graph.m_height = 0.1f;
		temp->m_vslItem.m_graph.m_width = 0.1f * num;
		temp->m_vslItem.m_graph.m_fontWidth =  10;
		temp->m_vslItem.m_graph.m_angle = angle * (i++);


		// create font
// 		LOGFONT lf;
// 		ZeroMemory(&lf, sizeof(LOGFONT));
// 
// 		lf.lfHeight         = m_itemsTextureHeigth;    // in logical units
// 		lf.lfWidth          = temp->m_vslItem.m_graph.m_fontWidth;    // in logical units
// 		lf.lfEscapement     = 0;
// 		lf.lfOrientation    = 0;
// 		lf.lfWeight         = 1000;   // boldness, range 0(light) - 1000(bold)
// 		lf.lfItalic         = false;
// 		lf.lfUnderline      = false;
// 		lf.lfStrikeOut      = false;
// 		lf.lfCharSet        = DEFAULT_CHARSET;
// 		lf.lfOutPrecision   = 0;              
// 		lf.lfClipPrecision  = 0;          
// 		lf.lfQuality        = 0;           
// 		lf.lfPitchAndFamily = 0;           
// 		strcpy(lf.lfFaceName, "Courier New"); // font style

		D3DXFONT_DESC lf;
		ZeroMemory(&lf, sizeof(D3DXFONT_DESC));

		lf.Height         = m_itemsTextureHeigth;    // in logical units
		lf.Width          = temp->m_vslItem.m_graph.m_fontWidth;    // in logical units
		lf.MipLevels     = 1;
		lf.Italic    = 0;
		lf.Weight         = 1000;   // boldness, range 0(light) - 1000(bold)
		lf.Italic         = false;
		lf.CharSet        = DEFAULT_CHARSET;
		lf.OutputPrecision   = 0;              
		lf.Quality        = 0;           
		lf.PitchAndFamily = 0;           
		strcpy(lf.FaceName, "Courier New"); // font style
		

		if( temp->m_vslItem.m_graph.m_font != NULL )
		{
			d3d::Release<ID3DXFont*>( temp->m_vslItem.m_graph.m_font );
			temp->m_vslItem.m_graph.m_font = NULL;
		}
		if(FAILED( D3DXCreateFontIndirect( m_device, &lf, &temp->m_vslItem.m_graph.m_font ) ) )
		{
			::MessageBox(0, "D3DXCreateFontIndirect() - FAILED", 0, 0);
			::PostQuitMessage(0);
		}
		

		// create texture
		if(FAILED(D3DXCreateTexture(m_device, 
								m_itemsTextureWidth * num, m_itemsTextureHeigth, 
								1, D3DUSAGE_RENDERTARGET, 
								D3DFMT_UNKNOWN, 
								D3DPOOL_DEFAULT, 
								&temp->m_vslItem.m_graph.m_texture )))
		{
			MessageBox(0,"fail to create renderTarget!", 0, 0);
			return;
		}
		
		// draw to the texture
		IDirect3DSurface9*	renderSurface = NULL;
		int result = temp->m_vslItem.m_graph.m_texture->GetSurfaceLevel( 0, &renderSurface );
		if( result != D3D_OK )
		{
			return;
		}
		
		IDirect3DSurface9*	oldSurface = NULL;
		result = m_device->GetRenderTarget( 0, &oldSurface );
		if( result != D3D_OK )
		{
			return;
		}
		result = m_device->SetRenderTarget( 0, renderSurface );
		if( result != D3D_OK )
		{
			return;
		}

		m_device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffffffff, 1.0f, 0);

		RECT rect = { 0, 0, m_width, m_height };
		
		//draw the text
		m_device->BeginScene();

// 		temp->m_vslItem.m_graph.m_font->DrawText(
// 				( temp->m_vslItem.m_item.GetName() ), 
// 				-1,
// 				&rect,
// 				DT_TOP | DT_LEFT,
// 				0xff000000); 

		temp->m_vslItem.m_graph.m_font->DrawText(NULL,
			( temp->m_vslItem.m_item.GetName() ), 
			-1,
			&rect,
			DT_TOP | DT_LEFT,
			0xff000000); 

		m_device->EndScene();



		m_device->SetRenderTarget( 0, oldSurface );
		
		d3d::Release<IDirect3DSurface9*>( renderSurface );


		D3DXVec3TransformCoord( &pos, &pos, &rx );
		temp = temp->m_next;
	}
}

void Device::DrawDirectory( VisualDirectory* pVisualDirectory )
{
	assert( pVisualDirectory != NULL );
	
	// draw taper
	DrawTaper( pVisualDirectory );

	// draw items
	ItemNode* temp = pVisualDirectory->m_vslItemQueue.m_head;
	while( temp != pVisualDirectory->m_vslItemQueue.m_tail->m_next )
	{
		DeviceDrawItem( &temp->m_vslItem );
		temp = temp->m_next;
	}
}



void Device::DrawTaper( VisualDirectory* pVisualDirectory )
{	
	assert( pVisualDirectory != NULL );

	D3DXMATRIX oldWorld;
	m_device->GetTransform( D3DTS_WORLD, &oldWorld );
	// calculate the world matrix
	D3DXMATRIX world;
	D3DXMATRIX rx;
	D3DXMatrixRotationX( &rx, pVisualDirectory->m_graph.m_angle );

	D3DXMATRIX t;
	D3DXMatrixTranslation( &t,  pVisualDirectory->m_graph.m_position.x, 
								pVisualDirectory->m_graph.m_position.y, 
								pVisualDirectory->m_graph.m_position.z );
	D3DXMATRIX s0;
	D3DXMatrixScaling( &s0, 1,	pVisualDirectory->m_graph.m_radius, 
								pVisualDirectory->m_graph.m_radius );
	D3DXMATRIX s1;
	D3DXMatrixScaling( &s1, pVisualDirectory->m_graph.m_height, 1, 1 );

	world = rx * s1 * s0 * t;
	
	// set world matrix
	m_device->SetTransform( D3DTS_WORLD, &( world ) );
	
	// set material
	D3DMATERIAL9 mtrl;
	mtrl.Ambient  = d3d::BLACK;
	mtrl.Diffuse  = d3d::CYAN;
	mtrl.Specular = d3d::BLACK;
	mtrl.Emissive = d3d::BLACK;
	mtrl.Power    = 5.0f;
	m_device->SetMaterial(&mtrl);

	// drew taper
	m_device->BeginScene();
	m_device->SetStreamSource(0, m_taper, 0, sizeof( Vertex ) );
	m_device->SetFVF(Vertex::FVF);
	m_device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, m_numOfTaperSector * 2 );
	m_device->EndScene();

	m_device->SetTransform( D3DTS_WORLD, &oldWorld );
}



void Device::DeviceDrawItem( VisualItem* pVisualItem )
{
	assert( pVisualItem != NULL );
	
	// draw rectangle
	DrawRectangle( pVisualItem );
	
	
	if( pVisualItem->IsHasItem() )
	{
		// draw sub directory
		DrawDirectory( pVisualItem->m_pVslDirectory );
	}
}

void Device::DrawRectangle( VisualItem* pVisualItem )
{
	assert( pVisualItem != NULL );

	IDirect3DBaseTexture9* tempTexture;
	m_device->GetTexture( 0, &tempTexture );

	//set texture
	m_device->SetTexture( 0, pVisualItem->m_graph.m_texture );

	// calculate world matrix
	D3DXMATRIX oldWorld;
	m_device->GetTransform( D3DTS_WORLD, &oldWorld );
	
	D3DXMATRIX sxy;
	D3DXMatrixScaling( &sxy, pVisualItem->m_graph.m_width, pVisualItem->m_graph.m_height, 1 );

	D3DXMATRIX rx;
	D3DXMatrixRotationX( &rx, pVisualItem->m_graph.m_angle );

	D3DXMATRIX t;
	D3DXMatrixTranslation( &t,  pVisualItem->m_graph.m_position.x,
									pVisualItem->m_graph.m_position.y, 
									pVisualItem->m_graph.m_position.z );

	//set world matrix
	D3DXMATRIX world = sxy * rx * t;
	m_device->SetTransform( D3DTS_WORLD, &world );
	
	// set sampler state
	m_device->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	m_device->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	m_device->SetSamplerState( 0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR );
	

	// set material
	D3DMATERIAL9 mtrl;
	if( pVisualItem->IsDirectory() )
	{
		mtrl.Ambient  = d3d::GREY;

		if( pVisualItem->HadBeOpened() )
		{
			if( pVisualItem->IsNoItem() )
			{
				mtrl.Diffuse = d3d::BLUE;
			}
			else
			{
				mtrl.Diffuse  = d3d::RED;
			}
		}
		else
		{
			mtrl.Diffuse  = d3d::YELLOW;	
		}

		mtrl.Specular = d3d::BLACK;
		mtrl.Emissive = d3d::BLACK;
		mtrl.Power    = 5.0f;
	}
	else
	{
		mtrl.Ambient  = d3d::GREY;
		mtrl.Diffuse  = d3d::WHITE;
		mtrl.Specular = d3d::BLACK;
		mtrl.Emissive = d3d::BLACK;
		mtrl.Power    = 5.0f;
	}
	m_device->SetMaterial( &mtrl );

	// draw rectangle
	m_device->BeginScene();
	m_device->SetStreamSource( 0, m_rectangle, 0, sizeof( Vertex ) );
	m_device->SetFVF( Vertex::FVF );
	m_device->DrawPrimitive( D3DPT_TRIANGLELIST, 0, 2 );
	m_device->EndScene();

	m_device->SetTransform( D3DTS_WORLD, &oldWorld );

	m_device->SetTexture( 0, tempTexture );
}


bool Device::Display()
{
	if( m_device )
	{
		m_device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x0, 1.0f, 0);
		
		// draw root directory
		DrawDirectory( &m_vslDirectory );

		m_device->Present(0, 0, 0, 0);
	}
	
	return true;
}





d3d::Ray Device::CalcPickingRay( float x, float y )
{
	D3DVIEWPORT9 vp;
	m_device->GetViewport(&vp);
	D3DXMATRIX viewport(		vp.Width / 2.0f,						0,					0, 0, 
											  0,	    vp.Height / -2.0f,					0, 0, 
											  0,						0,  vp.MaxZ - vp.MinZ, 0, 
						 vp.X + vp.Width / 2.0f,  vp.Y + vp.Height / 2.0f,			  vp.MinZ, 1);
	D3DXMATRIX proj;
	m_device->GetTransform(D3DTS_PROJECTION, &proj);
	
	d3d::Ray ray;
	ray._origin = D3DXVECTOR3( x, y, 0 );
	ray._direction = D3DXVECTOR3( 0, 0, 1 );

 	D3DXMatrixInverse( &viewport, NULL,	&viewport );
	D3DXMatrixInverse( &proj, NULL, &proj );
	
	TransformRay( &ray, &( viewport * proj ) );
	
	return ray;
}

void Device::TransformRay(d3d::Ray* ray, D3DXMATRIX* T )
{
	// transform the ray's origin, w = 1.
	D3DXVec3TransformCoord(
		&ray->_origin,
		&ray->_origin,
		T);

	// transform the ray's direction, w = 0.
	D3DXVec3TransformNormal(
		&ray->_direction,
		&ray->_direction,
		T);

	// normalize the direction
	D3DXVec3Normalize(&ray->_direction, &ray->_direction);
}


float Device::CalculateRayDistance( d3d::Ray* ray0, d3d::Ray* ray1 )
{
	// return the distance of rays
	D3DXVECTOR3 v0 = ray0->_origin - ray1->_origin;
	D3DXVECTOR3 v1;
	D3DXVec3Cross( &v1, &ray0->_direction, &ray1->_direction );
	return D3DXVec3Dot( &v0, &v1 ) / D3DXVec3Length( &v1 );	
}


bool Device::TaperPickTest( VisualDirectory* vd, d3d::Ray* ray, bool* isDir, void** vp, float* k )
{
	d3d::Box box;
	box.m_pos0 = D3DXVECTOR3( vd->m_graph.m_position.x, vd->m_graph.m_position.y + vd->m_graph.m_radius, vd->m_graph.m_position.z + vd->m_graph.m_radius );
	box.m_pos1 = D3DXVECTOR3( vd->m_graph.m_position.x + vd->m_graph.m_height, vd->m_graph.m_position.y - vd->m_graph.m_radius, vd->m_graph.m_position.z - vd->m_graph.m_radius );
	
	// simple picking test using box
	if( box.IsRayInside( ray ) )
	{
		//picking test using taper
		float xt = ray->_origin.x - vd->m_graph.m_position.x;
		float xd = ray->_direction.x;

		float yt = ray->_origin.y - vd->m_graph.m_position.y;
		float yd = ray->_direction.y;
		
		float zt = ray->_origin.z - vd->m_graph.m_position.z;
		float zd = ray->_direction.z;
		
		float m = ( vd->m_graph.m_radius * vd->m_graph.m_radius ) / ( vd->m_graph.m_height * vd->m_graph.m_height );

		float a = yd * yd + zd * zd - m * xd * xd;
		float b = 2 * ( yt * yd + zt * zd - m * xt * xd );
		float c = yt * yt + zt * zt - m * xt * xt;
		float d = b * b - 4 * a * c;

		if( d >= 0 ) 
		{	
			float sd = sqrt( d );
			
			float tk[2] = { ( - b + sd ) / ( 2 * a ), ( - b - sd ) / ( 2 * a ) };

			for( int i = 0; i < 2; i++ )
			{
				if( tk[i] >= 0 )
				{
					D3DXVECTOR3 p0 = ray->_origin + tk[i] * ray->_direction;
					
					D3DXVECTOR3 v = p0 - vd->m_graph.m_position;

					float R = sqrt( v.y * v.y + v.z * v.z );

					if( R <= vd->m_graph.m_radius   && 
						p0.x >= vd->m_graph.m_position.x && 
						p0.x <= ( vd->m_graph.m_position.x + vd->m_graph.m_height ) )
					{
						if( *k < 0 || tk[i] < *k )
						{
							*isDir = true;
							*vp = (void*)vd;
							*k = tk[i];
						}
					}
				}
			}
		}
	}

	// picking test using every item
	ItemNode* current = vd->m_vslItemQueue.m_head;
	while(  current != NULL )
	{
		RectanglePickTest( &current->m_vslItem, ray, isDir, vp, k );
		current = current->m_next;
	}

	if( *k > 0 ) return true;
	return false;
}



bool Device::RectanglePickTest( VisualItem* pVisualItem, d3d::Ray* ray, bool* isDir, void** vp, float* pk )
{
	float angle = pVisualItem->m_graph.m_angle;
	float height = pVisualItem->m_graph.m_height;
	float width = pVisualItem->m_graph.m_width;
	D3DXVECTOR3 pos = pVisualItem->m_graph.m_position;
	
	
	if( ( angle >= 0.0f && angle <= D3DX_PI / 2.0f ) || ( angle >= 3 * D3DX_PI / 2.0f && angle <= 2 * D3DX_PI ) )
	{
		//transform the rectangle to world space
		D3DXVECTOR3 p[4];
		p[0] = D3DXVECTOR3( 0,  1.0f, 0 );
		p[1] = D3DXVECTOR3( 1.0f,  1.0f, 0 );
		p[2] = D3DXVECTOR3( 1.0f, -1.0f, 0 );
		p[3] = D3DXVECTOR3( 0, -1.0f, 0 );
		
		D3DXMATRIX t, rx;
		D3DXMATRIX sxy;
		D3DXMatrixScaling( &sxy, width, height, 1 );

		D3DXMatrixTranslation( &t, pos.x, pos.y, pos.z );
		D3DXMatrixRotationX( &rx, angle );
		
		D3DXMATRIX world = sxy * rx * t;
		for( int i = 0; i < 4; i++ )
		{
			D3DXVec3TransformCoord( &p[i], &p[i], &world );	
		}
		
		
		// calculate the normal
		D3DXVECTOR3 n;
		D3DXVECTOR3 v0 = p[1] - p[0], v1 = p[3] - p[0];

		D3DXVec3Cross( &n, &v0, &v1 );
		
		float xd = ray->_direction.x;
		float yd = ray->_direction.y;
		float zd = ray->_direction.z;
		
		float xo = ray->_origin.x;
		float yo = ray->_origin.y;
		float zo = ray->_origin.z;
		
		float x1 = p[1].x;
		float y1 = p[1].y;
		float z1 = p[1].z;

		float A = n.x;
		float B = n.y;
		float C = n.z;

		float k = - ( A * ( xo - x1 ) + B * ( yo - y1 ) + C * ( zo - z1 ) ) /
					( A * xd + B * yd + C * zd );
		

		if( k >= 0 )
		{
			// pick test
			// transfrom the ray to the space of rectangle's coordinate
			D3DXVECTOR3 s = ray->_origin + k * ray->_direction;

			D3DXMATRIX ct;
			D3DXMatrixTranslation( &ct, -p[0].x, -p[0].y, -p[0].z );

			D3DXMATRIX c( v0.x, v0.y, v0.z, 0, 
						  v1.x, v1.y, v1.z, 0, 
						   n.x,  n.y,  n.z, 0, 
						  0,	0,	  0,	1);
			D3DXMatrixInverse( &c, 0, &c );

			D3DXVec3TransformCoord( &s, &s, &( ct * c ) );

			if( s.x >= 0 && s.x <= 1.0f && s.y >= 0 && s.y <= 1.0f )
			{
				if(*pk < 0 || k < *pk )
				{
					*isDir = false;
					*vp = ( void* )pVisualItem;
					*pk = k;
				}
			}
		}
	}
	
	// if had be opened, picking test
	if( pVisualItem->HadBeOpened() )
	{
		TaperPickTest( pVisualItem->m_pVslDirectory, ray, isDir, vp, pk );
	}
	
	if( *pk > 0 ) return true;
	return false;
}


void Device::ItemRevolution( VisualItem* vi, D3DXVECTOR3* center, float angle )
{
	//calculate the transform matrixes
	D3DXMATRIX t, rt, rx;
	D3DXMatrixTranslation( &t, center->x, center->y, center->z );
	D3DXMatrixRotationX( &rx, angle );

	D3DXMatrixInverse(&rt, 0, &t );

	//transformCoord the position
	D3DXVec3TransformCoord( &vi->m_graph.m_position, &vi->m_graph.m_position, &( rt * rx * t ) );

	vi->m_graph.m_angle += angle; // add angle

	// must 0 < angle < 2 * PI
	while( vi->m_graph.m_angle < 0 )
	{
		vi->m_graph.m_angle += 2 * D3DX_PI;
	}
	while( vi->m_graph.m_angle > 2 * D3DX_PI )
	{
		vi->m_graph.m_angle -= 2 * D3DX_PI;
	}
	
	// subitems revolution 
	if( vi->IsHasItem() )
	{
		DirectoryRevolution( vi->m_pVslDirectory, center, angle );
	}
}

void Device::DirectoryRevolution( VisualDirectory* vd, D3DXVECTOR3* center, float angle )
{
	//calculate the transform matrixes
	D3DXMATRIX t, rt, rx;
	D3DXMatrixTranslation( &t, center->x, center->y, center->z );
	D3DXMatrixRotationX( &rx, angle );

	D3DXMatrixInverse(&rt, 0, &t );
	
	//transformCoord the position
	D3DXVec3TransformCoord( &vd->m_graph.m_position, &vd->m_graph.m_position, &( rt * rx * t ) );
	
	vd->m_graph.m_angle += angle; // add angle
	// must 0 < angle < 2 * PI
	while( vd->m_graph.m_angle < 0 )
	{
		vd->m_graph.m_angle += 2 * D3DX_PI;
	}
	while( vd->m_graph.m_angle > 2 * D3DX_PI )
	{
		vd->m_graph.m_angle -= 2 * D3DX_PI;
	}
	
	// subitems revolution 
	ItemNode* temp = vd->m_vslItemQueue.m_head;
	while( temp != NULL )
	{
		ItemRevolution( &temp->m_vslItem, center, angle );
		temp = temp->m_next;
	}
}

void Device::DirectoryRotation( VisualDirectory* vd, float angle )
{
	// actually, the same as revolution
	Device::DirectoryRevolution( vd, &vd->m_graph.m_position, angle );
}

// window process 
LRESULT CALLBACK Device::WndProcEx(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch( msg )
	{
	case WM_DESTROY:
		::PostQuitMessage(0);
		break;
	
	case WM_PAINT:
		{
			PAINTSTRUCT ps = {0};
			BeginPaint( hWnd, &ps );

			if( m_device != NULL )
			{
				Display();
			}

			EndPaint( hWnd, &ps );
		}
		break;

	case WM_KEYDOWN:
		switch( wParam )
		{
		case VK_SPACE:
			{
				// rotation PI
				float angle = 0;
				float delta = 0.125;
				
				while( ( angle  + delta ) < D3DX_PI )
				{
					DirectoryRotation( &m_vslDirectory, delta );
					Display();
					angle += delta;
				}
	
				DirectoryRotation( &m_vslDirectory, D3DX_PI - angle );
				Display();
			}
			break;

		case VK_RETURN:
			{
				// change the angle of projection
				D3DXMATRIX proj;
				m_device->GetTransform( D3DTS_PROJECTION, &proj );
				

				float temp = -proj( 2, 0 );
				
				float rate = proj( 3, 0 ) / proj( 2, 0 );
				float delta = fabs( proj( 2, 0 ) ) * 0.2f * ( temp < 0 ? -1 : 1 );
				
				while( fabs( proj( 2, 0 ) + delta )  < fabs( temp ) || -temp == proj( 2, 0 ) )
				{
					proj( 2, 0 ) += delta;
					proj( 3, 0 ) = rate * proj( 2, 0 );
					m_device->SetTransform( D3DTS_PROJECTION, &proj );
					Display();
				}

				proj( 2, 0 ) = temp;
				proj( 3, 0 ) = rate * proj( 2, 0 );
				m_device->SetTransform( D3DTS_PROJECTION, &proj );
				Display();
			}
			break;

		case VK_ESCAPE:
			{
				::DestroyWindow(hWnd);
			}
			break;

		default:
			{
				Display();
			}
			break;
		}
		break;
			 
		case WM_MOUSEWHEEL:
			{
				// scale the projection
				short ro = (short)HIWORD( wParam );
				D3DXMATRIX proj, scaling;
				m_device->GetTransform( D3DTS_PROJECTION, &proj );
				
				ro /= WHEEL_DELTA;
				D3DXMatrixScaling( &scaling, 1 + ro * 0.1f, 1 + ro * 0.1f, 1.0f );
				m_device->SetTransform( D3DTS_PROJECTION, &( proj * scaling) );
				Display();
			}
			break;
	
	case WM_RBUTTONDOWN:
		{
			GetCursorPos( &m_firstPos );
			m_secondPos = m_firstPos;

			SetCapture( hWnd );

			m_isMoving = true;
			m_isRightButton = true;
		}
		break;

	case WM_LBUTTONDOWN:
		{
			// Calculate the ray in view space given the clicked screen point
			d3d::Ray ray = CalcPickingRay(LOWORD(lParam), HIWORD(lParam));

			// transform the ray to world space
			D3DXMATRIX view;
			m_device->GetTransform(D3DTS_VIEW, &view);

			D3DXMATRIX viewInverse;
			D3DXMatrixInverse(&viewInverse,	0, &view);
			
			// transformRay the ray to world space
			TransformRay(&ray, &viewInverse);
			
			bool isDir = false;
			void* itemOrDir = NULL;
			float k = -1;

			m_isRightButton = false;
			
			// picking test
			if( TaperPickTest( &m_vslDirectory, &ray, &isDir, &itemOrDir, &k ) )
			{
				if( isDir == true )
				{
					// click the taper, denote the directory
					m_pickingVslDir = ( VisualDirectory* )itemOrDir;
					
					GetCursorPos( &m_firstPos );
					m_secondPos = m_firstPos;

					SetCapture( hWnd );
					m_isMoving = true;
				}
				else
				{
					// click the rectangle, denote the item
					VisualItem* vi = ( VisualItem* )itemOrDir;
					if(vi->IsDirectory() )
					{
						
						if( vi->CanBeOpened() )
						{
							// open it
							vi->Open();
							D3DXVECTOR3 v(vi->m_graph.m_position.x + vi->m_graph.m_width, vi->m_graph.m_position.y, vi->m_graph.m_position.z );
							CalculateDirectoryGraph( vi->m_pVslDirectory, &v );
							CalculateItemGraph( vi->m_pVslDirectory );
							Display();
						}
						else if( vi->HadBeOpened() )
						{
							//close it
							vi->m_hadBeOpened = false;
							delete vi->m_pVslDirectory;
							vi->m_pVslDirectory = NULL;
							Display();
						}
					}
					else
					{
						// open the file
						ShellExecute( 0, "open", vi->m_item.m_path, NULL, NULL, SW_SHOWNORMAL );
					}
				}
			}
			else
			{
				m_isMoving = true;
				GetCursorPos( &m_firstPos );
				m_secondPos = m_firstPos;
				
				SetCapture( hWnd );
			}
		}
		break;

	case WM_MOUSEMOVE:
		{
			if( m_isMoving )
			{
				GetCursorPos( &m_firstPos );
 
				if( m_isRightButton )
				{	
					// change the angle of projection
					float delta = -( m_firstPos.x - m_secondPos.x ) / 500.0f;
					D3DXMATRIX proj;
					m_device->GetTransform( D3DTS_PROJECTION, &proj );

					float rate = proj( 3, 0 ) / proj( 2, 0 );
					proj( 2, 0 ) += ( delta );
					proj( 3, 0 ) = rate * proj( 2, 0 );
					m_device->SetTransform( D3DTS_PROJECTION, &proj );
				}
				else if( m_pickingVslDir != NULL )
				{
					// rotate the directory
					float angle = -( m_firstPos.y - m_secondPos.y ) / 50.0f;
					DirectoryRotation( m_pickingVslDir, angle );
				}
				else
				{
					// translate the view
					D3DXMATRIX V;
					m_device->GetTransform(D3DTS_VIEW, &V);
					D3DXMATRIX T;
					D3DXMatrixTranslation( &T, ( m_firstPos.x - m_secondPos.x ) / 50.0f, ( -m_firstPos.y + m_secondPos.y ) / 50.0f, 0 );
					m_device->SetTransform( D3DTS_VIEW, &( T * V ) );
				}

				m_secondPos = m_firstPos;
				Display();
			}
		}
		break;

	case WM_RBUTTONUP:
	case WM_LBUTTONUP:
		{
			if( m_isMoving = true )
			{
				SetCapture( m_oldHwnd );
				m_isMoving = false;

				if( m_pickingVslDir != NULL )
				{
					m_pickingVslDir = NULL;
				}
			}
		}
		break;

	default:
		return ::DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return false;
}


// window process
LRESULT CALLBACK Device::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	Device* device = ( Device* )GetWindowLong( hwnd, GWL_USERDATA );
	if( device != NULL )
	{
		return device->WndProcEx( hwnd, msg, wParam, lParam );
	}
	return ::DefWindowProc(hwnd, msg, wParam, lParam);
}



