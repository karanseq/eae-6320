/*
	A handle is an opaque identifier associated with an object

	It is used similar to a pointer,
	in that it is a small value that can be used indirectly to get to a bigger value.
	An asset manager will give out handles,
	and an asset manager can be given a handle to access the actual object pointer.
*/

#ifndef EAE6320_ASSETS_CHANDLE_H
#define EAE6320_ASSETS_CHANDLE_H

// Include Files
//==============

#include <cstdint>
#include <Engine/Results/Results.h>

// Forward Declarations
//=====================

namespace eae6320
{
	namespace Assets
	{
		template <class tAsset> class cManager;
	}
}

// Class Declaration
//==================

namespace eae6320
{
	namespace Assets
	{
		// Handles are templated for type safety:
		// A handle for one kind of asset can't be used for a different kind of asset
		template <class tAsset>
			class cHandle
		{
			// Interface
			//==========

		public:

			// This class is just a wrapper for a 32 bit value,
			// where different bits have different meanings

			// The handle's index and error checking ID together use 32 bits,
			// but there are tradeoffs in how those bits are divided between the two:
			//	* The more bits that the index has increases the total number of assets that can exist at any one time,
			//		but the fewer bits that the error checking ID has increases the potential of collisions due to overflow
			//	* A smaller number of index bits can also allow optimizations when an asset's index is used as a unique identifier

			// Access
			//-------

			// Access is provided to a handle's index and bit count for low-level optimization (fitting a unique index/ID into a small number of bits),
			// but this is for exceptional cases only:
			// In general a handle should be treated as a magic thing, like a pointer, with no knowledge of how it is implemented

#define EAE6320_ASSETS_INDEX_MASK 0Xfffff	// The lowest 20 bits

			uint_fast32_t GetIndex() const { return static_cast<uint_fast32_t>( value & EAE6320_ASSETS_INDEX_MASK ); }

			bool IsValid() const { return GetIndex () != InvalidIndex; }
			operator bool() const { return IsValid(); }

			// Initialization / Clean Up
			//--------------------------

			cHandle() = default;

			// Data
			//=====

		private:

			// The highest possible value will be used as an invalid index (akin to -1)
			static constexpr uint_fast32_t InvalidIndex = EAE6320_ASSETS_INDEX_MASK;
			uint32_t value = InvalidIndex;

			// Implementation
			//===============

		private:

#define EAE6320_ASSETS_ID_SHIFT 20	// The remaining high bits

			uint_fast32_t GetId() const { return static_cast<uint_fast32_t>( value >> EAE6320_ASSETS_ID_SHIFT ); }

			void MakeInvalid() { value = InvalidIndex; }
			static uint_fast32_t IncrementId( const uint_fast32_t i_id )
			{
				return ( ( i_id + 1 ) << EAE6320_ASSETS_ID_SHIFT ) >> EAE6320_ASSETS_ID_SHIFT;
			}

			// Initialization / Clean Up
			//--------------------------

			cHandle( const uint_fast32_t i_index, const uint_fast32_t i_id )
				:
				value( i_index | ( i_id << EAE6320_ASSETS_ID_SHIFT ) )
			{

			}

#undef EAE6320_ASSETS_INDEX_MASK
#undef EAE6320_ASSETS_ID_SHIFT

			// Friends
			//========

			// Nothing should ever worry about the IDs except asset managers
			template <class tAsset> friend class cManager;
		};
	}
};

#endif	// EAE6320_ASSETS_CHANDLE_H
