//-------------------------------------------------------------------
#ifndef __Persistence_H__
#define __Persistence_H__
//-------------------------------------------------------------------

#include "Arduino.h"

class PersistenceObject;

class PersistenceFile
{
private:
	PersistenceObject* pFirstObject;

public:
	PersistenceFile();

	/** Get the first object.
	@return The first object of the linked list of objects or NULL if the list is empty.
	*/
	PersistenceObject* getFirst();

	void add(PersistenceObject* inpObject);
	bool remove(PersistenceObject* inpObject);
	int count();

	virtual bool write();
	virtual bool read();

#ifdef DCCPP_DEBUG_MODE
	/** Print the list of persistence objects.
	@remark Only available if DCCPP_DEBUG_MODE is defined.
	*/
	void printObjects();
#endif
};

/** Class used to be able to save/load data into a PersistenceFile.
*/
class PersistenceObject
{
public:
	PersistenceObject* pNextObject;

	virtual bool write(PersistenceFile *apFile) { return false; }
	virtual bool read(PersistenceFile *apFile) { return false; }
};

#endif

