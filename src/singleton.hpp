#ifndef _SINGLETON_HPP
#define _SINGLETON_HPP

#include <wx/debug.h>

template<class T>
class Singleton
{
protected:
	Singleton()
	{
		wxASSERT_MSG(s_instance == nullptr,
			"This class should only have one instance");

		s_instance = static_cast<T *>(this);
	}
public:
	inline static T *GetInstance()
	{
		wxASSERT(s_instance);
		return s_instance;
	}
	inline static bool IsNull()
	{
		return s_instance == nullptr;
	}
private:
	inline static T *s_instance = nullptr;
};

#endif