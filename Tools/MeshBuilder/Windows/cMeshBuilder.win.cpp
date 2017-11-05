// Include Files
//==============

#include "../cMeshBuilder.h"

#include <Engine/Math/Functions.h>
#include <Engine/Platform/Platform.h>
#include <Tools/AssetBuildLibrary/Functions.h>

#include <string>

// Inherited Implementation
//=========================

// Build
//------

eae6320::cResult eae6320::Assets::cMeshBuilder::Build(const std::vector<std::string>& i_arguments)
{
    auto result = eae6320::Results::Success;

    std::string errorMessage;
    // Copy the mesh from the source directory to the destination directory
    if (!(result = eae6320::Platform::CopyFile(
        m_path_source,
        m_path_target,
        /*i_shouldFunctionFailIfTargetAlreadyExists = */ false,
        /*i_shouldTargetFileTimeBeModified = */ true,
        &errorMessage)))
    {
        eae6320::Assets::OutputErrorMessageWithFileInfo(m_path_source, errorMessage.c_str());
    }

    return result;
}
