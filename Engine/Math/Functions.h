/*
    This file contains math-related functions
*/

#ifndef EAE6320_MATH_FUNCTIONS_H
#define EAE6320_MATH_FUNCTIONS_H

#include <type_traits>

namespace eae6320
{
    namespace Math
    {
        // Interface
        //==========

        float ConvertDegreesToRadians( const float i_degrees );
        float ConvertRadiansToDegrees( const float i_radians );
        float ConvertHorizontalFieldOfViewToVerticalFieldOfView( const float i_horizontalFieldOfView_inRadians,
            // aspectRatio = width / height
            const float i_aspectRatio );

        float Rand();
        float RandRange( const float i_min, const float i_max );

        // Rounds up an unsigned integer to the next highest multiple
        template<typename tUnsignedInteger, class EnforceUnsigned = typename std::enable_if<std::is_unsigned<tUnsignedInteger>::value>::type>
            tUnsignedInteger RoundUpToMultiple( const tUnsignedInteger i_value, const tUnsignedInteger i_multiple );
        // If the multiple is known to be a power-of-2 this is cheaper than the previous function
        template<typename tUnsignedInteger, class EnforceUnsigned = typename std::enable_if<std::is_unsigned<tUnsignedInteger>::value>::type>
            tUnsignedInteger RoundUpToMultiple_powerOf2( const tUnsignedInteger i_value, const tUnsignedInteger i_multipleWhichIsAPowerOf2 );

        // Clamp a value between a minimum and maximum; inclusive
        template<typename T>
        T Clamp(const T i_value, const T i_min, const T i_max);
    }
}

#include "Functions.inl"

#endif  // EAE6320_MATH_FUNCTIONS_H
