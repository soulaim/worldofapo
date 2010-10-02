
#ifndef H_FIXEDPOINT
#define H_FIXEDPOINT

struct FixedPoint
{
	FixedPoint(const FixedPoint& a):number(a.number) {}
	FixedPoint(int a):number(a * 1000) {}
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
	
	int number;
	
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
	
	FixedPoint operator * (const FixedPoint& a) const
	{
		FixedPoint tmp(*this);
		tmp *= a;
		return tmp;
	}
	
	bool operator <= (const FixedPoint& a)
	{
		return number <= a.number;
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
		number *= 1000;
		number /= a.number;
	}
	
	FixedPoint operator / (const FixedPoint& a) const
	{
		FixedPoint tmp(*this);
		tmp /= a;
		return tmp;
	}
};

#endif

