#ifndef __POOLPTR
#define __POOLPTR

#include "PoolPtrBase.h"
#ifdef _DEBUG
	#include <assert.h>
#endif

template<typename T>
class PoolPtr : public PoolPtrBase
{
public:
	PoolPtr(MemoryChunk* referencedChunk = nullptr)
		: PoolPtrBase(referencedChunk)
#ifdef _DEBUG
		, m_allocatedInstances(0u)
#endif
	{}

#ifdef _DEBUG
	PoolPtr(MemoryChunk* referencedChunk, unsigned int allocatedInstances)
		: PoolPtrBase(referencedChunk)
		, m_allocatedInstances(allocatedInstances)
	{}

	unsigned int m_allocatedInstances;
#endif


	T& operator*();
	const T& operator*() const;
	T* operator->();
	const T* operator->() const;
	//Make sure you don't go over the allocated instances when initializing the Ptr
	//No exception will be thrown, but you may be messing some other data
	T& operator[](unsigned int index);
	//Make sure you don't go over the allocated instances when initializing the Ptr
	//No exception will be thrown, but you may be messing some other data
	const T& operator[](unsigned int index) const;

	T* GetData();
	const T* GetData() const;
};

#endif // !__POOLPTR

template<typename T>
inline T& PoolPtr<T>::operator*()
{
	return *GetData();
}

template<typename T>
inline const T& PoolPtr<T>::operator*() const
{
	return *GetData();
}

template<typename T>
inline T* PoolPtr<T>::operator->()
{
	return GetData();
}

template<typename T>
inline const T* PoolPtr<T>::operator->() const
{
	return *GetData();
}

template<typename T>
inline T& PoolPtr<T>::operator[](unsigned int index)
{
#ifdef _DEBUG
	assert(index >= 0 && index < m_allocatedInstances
		&& "Attempted to access memory not belonging to this ptr");
#endif
	return *(GetData() + index);
}

template<typename T>
inline const T& PoolPtr<T>::operator[](unsigned int index) const
{
#ifdef _DEBUG
	assert(index >= 0 && index < m_allocatedInstances);
#endif
	return *(GetData() + index);
}

template<typename T>
inline T* PoolPtr<T>::GetData()
{
	return (T*)GetRawData();
}

template<typename T>
inline const T* PoolPtr<T>::GetData() const
{
	return (T*)GetRawData();
}