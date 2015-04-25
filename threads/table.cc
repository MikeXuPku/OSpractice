//table.cc
//iplementation of tabel class

#include "table.h"
#include "utility.h"

//construction of Table
Table::Table(int size)
{
	size_ = size;
	table_head = new void*[size_];
	for(int i=0; i<size_; ++i)table_head[i]=NULL;  //initialize table array
}

//deconstruction of Table
Table::~Table()
{
	delete[] table_head;
}

//put object into table
int Table::Alloc(void *object)
{
	for(int i=0; i<size_; ++i)
		if(table_head[i] == NULL){
			table_head[i] = object;
			return i;
		}
	return -1;    //if not find a empty slot, then return -1	
}

//get a object pointer by index
void * Table::getByIndex(int index)
{
//if table[index] do not exist, then will return NULL
	if(index<0 || index>=size_) return NULL;
	else return table_head[index]; 
}

//release a object pointer from table
void Table::release(int index)
{
	ASSERT((index>=0 && index<size_));
	table_head[index]=NULL;
}
