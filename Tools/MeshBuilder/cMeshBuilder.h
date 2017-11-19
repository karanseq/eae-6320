/*
    This class builds meshes from human readable lua files.
*/

#ifndef EAE6320_CMESHBUILDER_H
#define EAE6320_CMESHBUILDER_H

// Include Files
//==============

#include <Tools/AssetBuildLibrary/cbBuilder.h>

// Forward Declarations
//=====================

namespace eae6320
{
    namespace Graphics
    {
        namespace VertexFormats
        {
            struct sMesh;
        }
    }
}

// Class Declaration
//==================

namespace eae6320
{
    namespace Assets
    {
        class cMeshBuilder : public cbBuilder
        {
            // Inherited Implementation
            //=========================

        private:

            // Build
            //------

            virtual cResult Build(const std::vector<std::string>& i_arguments) override;

            void PerformPlatformSpecificFixup(const uint16_t i_vertexCount, eae6320::Graphics::VertexFormats::sMesh* io_vertexData, const uint16_t i_indexCount, uint16_t* io_indexData);
        };
    }
}

#endif // EAE6320_CMESHBUILDER_H

