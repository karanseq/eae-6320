// Include Files
//==============

#include "sVector2d.h"

#include <cmath>
#include <Engine/Asserts/Asserts.h>

// Static Data Initialization
//===========================

namespace
{
	constexpr auto s_epsilon = 1.0e-9f;
}

// Interface
//==========

// Addition
//---------

eae6320::Math::sVector2d eae6320::Math::sVector2d::operator +(const sVector2d& i_rhs) const
{
	return sVector2d(x + i_rhs.x, y + i_rhs.y);
}

eae6320::Math::sVector2d& eae6320::Math::sVector2d::operator +=(const sVector2d& i_rhs)
{
	x += i_rhs.x;
	y += i_rhs.y;
	return *this;
}

// Subtraction / Negation
//-----------------------

eae6320::Math::sVector2d eae6320::Math::sVector2d::operator -(const sVector2d& i_rhs) const
{
	return sVector2d(x - i_rhs.x, y - i_rhs.y);
}

eae6320::Math::sVector2d& eae6320::Math::sVector2d::operator -=(const sVector2d& i_rhs)
{
	x -= i_rhs.x;
	y -= i_rhs.y;
	return *this;
}

eae6320::Math::sVector2d eae6320::Math::sVector2d::operator -() const
{
	return sVector2d(-x, -y);
}

// Multiplication
//---------------

eae6320::Math::sVector2d eae6320::Math::sVector2d::operator *(const float i_rhs) const
{
	return sVector2d(x * i_rhs, y * i_rhs);
}

eae6320::Math::sVector2d& eae6320::Math::sVector2d::operator *=(const float i_rhs)
{
	x *= i_rhs;
	y *= i_rhs;
	return *this;
}

// Division
//---------

eae6320::Math::sVector2d eae6320::Math::sVector2d::operator /(const float i_rhs) const
{
	EAE6320_ASSERTF(std::abs(i_rhs) > s_epsilon, "Can't divide by zero");
	const auto rhs_reciprocal = 1.0f / i_rhs;
	return sVector2d(x * rhs_reciprocal, y * rhs_reciprocal);
}

eae6320::Math::sVector2d& eae6320::Math::sVector2d::operator /=(const float i_rhs)
{
	EAE6320_ASSERTF(std::abs(i_rhs) > s_epsilon, "Can't divide by zero");
	const auto rhs_reciprocal = 1.0f / i_rhs;
	x *= rhs_reciprocal;
	y *= rhs_reciprocal;
	return *this;
}

// Length / Normalization
//-----------------------

float eae6320::Math::sVector2d::GetLength() const
{
	return std::sqrt((x * x) + (y * y));
}

float eae6320::Math::sVector2d::Normalize()
{
	const auto length = GetLength();
	EAE6320_ASSERTF(length > s_epsilon, "Can't divide by zero");
	operator /=(length);
	return length;
}

eae6320::Math::sVector2d eae6320::Math::sVector2d::GetNormalized() const
{
	const auto length = GetLength();
	EAE6320_ASSERTF(length > s_epsilon, "Can't divide by zero");
	const auto length_reciprocal = 1.0f / length;
	return sVector2d(x * length_reciprocal, y * length_reciprocal);
}

// Comparison
//-----------

bool eae6320::Math::sVector2d::operator ==(const sVector2d& i_rhs) const
{
	// Use & rather than && to prevent branches (all three comparisons will be evaluated)
	return (x == i_rhs.x) & (y == i_rhs.y);
}

bool eae6320::Math::sVector2d::operator !=(const sVector2d& i_rhs) const
{
	// Use | rather than || to prevent branches (all three comparisons will be evaluated)
	return (x != i_rhs.x) | (y != i_rhs.y);
}

// Products
//---------

float eae6320::Math::Dot(const sVector2d& i_lhs, const sVector2d& i_rhs)
{
	return (i_lhs.x * i_rhs.x) + (i_lhs.y * i_rhs.y);
}
