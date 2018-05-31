#ifndef VISUAL_H
#define VISUAL_H


// include some header file
#include <windows.h>
#include <iostream>
#include <string>
#include <assert.h>
#include <stdlib.h>
#include <d3dx9.h>
#include <string.h>

#include "d3dUtility.h"


// declare some classes 

class Item;			// use to describe the information of items, which is file or subdirectory, belong to current directory
class Directory;	// use to describe the information of directorys

class ItemGraph;	// use to describe the information of graph of items
class DirectoryGraph; // use to describe the information of graph of directorys

class VslItemNode;	// the node of the VslItemQueue, use to contact the VisualItems in the VslItemQueue
class VslItemQueue; // a queue, use to combine the VisualItems which belong to the current directory

class VisualItem;	// use to combine Item and ItemGraph, and if the Item is a directory then contact the information of the directory
class VisualDirectory;	// use to combine Directory, DirectoryGraph and the Items which belong to the directory


//use std namespace
using namespace std;




//
// Item
//
class Item
{
public:
	char*	m_path;			// describe the full path of the Item, include the name at tail
	DWORD	m_pathLen;		// the length of m_path
	DWORD	m_nameIndex;	// the index of m_path, describe where is the name in the m_path
	char	m_type;			// if m_type is 'f' then the item is file. else if is 'd', directory


	Item( void );			// construction
	Item( const char* path, char type );	// construction

	~Item( void );			// destruction
	
	char* GetName();		// just as its name implies

	int ItemCmp( Item* item );	// compare the items using GetName(), if is Equal then return 0

	bool IsDirectory( void );	// if the item is directory return true, else return false

	DWORD GetNameIndex( void );	// just as its name implies
};





//
// Directory
//
class Directory
{
public:
	char*	m_path;			// describe the full path of the directory, include the name at tail
	DWORD	m_pathLen;		// the length of m_path
	DWORD	m_nameIndex;	// the index of m_path, describe where is the name in the m_path
	
	Directory( void );		// construction
	Directory( const char* path );	

	~Directory( void );		// destruction

	DWORD GetNameIndex( void ); // just as its name implies
};




//
// ItemGraph
//
class ItemGraph
{
public:
	D3DXVECTOR3	m_position;		// position of the rectangle graph
	ID3DXFont*	m_font;			// font of the rectangle graph
	IDirect3DTexture9* m_texture;	// texture of the rectangle graph


	float	m_angle;			// the rotaton angle of the rectangle graph
	float	m_width;			// the width of the rectangle graph
	float	m_height;			// the height of the the rectangle graph
	float	m_fontWidth;		// the width of m_font
	
	ItemGraph();				// construction
	~ItemGraph( void );			// destruction
};




//
// DirectoryGraph
//
class DirectoryGraph
{
public:
	D3DXVECTOR3	m_position;		// position of the taper graph
	float	m_angle;			// the rotation angle of the taper graph
	float	m_radius;			// the radius of the bottom of the taper graph
	float	m_height;			// the radius of the taper graph
	DirectoryGraph();			// construction
};






//
// VisualItem
//
class VisualItem				
{
public:
	Item			m_item;		// include the information of the item
	ItemGraph		m_graph;	// include the information of the graph

	bool			m_hadBeOpened;	// if the item is a directory and had opened, then current variable is true
	VisualDirectory* m_pVslDirectory;	// if the item is a directory and had opened, then current pointer point to a VisualDirectory Object

	VisualItem( void );			// construction
	VisualItem( const char* name, char type );	//construction
	~VisualItem( void );		// destruction
	
	bool CanBeOpened( void );	// if current visual item can be Opened, return true
	bool HadBeOpened( void );	// if current visual item had be opened, return true
	
	bool IsDirectory( void );	// if current visual item is a directory, return true

	bool IsNoItem( void );		// if no items belong to current visual item, return true
	bool IsHasItem( void );		// if current visual item has sub items, return true

	int ItemCmp( VisualItem* visualItem );	// compare visual item, if equal then return true

	void Open( void );			// if current item is directory and can be opened, open it and create sub items
};



//
// ItemNode
//
class ItemNode
{
public:
	VisualItem	m_vslItem;		// visual item
	ItemNode*	m_next;			// point to the next item node in the VslItemQueue
	

	ItemNode( const char* name, char type );	// construction
	bool IsDirectory( void );					// if current item is a directory, return true
	
	int ItemCmp( ItemNode* itemNode );	// compare item using the name of item, if equel then return true

//	void Print( const char* prefix );
};




//
// VslItemQueue
//
class VslItemQueue
{
public:
	DWORD			m_num;		// the number of item nodes
	ItemNode*		m_head;		// the head of the queue
	ItemNode*		m_tail;		// the tail of the tail
	
	VslItemQueue( void );		// construction
	~VslItemQueue( void );		// destruction

	bool IsEmpty( void );		// if m_num is 0, which denote that the queue is empty, return true

	void AddItem( const char* itemName, char type ); // use name and type to add one item
	//void Print( const char* prefix );					
};





//
// VisualDirectory
//
class VisualDirectory			
{
public:
	Directory		m_directory;	// include the information of the directory
	DirectoryGraph	m_graph;		// include the information of the graph
	VslItemQueue	m_vslItemQueue;	// a queue, include the item nodes which belong to the directory
	
	
	
	VisualDirectory( void );		// construction
	VisualDirectory( const char* name );	// destruction
	
	void InitVslDirectoryUsingSections( void );	// initialize the directory using disk information

	bool IsRoot( string Path );		// if current directory is root, return true

	void CreateAllItems( void );		// create all items which belong to current directory
};






#endif