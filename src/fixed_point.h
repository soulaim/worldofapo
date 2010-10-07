
#ifndef H_FIXEDPOINT
#define H_FIXEDPOINT

#include <ostream>

// for square root message only
#include <iostream>

// for debugging
#include <cassert>

struct FixedPoint
{
	long long number;
	
	FixedPoint(const FixedPoint& a):number(a.number) {}
	FixedPoint(const int a, bool isFloat = false):number(isFloat?a:(a * 1000)) {}
	FixedPoint():number(0) {}
	
	float getFloat() const
	{
		return number / 1000 + (number % 1000) / 1000.0;
	}
	
	int getInteger() const
	{
		return number / 1000;
	}
	
	int getDesimal() const
	{
		return number % 1000;
	}
	
	FixedPoint abs()
	{
		FixedPoint tmp(*this);
		tmp.number *= (tmp.number < 0)?-1:1;
		return tmp;
	}
	
	void operator += (const FixedPoint& a)
	{
		number += a.number;
	}
	
	FixedPoint operator + (const FixedPoint& a) const
	{
		FixedPoint tmp(a);
		tmp += *this;
		return tmp;
	}
	
	void operator -= (const FixedPoint& a)
	{
		number -= a.number;
	}
	
	FixedPoint operator - (const FixedPoint& a) const
	{
		FixedPoint tmp(a);
		tmp -= *this;
		return tmp;
	}

	FixedPoint operator -() const
	{
		FixedPoint tmp(*this);
		tmp.number = -tmp.number;
		return tmp;
	}
	
	FixedPoint operator * (const FixedPoint& a) const
	{
		FixedPoint tmp(*this);
		tmp *= a;
		return tmp;
	}
	
	bool operator <= (const FixedPoint& a) const
	{
		return number <= a.number;
	}
	bool operator < (const FixedPoint& a) const
	{
		return number < a.number;
	}
	bool operator > (const FixedPoint& a) const
	{
		return number > a.number;
	}
	
	void operator *= (const FixedPoint& a)
	{
		long long tmp = a.number;
		tmp *= number;
		tmp /= 1000;
		number = tmp;
	}
	
	void operator /= (const FixedPoint& a)
	{
		assert(a.number != 0);
		
		number *= 1000;
		number /= a.number;
	}
	
	FixedPoint operator / (const FixedPoint& a) const
	{
		FixedPoint tmp(*this);
		tmp /= a;
		return tmp;
	}
	
	FixedPoint squareRoot() const
	{
		if(number < 2)
			return FixedPoint(0);
		
		// approximates the square root quite nicely
		FixedPoint currentVal = *this / FixedPoint(2);
		for(int i=0; i<30; i++)
			currentVal = (currentVal + *this / currentVal) / FixedPoint(2);
		return currentVal;
	}
	
};

inline std::ostream& operator<<(std::ostream& out, const FixedPoint& point)
{
	return out << point.getFloat();
}

#endif

