/* Copyright (C) 2008, 2009 TitanFiesta Dev Team
 * Licensed under GNU GPL v3
 * For license details, see LICENCE in the root folder. */

//Made by Caali

#define __CARRAY_HPP__

/* Includes */
#include <memory.h>

/* Variable type definitions */
typedef size_t					CARRAY_SIZE;

/* Defines */
#ifndef NULL
#	ifdef __cplusplus
#		define NULL 0
#	else
#		define NULL (void*)0
#	endif
#endif

/* A basic CArray class. It is being used because C++ has no function to get the actual size in memory of a pointer. */
template <typename T>
class CTitanArray
{
	private:
		CARRAY_SIZE arraysize;
	public:
		T* arr;
		CTitanArray( CARRAY_SIZE arrsize )
		{
			this->arr = NULL;
			this->arraysize = 0;
			this->arr = new T[arrsize];
			if( this->arr != NULL )
			{
				this->arraysize = arrsize;
				Clean( );
			} 
		}

		~CTitanArray( )
		{
			if( this->arr != NULL )
			{
				delete[] this->arr;
				this->arraysize = 0;
			}
		}

		void resize( CARRAY_SIZE newsize )
		{
			if( this->arr == NULL ) return;
			T* tmp = new T[(size_t)(this->arraysize)];
			CARRAY_SIZE tmpsize = this->arraysize;
			memcpy( tmp, this->arr, (size_t)(this->arraysize*sizeof(T)) ); 
			delete[] this->arr;
			this->arr = new T[(size_t)(newsize)];
			if( this->arr != NULL )
			{
				memcpy( this->arr, tmp, (size_t)(tmpsize * sizeof(T)) );
				delete[] tmp;
				this->arraysize = newsize;
			}
			else
			{
				delete[] tmp;
				this->arraysize = 0;
			}
		}

		void Clean( )
		{
			if( this->arr != NULL )
			{
				for( CARRAY_SIZE i = 0; i < this->arraysize; i++ )
				{
					this->arr[i] = 0;
				}
			}
		}
		CARRAY_SIZE size()
		{
			return this->arraysize;
		}
		CARRAY_SIZE absSize()
		{
			return (this->arraysize*sizeof(T));
		}
		inline CTitanArray& operator = ( const CTitanArray& otherarr )
		{
			if( arr != NULL ) delete[] arr;
			arraysize = otherarr.arraysize;
			for( CARRAY_SIZE i = 0; i<size(); i++ ){
				arr[i] = otherarr.arr[i];
			}
		}
		inline bool operator == ( const CTitanArray& otherarr )
		{
			if( size() == otherarr.size() )
			{
				if( arr == NULL && otherarr.arr == NULL )
				{
					return true;
				}
				else
				{
					for( CARRAY_SIZE i = 0; i<size(); i++ )
					{
						if( arr[i] != otherarr.arr[i] ) return false;
					}
					return true;
				}
			}
			return false;
		}

		inline bool operator != ( const CTitanArray& otherarr )
		{
			return !(this == otherarr);
		}
};
