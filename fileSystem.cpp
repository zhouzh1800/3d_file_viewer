#include "fileSystem.h"




//
//Directory
//
Directory::Directory():
m_path(NULL),
m_nameIndex(0)
{}


Directory::Directory( const char* path )
{
	assert( path != NULL );
	
	m_pathLen = strlen( path ) + 1;
	m_path = new char[ m_pathLen ];
	
	
	memcpy( m_path, path, m_pathLen );
	m_nameIndex = GetNameIndex();
}

Directory::~Directory( void )
{
	if( m_path != NULL )
	{
		delete [] m_path;
	}
}

DWORD Directory::GetNameIndex()
{
	int i = m_pathLen - 1;

	while( m_path[i]==' ' && i >= 0 ) { i--; }	
	if( m_path[i] == '\\' )
	{
		i--;
	}
	for( ; i >=0 ; i-- )
	{
		if( m_path[i] == '\\' )
		{
			return i + 1;
		}
	}
	
	return i + 1;
}












//
//Item
//
Item::Item():
m_path(NULL)
{}

// use path and type to constuct
Item::Item( const char* path, char type )
{
	assert( path != NULL );
	
	m_pathLen = strlen( path ) + 1;
	m_path = new char[ m_pathLen ];	// the path
	
	memcpy( m_path, path, m_pathLen );
	m_nameIndex = GetNameIndex();	// name index, path[index] is actually the name

	m_type = type;					// item type
}

Item::~Item( void )
{
	if( m_path != NULL )
	{
		delete m_path;
	}
}

char* Item::GetName( void )
{	
	if( m_path != NULL )
	{
		return &m_path[ m_nameIndex ];		// return the address of the name
	}
	return NULL;

}

int Item::ItemCmp( Item* item )
{
	assert( item != NULL );
	
	// actually, file > directory
	if( IsDirectory() && !item->IsDirectory() ) return -1;
	if( !IsDirectory() && item->IsDirectory() ) return 1;
	
	// if the items is both of files or directory then using strcmp to compare
	return strcmp( GetName(), item->GetName() );
}

bool Item::IsDirectory( void )
{
	// use m_type to judge current item is directory or not
	return m_type == 'd' ? true : false;
}


DWORD Item::GetNameIndex( void )
{
	int i = m_pathLen - 1;
	
	while( m_path[i]=='\0' && i >= 0 ) { i--; }		// filter the '\0' charater
	while( m_path[i]==' ' && i >= 0 ) { i--; }		// filter the ' ' charater
	if( m_path[i] == '\\' )						// filter the '\\' charater
	{
		i--;
	}

	for( ; i >=0 ; i-- )						// find the first charater which is not '\\' charater
	{
		if( m_path[i] == '\\' )					// compare
		{
			return i + 1;						// return index
		}
	}
	
	return i + 1;								// beacuse the '\\' chararter is not be finded, return index which is actually 0
}









//
//ItemGraph
//
ItemGraph::ItemGraph()
{
	m_font = NULL;
	m_texture = NULL;
}

ItemGraph::~ItemGraph()
{
	// release the resources
	if( m_font != NULL )
	{
		d3d::Release< ID3DXFont* >( m_font );
	}

	if( m_texture != NULL )
	{
		d3d::Release< IDirect3DTexture9* >( m_texture );
	}
}








//
// VisualItem
//
VisualItem::VisualItem():
m_hadBeOpened( false ),
m_pVslDirectory( NULL )
{}


VisualItem::~VisualItem()
{
	if( m_pVslDirectory != NULL )
	{
		delete m_pVslDirectory;
	}
}


VisualItem::VisualItem( const char* name, char type ):
m_item( name, type ),
m_hadBeOpened( false ),
m_pVslDirectory( NULL )
{
	m_pVslDirectory = NULL;
}


int VisualItem::ItemCmp( VisualItem* visualItem )
{
	assert( visualItem != NULL );
	return m_item.ItemCmp( &visualItem->m_item );
}


// if current item can be opened, then create the items which belong to current item
void VisualItem::Open( void )
{
	if( CanBeOpened() )
	{
		m_pVslDirectory = new VisualDirectory( m_item.m_path );
	
		// create all items
		m_pVslDirectory->CreateAllItems();
	}
	// make the m_hadBeOpened to true, denote that current item had be opened
	m_hadBeOpened = true;
}


bool VisualItem::IsDirectory( void )
{
	return m_item.IsDirectory();
}

bool VisualItem::CanBeOpened( void )
{
	return IsDirectory() && m_hadBeOpened == false && m_pVslDirectory == NULL;
}

bool VisualItem::HadBeOpened( void )
{
	if( IsDirectory() && m_hadBeOpened == true )
	{
		return true;
	}
	return false;
}

bool VisualItem::IsNoItem( void )
{
	if( m_hadBeOpened && IsDirectory() )
	{
		if ( m_pVslDirectory->m_vslItemQueue.IsEmpty() )
		{
			return true;
		}
	}
	return false;
}

bool VisualItem::IsHasItem( void )
{
	if( m_hadBeOpened && IsDirectory()  )
	{
		if( !m_pVslDirectory->m_vslItemQueue.IsEmpty() )
		{
			return true;
		}
	}
	return false;
}






//
//ItemNode
//
ItemNode::ItemNode( const char* name, char type ):
m_vslItem( name, type )
{
	m_next = NULL;
}

int ItemNode::ItemCmp( ItemNode* itemNode )
{
	assert( itemNode != NULL );
	return  m_vslItem.ItemCmp( &itemNode->m_vslItem );
}
/*
void ItemNode::Print( const char* prefix )
{
	if( prefix != NULL )
	{
		cout << prefix;
	}
	cout << m_vslItem.m_item.m_path << endl;
	
	if( m_vslItem.IsHadOpened() )
	{
		string tab = "  ";
		if( prefix != NULL )
		{
			tab += prefix;
		}
		m_vslItem.m_pDirectory->m_vslItemQueue.Print( tab.c_str() );
	}
}
*/




//
//VslItemQueue
//
VslItemQueue::VslItemQueue()
{
	m_num = 0;
	m_head = m_tail = NULL;
}

VslItemQueue::~VslItemQueue()
{
	if( !IsEmpty() )
	{
		ItemNode* temp = NULL;
		// delete the item nodes object which belong to current queue and Directory
		for( ItemNode* current = m_head; current != NULL; current = temp )
		{
			temp = current->m_next;
			delete current;
		}
	}
}

bool VslItemQueue::IsEmpty()
{
	if( m_num == 0 && m_head == NULL && m_tail == NULL )
	{
		return true;
	}
	return false;
}

// add item using item name and type
void VslItemQueue::AddItem( const char* itemName, char type )
{
	assert( itemName != NULL );

	// if current queue is empty
	if( IsEmpty() )
	{
		m_head = m_tail = new ItemNode( itemName, type );
	}
	else 
	{
		ItemNode* node = new ItemNode( itemName, type );

		ItemNode* current = m_head;
		ItemNode* temp = m_head;
		
		// sort by the name and type of item
		while ( current != NULL )
		{
			if( node->ItemCmp( current ) < 0  )
			{
				//add before m_head
				if( current == m_head )
				{
					node->m_next = m_head;
					m_head = node;
				}
				else // add to middle
				{
					node->m_next = current;
					temp->m_next = node;
				}
				break;
			}
			temp = current;
			current = current->m_next;
		}
		
		// add after m_tail
		if( current == NULL )
		{
			m_tail->m_next = node;
			m_tail = node;
			node->m_next = NULL;
		}
	}
	m_num++;		// add the number
}

/*
void VslItemQueue::Print( const char* prefix )
{
	if( m_head == NULL )
	{
		return;
	}
	for( ItemNode* current = m_head; current != m_tail->m_next; current = current->m_next )
	{
		current->Print( prefix );
	}
}
*/




//
//VisualDirectory
//
VisualDirectory::VisualDirectory()
{}

VisualDirectory::VisualDirectory( const char* name ):
m_directory( name )
{
	
}

void VisualDirectory::InitVslDirectoryUsingSections()
{
	DWORD strLen = GetLogicalDriveStrings( 0, NULL );	// get the length of the information of sections 
	
	char* buffer = new char[ strLen ];
	GetLogicalDriveStrings( strLen, buffer );		// get the information of sections
	int i = 0;
	
	int sum = 0;			// use to denote the number of section
	i = 0;

	// compute sum
	while( i < strLen )
	{
		while( buffer[i] == '\0' && i < strLen ){ i++; }
		while( buffer[i] != '\0' && i < strLen ){ i++; }
		while( buffer[i] == '\0' && i < strLen ){ i++; }
		sum++;
	}
	
	char** diskNames = new char*[ sum ]; // use to save the address of the name of sections
	
	i = 0;
	DWORD k = 0;
	while( i < strLen )
	{
		while( buffer[ i ] == '\0' && i < strLen ){ i++; }
		diskNames[ k++ ] = &buffer[ i ];
		while( buffer[ i ] != '\0' && i < strLen ){ i++; }
		while( buffer[ i ] == '\0' && i < strLen ){ i++; }
	}

	// add section items to queue
	for( i = 0; i < sum; i++ )
	{
		m_vslItemQueue.AddItem( diskNames[i], 'd' );
	}

	delete [] diskNames;
	delete [] buffer;
}

bool VisualDirectory::IsRoot( string Path )
{
	string Root;
	Root = Path.at(0) + ":\\";
	if( Root == Path )
	{
		return true;
	}
	else
	{
		return false;
	}
}

void VisualDirectory::CreateAllItems( void )
{
	string Path = m_directory.m_path;

	string szFind;
	szFind = Path;
	if( !IsRoot( szFind ) )
	{
		szFind += "\\";
	}
	szFind += "*.*";

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile( szFind.c_str(), &FindFileData ); // find first file or directory
	if( hFind == INVALID_HANDLE_VALUE )	// don't has file
	{
		return;
	}
	

	// find all file and directory
	do
	{
		if( FindFileData.cFileName[0] == '.' ) // filter current level directory and parent directory
		{
		   continue;
		}

		char type = '\0';
		if( FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) // is a directory
		{
			type = 'd';
		}
		else	// is a file
		{
		   type = 'f';
		}

		string szFile;
		if(IsRoot( Path ) )
		{
			szFile=Path+FindFileData.cFileName;
		}
		else
		{
			szFile=Path+"\\"+FindFileData.cFileName;
		}
		
		// add item
		m_vslItemQueue.AddItem( szFile.c_str(), type );
	}
	while(FindNextFile( hFind, &FindFileData ) );	// find next file and directory
	
	// close the handle
	FindClose( hFind );
}





//
//DirectoryGraph
//
DirectoryGraph::DirectoryGraph() 
{}
 



// example, please compile and run it at console system
/*
void main()
{
	VisualDirectory vd;
	vd.InitVisualDirectoryUsedDisk();

	vd.m_vslItemQueue.m_head->m_vslItem.Open();
	vd.m_vslItemQueue.m_head->m_vslItem.m_pDirectory->m_vslItemQueue.m_head->m_next->m_vslItem.Open();
	vd.m_vslItemQueue.m_head->m_vslItem.m_pDirectory->m_vslItemQueue.m_head->m_next->m_vslItem.m_pDirectory->m_vslItemQueue.m_head->m_vslItem.Open();
	vd.m_vslItemQueue.Print( NULL );
}
*/