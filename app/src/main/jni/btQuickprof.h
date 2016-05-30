
/***************************************************************************************************
**
** Real-Time Hierarchical Profiling for Game Programming Gems 3
**
** by Greg Hjelstrom & Byon Garrabrant
**
***************************************************************************************************/

// Credits: The Clock class was inspired by the Timer classes in 
// Ogre (www.ogre3d.org).



#ifndef BT_QUICK_PROF_H
#define BT_QUICK_PROF_H

#include <stdio.h>//@todo remove this, backwards compatibility
#include <new>

///The btClock is a portable basic clock that measures accurate time in seconds, use for profiling.
class btClock
{
public:
	btClock();

	btClock(const btClock& other);
	btClock& operator=(const btClock& other);

	~btClock();

	/// Resets the initial reference time.
	void reset();

	/// Returns the time in ms since the last call to reset or since 
	/// the btClock was created.
	unsigned long int getTimeMilliseconds();

	/// Returns the time in us since the last call to reset or since 
	/// the Clock was created.
	unsigned long int getTimeMicroseconds();
private:
	struct btClockData* m_data;
};

#endif //BT_QUICK_PROF_H


