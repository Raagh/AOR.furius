#ifndef KEYTOGGLE_H
#define KEYTOGGLE_H
#include <windows.h>

class KeyToggle
{
public:
	int mKey;

	//KeyToggle(int key) :mKey(key), mActive(false){}
	KeyToggle(){};

	operator bool()
	{
		if (GetAsyncKeyState(mKey))
			//if (GetKeyState(mKey))
		{
			if (!mActive)
			{
				mActive = true;
				return true;
			}
		}
		else
		{
			mActive = false;
		}

		return false;
	}

private:
	bool mActive;
};

#endif

