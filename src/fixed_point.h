#ifndef H_FIXEDPOINT
#define H_FIXEDPOINT

#include <ostream>
#include <istream>

// for square root message only
#include <iostream>

// for debugging
#include <cassert>

class FixedPoint
{
	long long number;

public:
	static const FixedPoint ZERO;

	FixedPoint(const FixedPoint& a):
		number(a.number)
	{
	}
	FixedPoint(int a, int b = 1):
		number( (a * 1000) / b )
	{
	}

	FixedPoint():
		number(0)
	{
	}

	float getFloat() const
	{
		return number / 1000.0;
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
	
	FixedPoint& operator += (const FixedPoint& a)
	{
		number += a.number;
		return *this;
	}
	
	FixedPoint operator + (const FixedPoint& a) const
	{
		return FixedPoint(*this) += a;
	}
	
	FixedPoint& operator -= (const FixedPoint& a)
	{
		number -= a.number;
		return *this;
	}
	
	FixedPoint operator - (const FixedPoint& a) const
	{
		return FixedPoint(*this) -= a;
	}

	FixedPoint operator -() const
	{
		FixedPoint tmp(*this);
		tmp.number = -tmp.number;
		return tmp;
	}
	
	FixedPoint operator * (const FixedPoint& a) const
	{
		return FixedPoint(*this) *= a;
	}
	
	bool operator==(const FixedPoint& a) const
	{
		return number == a.number;
	}
	bool operator!=(const FixedPoint& a) const
	{
		return number != a.number;
	}
	bool operator <= (const FixedPoint& a) const
	{
		return number <= a.number;
	}
	bool operator >= (const FixedPoint& a) const
	{
		return number >= a.number;
	}
	bool operator < (const FixedPoint& a) const
	{
		return number < a.number;
	}
	bool operator > (const FixedPoint& a) const
	{
		return number > a.number;
	}
	
	FixedPoint& operator *= (const FixedPoint& a)
	{
		number *= a.number;
		number /= 1000;
		return *this;
	}
	
	FixedPoint& operator/=(const FixedPoint& a)
	{
		assert(a.number != 0);
		
		number *= 1000;
		number /= a.number;
		return *this;
	}
	
	FixedPoint operator / (const FixedPoint& a) const
	{
		return FixedPoint(*this) /= a;
	}
	
	FixedPoint squareRoot() const
	{
		if(number < 2)
		{
			return FixedPoint(0);
		}
		
		// approximates the square root quite nicely
		FixedPoint currentVal(*this);
		currentVal /= FixedPoint(2);
		for(int i = 0; i < 10; ++i)
		{
			currentVal += *this / currentVal;
			currentVal /= FixedPoint(2);
		}
		return currentVal;
	}

	friend inline std::ostream& operator<<(std::ostream& out, const FixedPoint& point)
	{
		return out << point.number;
	}

	friend inline std::istream& operator>>(std::istream& in, FixedPoint& point)
	{
		long long tmp;
		in >> tmp;
		if(in)
		{
			point.number = tmp;
		}
		return in;
	}
	
};


#endif

