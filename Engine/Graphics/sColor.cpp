// Include Files
//==============

#include "sColor.h"

#include <Engine/Math/Functions.h>
#include <Engine/Asserts/Asserts.h>

// Static Data Initialization
//===========================

const eae6320::Graphics::sColor eae6320::Graphics::sColor::BLACK(0.0f, 0.0f, 0.0f, 1.0f);
const eae6320::Graphics::sColor eae6320::Graphics::sColor::BLUE(0.0f, 0.0f, 1.0f, 1.0f);
const eae6320::Graphics::sColor eae6320::Graphics::sColor::CYAN(0.0f, 1.0f, 1.0f, 1.0f);
const eae6320::Graphics::sColor eae6320::Graphics::sColor::EMERALD(0.18039f, 0.8f, 0.44314f, 1.0f);
const eae6320::Graphics::sColor eae6320::Graphics::sColor::GREEN(0.0f, 1.0f, 0.0f, 1.0f);
const eae6320::Graphics::sColor eae6320::Graphics::sColor::MAGENTA(1.0f, 0.0f, 1.0f, 1.0f);
const eae6320::Graphics::sColor eae6320::Graphics::sColor::ORANGE(0.95294f, 0.61176f, 0.07059f, 1.0f);
const eae6320::Graphics::sColor eae6320::Graphics::sColor::PURPLE(0.66275f, 0.02745f, 0.89412f, 1.0f);
const eae6320::Graphics::sColor eae6320::Graphics::sColor::RED(1.0f, 0.0f, 0.0f, 1.0f);
const eae6320::Graphics::sColor eae6320::Graphics::sColor::SILVER(0.74118f, 0.76471f, 0.78039f, 1.0f);
const eae6320::Graphics::sColor eae6320::Graphics::sColor::TURQUOISE(0.10196f, 0.73725f, 0.61176f, 1.0f);
const eae6320::Graphics::sColor eae6320::Graphics::sColor::WHITE(1.0f, 1.0f, 1.0f, 1.0f);
const eae6320::Graphics::sColor eae6320::Graphics::sColor::YELLOW(1.0f, 1.0f, 0.0f, 1.0f);

namespace
{
	constexpr auto s_epsilon = 1.0e-9f;
}

// Initialization / Shut Down
//---------------------------

eae6320::Graphics::sColor::sColor(const float i_r, const float i_g, const float i_b, const float i_a)
	: 
	r(eae6320::Math::Clamp<float>(i_r, 0.0f, 1.0f)), 
	g(eae6320::Math::Clamp<float>(i_g, 0.0f, 1.0f)),
	b(eae6320::Math::Clamp<float>(i_b, 0.0f, 1.0f)),
	a(eae6320::Math::Clamp<float>(i_a, 0.0f, 1.0f))
{}

// Interface
//==========

// Addition
//---------

eae6320::Graphics::sColor eae6320::Graphics::sColor::operator+(const eae6320::Graphics::sColor& i_rhs) const
{
	return sColor(r + i_rhs.r, g + i_rhs.g, b + i_rhs.b, a + i_rhs.a);
}

eae6320::Graphics::sColor& eae6320::Graphics::sColor::operator+=(const eae6320::Graphics::sColor& i_rhs)
{
	r = eae6320::Math::Clamp<float>(r + i_rhs.r, 0.0f, 1.0f);
	g = eae6320::Math::Clamp<float>(g + i_rhs.g, 0.0f, 1.0f);
	b = eae6320::Math::Clamp<float>(b + i_rhs.b, 0.0f, 1.0f);
	a = eae6320::Math::Clamp<float>(a + i_rhs.a, 0.0f, 1.0f);
	return *this;
}

// Subtraction
//------------

eae6320::Graphics::sColor eae6320::Graphics::sColor::operator-(const eae6320::Graphics::sColor& i_rhs) const
{
	return sColor(r - i_rhs.r, g - i_rhs.g, b - i_rhs.b, a - i_rhs.a);
}

eae6320::Graphics::sColor& eae6320::Graphics::sColor::operator-=(const eae6320::Graphics::sColor& i_rhs)
{
	r = eae6320::Math::Clamp<float>(r - i_rhs.r, 0.0f, 1.0f);
	g = eae6320::Math::Clamp<float>(g - i_rhs.g, 0.0f, 1.0f);
	b = eae6320::Math::Clamp<float>(b - i_rhs.b, 0.0f, 1.0f);
	a = eae6320::Math::Clamp<float>(a - i_rhs.a, 0.0f, 1.0f);
	return *this;
}

// Multiplication
//---------------

eae6320::Graphics::sColor eae6320::Graphics::sColor::operator*(const eae6320::Graphics::sColor& i_rhs) const
{
	return sColor(r * i_rhs.r, g * i_rhs.g, b * i_rhs.b, a * i_rhs.a);
}

eae6320::Graphics::sColor& eae6320::Graphics::sColor::operator*=(const eae6320::Graphics::sColor& i_rhs)
{
	r = eae6320::Math::Clamp<float>(r * i_rhs.r, 0.0f, 1.0f);
	g = eae6320::Math::Clamp<float>(g * i_rhs.g, 0.0f, 1.0f);
	b = eae6320::Math::Clamp<float>(b * i_rhs.b, 0.0f, 1.0f);
	a = eae6320::Math::Clamp<float>(a * i_rhs.a, 0.0f, 1.0f);
	return *this;
}

eae6320::Graphics::sColor eae6320::Graphics::sColor::operator*(const float i_scalar) const
{
	return sColor(r * i_scalar, g * i_scalar, b * i_scalar, a * i_scalar);
}

eae6320::Graphics::sColor& eae6320::Graphics::sColor::operator*=(const float i_scalar)
{
	r = eae6320::Math::Clamp<float>(r * i_scalar, 0.0f, 1.0f);
	g = eae6320::Math::Clamp<float>(g * i_scalar, 0.0f, 1.0f);
	b = eae6320::Math::Clamp<float>(b * i_scalar, 0.0f, 1.0f);
	a = eae6320::Math::Clamp<float>(a * i_scalar, 0.0f, 1.0f);
	return *this;
}

// Division
//---------

eae6320::Graphics::sColor eae6320::Graphics::sColor::operator/(const eae6320::Graphics::sColor& i_rhs) const
{
	EAE6320_ASSERTF(std::abs(i_rhs.r) > s_epsilon, "Can't divide by zero");
	EAE6320_ASSERTF(std::abs(i_rhs.g) > s_epsilon, "Can't divide by zero");
	EAE6320_ASSERTF(std::abs(i_rhs.b) > s_epsilon, "Can't divide by zero");
	EAE6320_ASSERTF(std::abs(i_rhs.a) > s_epsilon, "Can't divide by zero");
	return sColor(r / i_rhs.r, g / i_rhs.g, b / i_rhs.b, a / i_rhs.a);
}

eae6320::Graphics::sColor& eae6320::Graphics::sColor::operator/=(const eae6320::Graphics::sColor& i_rhs)
{
	EAE6320_ASSERTF(std::abs(i_rhs.r) > s_epsilon, "Can't divide by zero");
	EAE6320_ASSERTF(std::abs(i_rhs.g) > s_epsilon, "Can't divide by zero");
	EAE6320_ASSERTF(std::abs(i_rhs.b) > s_epsilon, "Can't divide by zero");
	EAE6320_ASSERTF(std::abs(i_rhs.a) > s_epsilon, "Can't divide by zero");
	r = eae6320::Math::Clamp<float>(r / i_rhs.r, 0.0f, 1.0f);
	g = eae6320::Math::Clamp<float>(g / i_rhs.g, 0.0f, 1.0f);
	b = eae6320::Math::Clamp<float>(b / i_rhs.b, 0.0f, 1.0f);
	a = eae6320::Math::Clamp<float>(a / i_rhs.a, 0.0f, 1.0f);
	return *this;
}

eae6320::Graphics::sColor eae6320::Graphics::sColor::operator/(const float i_scalar) const
{
	EAE6320_ASSERTF(std::abs(i_scalar) > s_epsilon, "Can't divide by zero");
	float inverseScalar = 1.0f / i_scalar;
	return sColor(r * inverseScalar, g * inverseScalar, b * inverseScalar, a * inverseScalar);
}

eae6320::Graphics::sColor& eae6320::Graphics::sColor::operator/=(const float i_scalar)
{
	EAE6320_ASSERTF(std::abs(i_scalar) > s_epsilon, "Can't divide by zero");
	float inverseScalar = 1.0f / i_scalar;
	r = eae6320::Math::Clamp<float>(r * inverseScalar, 0.0f, 1.0f);
	g = eae6320::Math::Clamp<float>(g * inverseScalar, 0.0f, 1.0f);
	b = eae6320::Math::Clamp<float>(b * inverseScalar, 0.0f, 1.0f);
	a = eae6320::Math::Clamp<float>(a * inverseScalar, 0.0f, 1.0f);
	return *this;
}

// Comparison
//-----------

bool eae6320::Graphics::sColor::operator==(const eae6320::Graphics::sColor& i_rhs) const
{
	return (r == i_rhs.r) & (g == i_rhs.g) & (b == i_rhs.b) & (a == i_rhs.a);
}

bool eae6320::Graphics::sColor::operator!=(const eae6320::Graphics::sColor& i_rhs) const
{
	return (r != i_rhs.r) | (g != i_rhs.g) | (b != i_rhs.b) | (a != i_rhs.a);
}
