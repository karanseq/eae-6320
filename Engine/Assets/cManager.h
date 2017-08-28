/*
	An asset manager is in charge of a specific asset type:
		* The manager works like a factory when an asset is loaded
			and ensures that a single asset is only loaded once even if multiple load requests are made
		* The manager tracks handles for assets,
			and can return the actual asset pointer given its handle
		* When an asset's reference count is decremented to 0 and it is destroyed
			the manager takes care of handle clean up
*/

#ifndef EAE6320_ASSETS_CMANAGER_H
#define EAE6320_ASSETS_CMANAGER_H

// Include Files
//==============

#include "cHandle.h"
#include <Engine/Concurrency/cMutex.h>
#include <Engine/Results/Results.h>
#include <map>
#include <string>
#include <vector>

// Interface
//==========

namespace eae6320
{
	namespace Assets
	{
		template <class tAsset>
			class cManager
		{
			// Interface
			//==========

		public:

			tAsset* Get( const cHandle<tAsset> i_handle );

			template <typename... tConstructorArguments>
				cResult Load( const char* const i_path, cHandle<tAsset>& o_handle, tConstructorArguments&&... i_constructorArguments );
			cResult Release( cHandle<tAsset>& io_handle );

			// Initialization / Clean Up
			//==========================

			cResult Initialize();
			cResult CleanUp();

			~cManager<tAsset>();

			// Data
			//=====

		private:

			struct sAssetRecord
			{
				tAsset* asset;
				uint32_t id;
			};
			std::vector<sAssetRecord> m_assetRecords;
			std::vector<uint32_t> m_unusedAssetRecordIndices;
			std::map< std::string, cHandle<tAsset> > m_map_pathsToHandles;
			eae6320::Concurrency::cMutex m_mutex;
		};
	}
}

#include "cManager.inl"

#endif	// EAE6320_ASSETS_CMANAGER_H
