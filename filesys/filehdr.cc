// filehdr.cc 
//	Routines for managing the disk file header (in UNIX, this
//	would be called the i-node).
//
//	The file header is used to locate where on disk the 
//	file's data is stored.  We implement this as a fixed size
//	table of pointers -- each entry in the table points to the 
//	disk sector containing that portion of the file data
//	(in other words, there are no indirect or doubly indirect 
//	blocks). The table size is chosen so that the file header
//	will be just big enough to fit in one disk sector, 
//
//      Unlike in a real system, we do not keep track of file permissions, 
//	ownership, last modification date, etc., in the file header. 
//
//	A file header can be initialized in two ways:
//	   for a new file, by modifying the in-memory data structure
//	     to point to the newly allocated data blocks
//	   for a file already on disk, by reading the file header from disk
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#include "system.h"
#include "filehdr.h"

//----------------------------------------------------------------------
// FileHeader::Allocate
// 	Initialize a fresh file header for a newly created file.
//	Allocate data blocks for the file out of the map of free disk blocks.
//	Return FALSE if there are not enough free blocks to accomodate
//	the new file.
//
//	"freeMap" is the bit map of free disk sectors
//	"fileSize" is the bit map of free disk sectors
//----------------------------------------------------------------------

bool
FileHeader::Allocate(BitMap *freeMap, int fileSize)
{ 
    numBytes = fileSize;
    numSectors  = divRoundUp(fileSize, SectorSize);
    if (freeMap->NumClear() < numSectors)
	return FALSE;		// not enough space
    if(numSectors <= NumDirect -1){
        for (int i = 0; i < numSectors; i++){
            dataSectors[i] = freeMap->Find();
        }
    }
    else{
        for(int i = 0; i<NumDirect-1;i++){
            dataSectors[i] = freeMap->Find();
            //printf("Allocate %d\n",dataSectors[i]);
        }
        dataSectors[NumDirect-1] = freeMap->Find();
        if(dataSectors[NumDirect-1] == -1) printf("indirect index in inode error: need more free sector space\n");
        else printf("we Allocate sector %d for indirect index use\n", dataSectors[NumDirect-1]);
        // write it into disk
        int indirectSectorText[SectorSize/4];
        for(int i=0;i<numSectors - NumDirect +1;++i){
            indirectSectorText[i ] = freeMap->Find();
            //printf("Allocate %d\n",indirectSectorText[i]);
        }
        // write it into disk
        synchDisk->WriteSector(dataSectors[NumDirect-1], (char *)indirectSectorText);

        //debug  check for if the writesector done the job
        //printf("check if the writesector done the job:\n");
        //synchDisk->ReadSector(dataSectors[NumDirect-1], (char *)indirectSectorText);
        //for(int i=0;i<32;++i)printf("%d ",indirectSectorText[i]);
        printf("\n");
    }
    lastAccess = lastModify = createTime = stats->totalTicks;
    return TRUE;
}

//----------------------------------------------------------------------
// FileHeader::Deallocate
// 	De-allocate all the space allocated for data blocks for this file.
//
//	"freeMap" is the bit map of free disk sectors
//----------------------------------------------------------------------

void 
FileHeader::Deallocate(BitMap *freeMap)
{
    //printf("contents of bitmap before delete file\n");
    //fflush(stdout);
    //freeMap->Print();
    if(numSectors<NumDirect){
        for (int i = 0; i < numSectors; i++) {
            //printf("want to clear %d\n", dataSectors[i]);
            fflush(stdout);
	ASSERT(freeMap->Test((int) dataSectors[i]));  // ought to be marked!
	freeMap->Clear((int) dataSectors[i]);
        }
    }
    else{
        for(int i=0;i<NumDirect-1;++i){
            ASSERT(freeMap->Test((int)dataSectors[i]));
            freeMap->Clear((int)dataSectors[i]);
        }
        ASSERT(freeMap->Test((int)dataSectors[NumDirect-1]));
        int indirectSectorText[SectorSize/4];
        synchDisk->ReadSector(dataSectors[NumDirect-1], (char*)indirectSectorText);
        for(int i = NumDirect-1;i<numSectors;++i){
            ASSERT(freeMap->Test(indirectSectorText[i- NumDirect+1]));
            freeMap->Clear(indirectSectorText[i - NumDirect+1]);
        }
        freeMap->Clear(dataSectors[NumDirect-1]);
    }
}

//----------------------------------------------------------------------
// FileHeader::FetchFrom
// 	Fetch contents of file header from disk. 
//
//	"sector" is the disk sector containing the file header
//----------------------------------------------------------------------

void
FileHeader::FetchFrom(int sector)
{
    synchDisk->ReadSector(sector, (char *)this);
    lastAccess = stats->totalTicks;
}

//----------------------------------------------------------------------
// FileHeader::WriteBack
// 	Write the modified contents of the file header back to disk. 
//
//	"sector" is the disk sector to contain the file header
//----------------------------------------------------------------------

void
FileHeader::WriteBack(int sector)
{
    synchDisk->WriteSector(sector, (char *)this); 
    lastAccess = lastModify = stats->totalTicks;
}

//----------------------------------------------------------------------
// FileHeader::ByteToSector
// 	Return which disk sector is storing a particular byte within the file.
//      This is essentially a translation from a virtual address (the
//	offset in the file) to a physical address (the sector where the
//	data at the offset is stored).
//
//	"offset" is the location within the file of the byte in question
//----------------------------------------------------------------------

int
FileHeader::ByteToSector(int offset)                                             //maybe some problem
{
    int sectorNum = offset / SectorSize;
    if(sectorNum < NumDirect - 1) return dataSectors[sectorNum];
    else{
        int indirectSectorText[SectorSize/4];
         synchDisk->ReadSector(dataSectors[NumDirect-1], (char*)indirectSectorText);
         return indirectSectorText[sectorNum - NumDirect + 1];
    }
}

//----------------------------------------------------------------------
// FileHeader::FileLength
// 	Return the number of bytes in the file.
//----------------------------------------------------------------------

int
FileHeader::FileLength()
{
    return numBytes;
}

//----------------------------------------------------------------------
// FileHeader::Print
// 	Print the contents of the file header, and the contents of all
//	the data blocks pointed to by the file header.
//----------------------------------------------------------------------

void
FileHeader::Print()
{
    int i, j, k;
    char *data = new char[SectorSize];
    int indirectSectorText[SectorSize/4];
    printf("FileHeader contents.  File size: %d. numSector:%d createTime:%d. lastAccess:%d. lastModify:%d. File blocks:\n", numBytes, numSectors, createTime, lastAccess, lastModify);
    if(numSectors < NumDirect){
        for(i =0;i<numSectors;++i)printf("%d ", dataSectors[i]);
    }
    else{
    for (i = 0; i < NumDirect; i++)
	printf("%d ", dataSectors[i]);
    if(numSectors >= NumDirect){
        printf("we need a 1st indirect index, and print the context:\n");
        synchDisk->ReadSector(dataSectors[NumDirect-1], (char*)indirectSectorText);
        for(int i = 0;i<SectorSize/4;++i)printf("%d ",indirectSectorText[i]);
    }
    }
    printf("\nFile contents:\n");
      // make sure the datasectors[] won't access exceeding
    int size_ = NumDirect-1;
    if(numSectors <= NumDirect-1) size_ = numSectors;
    for (i = k = 0; i < size_; i++) {
	synchDisk->ReadSector(dataSectors[i], data);
        for (j = 0; (j < SectorSize) && (k < numBytes); j++, k++) {
	    if ('\040' <= data[j] && data[j] <= '\176')   // isprint(data[j])
		printf("%c", data[j]);
            else
		printf("\\%x", (unsigned char)data[j]);
	}
        printf("\n"); 
    }
    if(numSectors>NumDirect-1){
        for(i =0; i < numSectors - NumDirect +1;++i){
            synchDisk->ReadSector(indirectSectorText[i], data);
            for(j = 0;(j<SectorSize)&&(k<numBytes); j++,k++){
                if ('\040' <= data[j] && data[j] <= '\176')   // isprint(data[j])
                    printf("%c", data[j]);
                else
                    printf("\\%x", (unsigned char)data[j]);
            }
            printf("\n");
        }
    }
    delete [] data;
}
