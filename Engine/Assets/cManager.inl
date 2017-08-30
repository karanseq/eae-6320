#ifndef EAE6320_ASSETS_CMANAGER_INL
#define EAE6320_ASSETS_CMANAGER_INL

// Include Files
//==============

#include "cManager.h"

#include <Engine/Asserts/Asserts.h>
#include <Engine/Logging/Logging.h>

// Interface
//==========

template <class tAsset>
	tAsset* eae6320::Assets::cManager<tAsset>::Get( const cHandle<tAsset> i_handle )
{
	EAE6320_ASSERTF( i_handle, "This handle is invalid (it has never been associated with a valid asset)" );
	// Lock the collections
	Concurrency::cMutex::cScopeLock autoLock( m_mutex );
	{
		const auto index = i_handle.GetIndex();
		const auto assetCount = m_assetRecords.size();
		if ( index < assetCount )
		{
			auto &assetRecord = m_assetRecords[index];
			const auto id_assetRecord = assetRecord.id;
			const auto id_handle = i_handle.GetId();
			if ( id_handle == id_assetRecord )
			{
				return assetRecord.asset;
			}
			else
			{
				EAE6320_ASSERTF( false, "A handle (at index %u) has an ID (%u) that doesn't match the asset record (%u)",
					index, id_handle, id_assetRecord );
			}
		}
		else
		{
			EAE6320_ASSERTF( false, "A handle has an index (%u) that's too big for the number of assets (%u)",
				index, assetCount );
		}
	}
	// If this code is reached the handle doesn't point to a valid asset
	return nullptr;
}

template <class tAsset> template <typename... tConstructorArguments>
	eae6320::cResult eae6320::Assets::cManager<tAsset>::Load( const char* const i_path, cHandle<tAsset>& o_handle, tConstructorArguments&&... i_constructorArguments )
{
	// Get the existing asset if the path has already been loaded
	{
		// Lock the collections
		Concurrency::cMutex::cScopeLock autoLock( m_mutex );
		{
			auto iterator = m_map_pathsToHandles.find( i_path );
			if ( iterator != m_map_pathsToHandles.end() )
			{
				// Even if an entry exists it may no longer be valid
				// (the map doesn't get cleared when an asset is deleted)
				const auto existingHandle = iterator->second;
				const auto index = existingHandle.GetIndex();
				const auto assetCount = m_assetRecords.size();
				if ( index < assetCount )
				{
					auto &assetRecord = m_assetRecords[index];
					const auto id_assetRecord = assetRecord.id;
					const auto id_handle = existingHandle.GetId();
					if ( id_handle == id_assetRecord )
					{
						auto *const existingAsset = assetRecord.asset;
						existingAsset->IncrementReferenceCount();
						o_handle = existingHandle;
						return Results::Success;
					}
				}
				// If this code is reached it means that the existing entry is invalid
				m_map_pathsToHandles.erase( iterator );
			}
		}
	}

	// If the asset hasn't already been loaded load it now
	auto result = Results::Success;

	tAsset* newAsset;
	if ( result = tAsset::Load( i_path, newAsset, std::forward<tConstructorArguments>( i_constructorArguments )... ) )
	{
		// Lock the collections
		Concurrency::cMutex::cScopeLock autoLock( m_mutex );
		{
			// Look for an existing asset record that is unused
			if ( !m_unusedAssetRecordIndices.empty() )
			{
				const auto index = m_unusedAssetRecordIndices.back();
				m_unusedAssetRecordIndices.pop_back();
				auto &assetRecord = m_assetRecords[index];
				assetRecord.asset = newAsset;
				o_handle = cHandle<tAsset>( index, assetRecord.id );
			}
			else
			{
				// Create a new asset record
				const auto assetRecordCount = m_assetRecords.size();
				if ( assetRecordCount < cHandle<tAsset>::InvalidIndex )
				{
					const auto index = static_cast<uint_fast32_t>( assetRecordCount );
					constexpr uint32_t id = 0;
					m_assetRecords.push_back( sAssetRecord{ newAsset, id } );
					o_handle = cHandle<tAsset>( index, id );
				}
				else
				{
					result = Results::OutOfMemory;
					EAE6320_ASSERTF( false, "Too many of this kind of asset have been created" );
					Logging::OutputError( "A new asset couldn't be loaded because there were too many (%u)", assetRecordCount );
				}
			}
			if ( result )
			{
				m_map_pathsToHandles.insert( std::make_pair( i_path, o_handle ) );
			}
		}
	}

	if ( !result && newAsset )
	{
		newAsset->DecrementReferenceCount();
		newAsset = nullptr;
	}

	return result;
}

template <class tAsset>
	eae6320::cResult eae6320::Assets::cManager<tAsset>::Release( cHandle<tAsset>& o_handle )
{
	auto result = Results::Success;

	EAE6320_ASSERTF( o_handle, "This handle is invalid (it has never been associated with a valid asset)" );
	// Lock the collections
	{
		Concurrency::cMutex::cScopeLock autoLock( m_mutex );
		{
			const auto index = o_handle.GetIndex();
			const auto assetCount = m_assetRecords.size();
			if ( index < assetCount )
			{
				auto &assetRecord = m_assetRecords[index];
				const auto id_assetRecord = assetRecord.id;
				const auto id_handle = o_handle.GetId();
				if ( id_handle == id_assetRecord )
				{
					const auto newReferenceCount = assetRecord.asset->DecrementReferenceCount();
					if ( newReferenceCount == 0 )
					{
						assetRecord.asset = nullptr;
						assetRecord.id = cHandle<tAsset>::IncrementId( id_assetRecord );
						m_unusedAssetRecordIndices.push_back( index );
					}
				}
				else
				{
					EAE6320_ASSERTF( false, "A handle attempting to be released (at index %u) has an ID (%u) that doesn't match the asset record (%u)",
						index, id_handle, id_assetRecord );
					result = Results::Failure;
				}
			}
			else
			{
				EAE6320_ASSERTF( false, "A handle attempting to be released has an index (%u) that's too big for the number of assets (%u)",
					index, assetCount );
				result = Results::Failure;
			}
		}
	}
	// Make the handle invalid so that the caller no longer has access to an asset it doesn't hold a reference to
	o_handle.MakeInvalid();

	return result;
}

// Initialization / Clean Up
//==========================

template <class tAsset>
	eae6320::cResult eae6320::Assets::cManager<tAsset>::Initialize()
{
	return Results::Success;
}

template <class tAsset>
	eae6320::cResult eae6320::Assets::cManager<tAsset>::CleanUp()
{
	auto result = Results::Success;

	{
		bool wereThereStillAssets = false;

		// Lock the collections
		{
			Concurrency::cMutex::cScopeLock autoLock( m_mutex );
			const auto assetRecordCount = m_assetRecords.size();
			for ( size_t i = 0; i < assetRecordCount; ++i )
			{
				auto& assetRecord = m_assetRecords[i];
				if ( assetRecord.asset )
				{
					EAE6320_ASSERTF( false, "A manager still has a record of an asset that hasn't been released" );
					result = Results::Failure;
					wereThereStillAssets = true;
					// The asset's reference count could be decremented until it gets destroyed,
					// but there's no way of knowing that the asset still isn't being used
					// and so the asset will leak
					assetRecord.asset = nullptr;
					// The following shouldn't be necessary since the manager is being cleaned up,
					// but it doesn't hurt to be safe
					assetRecord.id = cHandle<tAsset>::IncrementId( assetRecord.id );
				}
			}
		}

		if ( wereThereStillAssets )
		{
			Logging::OutputError( "A manager still had asset records while it was being cleaned up" );
		}
	}

	return result;
}

template <class tAsset>
	eae6320::Assets::cManager<tAsset>::~cManager<tAsset>()
{
	const auto result = CleanUp();
	EAE6320_ASSERT( result );
}

#endif	// EAE6320_ASSETS_CMANAGER_INL
