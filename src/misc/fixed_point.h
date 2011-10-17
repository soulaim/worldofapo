#ifndef H_FIXEDPOINT
#define H_FIXEDPOINT

#include <ostream>
#include <istream>
#include <iostream>
#include <cassert>

class FixedPoint
{
	long long number;

public:
	enum { FIXED_POINT_ONE = 1024, FP_BITS_DESIMALS = 9 };
	static const FixedPoint ZERO;


	FixedPoint(const FixedPoint& a):
		number(a.number)
	{
	}
	FixedPoint(int a, int b = 1):
		number( a * FIXED_POINT_ONE / b )
	{
	}

	FixedPoint():
		number(0)
	{
	}

    void setNumber(int a) {
        number = a;
    }

	float getFloat() const
	{
		return number / float(FIXED_POINT_ONE);
	}

    // note, this is not exactly safe. larger fixed point
    // values will not fit in an integer.
    int getAsInteger() const
    {
        return number;
    }

	int getInteger() const
	{
		return number / FIXED_POINT_ONE;
		// return number >> FP_BITS_DESIMALS;
	}

	int getDesimal() const
	{
		return number % FIXED_POINT_ONE;
		// return number & ((1 << (FP_BITS_DESIMALS)) - 1);
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
		number /= FIXED_POINT_ONE;
		// number >>= FP_BITS_DESIMALS;
		return *this;
	}

	FixedPoint& operator/=(const FixedPoint& a)
	{
		assert(a.number != 0);
		number *= FIXED_POINT_ONE;
		// number <<= FP_BITS_DESIMALS;
		number /= a.number;
		return *this;
	}

	FixedPoint operator / (const FixedPoint& a) const
	{
		return FixedPoint(*this) /= a;
	}

	FixedPoint squared() const
	{
		return (*this) * (*this);
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

template <class T>
T squareRoot(const T& t)
{
	if(t * 1000 < 1)
	{
		return T(0);
	}

	// approximates the square root quite nicely
	T currentVal(t);
	currentVal /= T(2);
	for(int i = 0; i < 10; ++i)
	{
		currentVal += t / currentVal;
		currentVal /= T(2);
	}

	return currentVal;
}

#endif

