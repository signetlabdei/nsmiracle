/*
 * Copyright (c) 2007 Regents of the SIGNET lab, University of Padova.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University of Padova (SIGNET lab) nor the 
 *    names of its contributors may be used to endorse or promote products 
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED 
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <string.h>
#include "utility.h"
#include "stdio.h"
#include <new>
//#define DEBUG
#include "assert.h"
using namespace std;
DinArray::DinArray(int size) : obj_(0), count_(0)
{
	if(size > 0)
	{
		obj_ = new void*[size];
		count_ = size;
	}
}

DinArray::~DinArray()
{
	if(count_ > 0)
		delete [] obj_;
}

int DinArray::count()
{
	return count_;
}

void *DinArray::get(int i)
{
	if(i < 0 || i >= count_)
		return 0;
	return obj_[i];
}

void DinArray::set(int i, void *o)
{
	if(i >= count_)
	{
		void **tmp = new void*[i+1];
		for(int j = 0; j < count_; j++){
			tmp[j] = obj_[j];
		}
		if(count_ > 0){
			delete [] obj_;
			
		}
		obj_ = tmp;
		count_ = i + 1;
		
	}
	obj_[i] = o;
	#ifdef DEBUG
	for(int h = 0; h < count_; h++){
		printf("set obj_[%d] = %p\n", h, obj_[h]);
	}
	#endif
}

void DinArray::clear()
{
	// Mior, 01 2009
	// to prevent memory leak in routing-module.cc
	//printf("Call clear count_ %d\n", count_);
	if(count_ > 0)
		delete [] obj_;
	count_ = 0;

}

void DinArray::sort(Comparator *c)
{
	if(c && count_ > 1)
		mergeSort(0, count_ - 1, c);
}

void DinArray::mergeSort(int i, int f, Comparator *c)
{
	if(f - i > 1)
	{
		int m = (i + f) / 2;
		mergeSort(i, m, c);
		mergeSort(m+1, f, c);
		DinArray tmp(i - f + 1);
		int cur1 = i;
		int cur2 = m + 1;
		int cur = 0;
		while(cur1 <= m || cur2 <= f)
		{
			if(cur1 > m)
			{
				tmp.set(cur, obj_[cur2]);
				cur2++;
			}
			else if(cur2 > m)
			{
				tmp.set(cur, obj_[cur1]);
				cur1++;
			}
			else if(c->isLess(obj_[cur1], obj_[cur2]))
			{
				tmp.set(cur, obj_[cur1]);
				cur1++;
			}
			else
			{
				tmp.set(cur, obj_[cur2]);
				cur2++;
			}
			cur++;
		}
		for(int j = 0; j < tmp.count(); j++)
			obj_[i + j] = tmp.get(j);
	}
	else if(f - i == 1 && c->isLess(obj_[f], obj_[i]))
	{
		void *tmp = obj_[i];
		obj_[i] = obj_[f];
		obj_[f] = tmp;
	}
}

IntegerDinArray::IntegerDinArray(int size) : DinArray(size)
{
}

IntegerDinArray::~IntegerDinArray()
{
}

int IntegerDinArray::get(int i)
{
	return *((int *)(DinArray::get(i)));
}

void IntegerDinArray::set(int i, int value)
{
	int *v = new int(value);
	DinArray::set(i,(void *)v);
}

class IntegerAscComp : public Comparator
{
	virtual int isLess(void *a, void *b)
	{
		return *((int *)a) < *((int *)b);
	}
};

class IntegerDescComp : public Comparator
{
	virtual int isLess(void *a, void *b)
	{
		return *((int *)a) > *((int *)b);
	}
};

void IntegerDinArray::sortAscending()
{
	IntegerAscComp c;
	DinArray::sort(&c);
}

void IntegerDinArray::sortDescending()
{
	IntegerDescComp c;
	DinArray::sort(&c);
}

void *realloc(void *array, int olddim, int newdim)
{printf("old %d, new %d\n", olddim, newdim);
	char *tmp = new char[newdim];
	printf("tmp e' d %d p %p\n", tmp, tmp);
	if (olddim>0)
	{printf("Ma qui entro?\n");
		memcpy(tmp, array, olddim);
		delete [] array;
		assert(false);
	}
	return tmp;
}

