/*
This struct represents a position or direction
*/

#ifndef EAE6320_MATH_SVECTOR2D_H
#define EAE6320_MATH_SVECTOR2D_H

// Struct Declaration
//===================

namespace eae6320
{
	namespace Math
	{
		struct sVector2d
		{
			// Data
			//=====

			float x = 0.0f, y = 0.0f;

			// Interface
			//==========

			// Addition
			//---------

			sVector2d operator +(const sVector2d& i_rhs) const;
			sVector2d& operator +=(const sVector2d& i_rhs);

			// Subtraction / Negation
			//-----------------------

			sVector2d operator -(const sVector2d& i_rhs) const;
			sVector2d& operator -=(const sVector2d& i_rhs);
			sVector2d operator -() const;

			// Multiplication
			//---------------

			sVector2d operator *(const float i_rhs) const;
			sVector2d& operator *=(const float i_rhs);
			friend sVector2d operator *(const float i_lhs, const sVector2d& i_rhs);

			// Division
			//---------

			sVector2d operator /(const float i_rhs) const;
			sVector2d& operator /=(const float i_rhs);

			// Length / Normalization
			//-----------------------

			float GetLength() const;
			float Normalize();
			sVector2d GetNormalized() const;

			// Products
			//---------

			friend float Dot(const sVector2d& i_lhs, const sVector2d& i_rhs);

			// Comparison
			//-----------

			bool operator ==(const sVector2d& i_rhs) const;
			bool operator !=(const sVector2d& i_rhs) const;

			// Initialization / Shut Down
			//---------------------------

			sVector2d() = default;
			sVector2d(const float i_x, const float i_y);
		};
	}
}

#include "sVector2d.inl"

#endif	// EAE6320_MATH_SVECTOR2D_H
