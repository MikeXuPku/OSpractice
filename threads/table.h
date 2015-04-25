// Table.h 
//	Data structures for table which may be used for managing thread messages
//
//liyou xu

#ifndef TABLE_H
#define TABLE_H

class Table{
public:
	Table(int size);
	~Table();
	int Alloc(void *object);
	void * getByIndex(int index);
	void release(int index);
private:
	void ** table_head;
	int size_;
};

#endif //TABLE_H
