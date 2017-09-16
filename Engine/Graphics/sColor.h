/*
	This struct represents color in the RGBA format.
*/

#ifndef EAE6320_GRAPHICS_SCOLOR_H
#define EAE6320_GRAPHICS_SCOLOR_H

// Struct Declaration
//===================

namespace eae6320
{
	namespace Graphics
	{
		struct sColor
		{
			// Data
			//=====

			float r = 0.0f, g = 0.0f, b = 0.0f, a = 0.0f;

			// Interface
			//==========

			// Addition
			//---------

			sColor operator +(const sColor& i_rhs) const;
			sColor& operator +=(const sColor& i_rhs);

			// Subtraction
			//------------

			sColor operator -(const sColor& i_rhs) const;
			sColor& operator -=(const sColor& i_rhs);

			// Multiplication
			//---------------

			sColor operator *(const sColor& i_rhs) const;
			sColor& operator *=(const sColor& i_rhs);

			sColor operator *(const float i_scalar) const;
			sColor& operator *=(const float i_scalar);

			// Division
			//---------

			sColor operator /(const sColor& i_rhs) const;
			sColor& operator /=(const sColor& i_rhs);

			sColor operator /(const float i_scalar) const;
			sColor& operator /=(const float i_scalar);

			// Comparison
			//-----------

			bool operator ==(const sColor& i_rhs) const;
			bool operator !=(const sColor& i_rhs) const;

			// Static Data
			//============

			static const sColor BLACK;
			static const sColor BLUE;
			static const sColor CYAN;
			static const sColor EMERALD;
			static const sColor GREEN;
			static const sColor MAGENTA;
			static const sColor ORANGE;
			static const sColor PURPLE;
			static const sColor RED;
			static const sColor SILVER;
			static const sColor TURQUOISE;
			static const sColor WHITE;
			static const sColor YELLOW;

			// Initialization / Shut Down
			//---------------------------

			sColor() = default;
			sColor(const float i_r, const float i_g, const float i_b, const float i_a);

		};
	}
}

#endif // EAE6320_GRAPHICS_SCOLOR_H