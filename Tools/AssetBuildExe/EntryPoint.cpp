/*
The main() function is where the program starts execution
*/

// Include Files
//==============

#include <cstdlib>
#include <Engine/Results/Results.h>
#include <Tools/AssetBuildLibrary/Functions.h>

// Entry Point
//============

int main(int i_argumentCount, char** i_arguments)
{
    const auto result = eae6320::Assets::BuildAssets();
    return result ? EXIT_SUCCESS : EXIT_FAILURE;
}
