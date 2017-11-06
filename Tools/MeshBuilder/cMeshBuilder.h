/*
    This class builds meshes from human readable lua files.
*/

#ifndef EAE6320_CMESHBUILDER_H
#define EAE6320_CMESHBUILDER_H

// Include Files
//==============

#include <Tools/AssetBuildLibrary/cbBuilder.h>

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
        };
    }
}

#endif // EAE6320_CMESHBUILDER_H

