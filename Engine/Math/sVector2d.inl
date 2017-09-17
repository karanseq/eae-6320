#ifndef EAE6320_MATH_SVECTOR2D_INL
#define EAE6320_MATH_SVECTOR2D_INL

// Include Files
//==============

#include "sVector2d.h"

// Interface
//==========

// Multiplication
//---------------

inline eae6320::Math::sVector2d operator *(const float i_lhs, const eae6320::Math::sVector2d& i_rhs)
{
	return i_rhs * i_lhs;
}

// Initialization / Shut Down
//---------------------------

inline eae6320::Math::sVector2d::sVector2d(const float i_x, const float i_y)
	:
	x(i_x), y(i_y)
{}

#endif	// EAE6320_MATH_SVECTOR2D_INL
