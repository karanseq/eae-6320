/*
	The main() function is where the program starts execution
*/

// Include Files
//==============

#include <cstdlib>
#include <Engine/Platform/Platform.h>
#include <Engine/Results/Results.h>
#include <Tools/AssetBuildLibrary/Functions.h>
#include <string>

// Entry Point
//============

int main( int i_argumentCount, char** i_arguments )
{
	auto result = eae6320::Results::Success;

	// Get the necessary directories
	std::string engineSourceContentDir, gameInstallDir, gameLicenseDir, licenseDir, outputDir;
	{
		std::string errorMessage;
		if ( !( result = eae6320::Platform::GetEnvironmentVariable( "EngineSourceContentDir", engineSourceContentDir, &errorMessage ) ) )
		{
			eae6320::Assets::OutputErrorMessageWithFileInfo( __FILE__, __LINE__, errorMessage.c_str() );
			goto OnExit;
		}
		if ( !( result = eae6320::Platform::GetEnvironmentVariable( "GameInstallDir", gameInstallDir, &errorMessage ) ) )
		{
			eae6320::Assets::OutputErrorMessageWithFileInfo( __FILE__, __LINE__, errorMessage.c_str() );
			goto OnExit;
		}
		if ( !( result = eae6320::Platform::GetEnvironmentVariable( "GameLicenseDir", gameLicenseDir, &errorMessage ) ) )
		{
			eae6320::Assets::OutputErrorMessageWithFileInfo( __FILE__, __LINE__, errorMessage.c_str() );
			goto OnExit;
		}
		if ( !( result = eae6320::Platform::GetEnvironmentVariable( "LicenseDir", licenseDir, &errorMessage ) ) )
		{
			eae6320::Assets::OutputErrorMessageWithFileInfo( __FILE__, __LINE__, errorMessage.c_str() );
			goto OnExit;
		}
		if ( !( result = eae6320::Platform::GetEnvironmentVariable( "OutputDir", outputDir, &errorMessage ) ) )
		{
			eae6320::Assets::OutputErrorMessageWithFileInfo( __FILE__, __LINE__, errorMessage.c_str() );
			goto OnExit;
		}
	}
	// Build the shaders and copy them to the installation location
	{
		const auto path_shaderBuilder = outputDir + "ShaderBuilder.exe";
		std::string errorMessage;
		int exitCode;
		{
			const auto shader_authored = engineSourceContentDir + "Shaders/Vertex/example."
#if defined( EAE6320_PLATFORM_D3D )
				"hlsl"
#elif defined( EAE6320_PLATFORM_GL )
				"glsl"
#endif
				;
			const auto shader_built = gameInstallDir + "data/Shaders/Vertex/example.shd";
			if ( result = eae6320::Platform::CreateDirectoryIfItDoesntExist( shader_built, &errorMessage) )
			{
				if ( result = eae6320::Platform::ExecuteCommand( ( std::string( "\"" ) + path_shaderBuilder
					+ "\" \"" + shader_authored + "\" \"" + shader_built + "\" vertex" ).c_str(),
					&exitCode, &errorMessage ) )
				{
					if ( exitCode != EXIT_SUCCESS )
					{
						return exitCode;
					}
				}
				else
				{
					eae6320::Assets::OutputErrorMessageWithFileInfo( __FILE__, __LINE__, errorMessage.c_str() );
					goto OnExit;
				}
			}
			else
			{
				eae6320::Assets::OutputErrorMessageWithFileInfo( __FILE__, __LINE__, errorMessage.c_str() );
				goto OnExit;
			}
		}
		{
			const auto shader_authored = engineSourceContentDir + "Shaders/Fragment/example."
#if defined( EAE6320_PLATFORM_D3D )
				"hlsl"
#elif defined( EAE6320_PLATFORM_GL )
				"glsl"
#endif
				;
			const auto shader_built = gameInstallDir + "data/Shaders/Fragment/example.shd";
			if ( result = eae6320::Platform::CreateDirectoryIfItDoesntExist( shader_built, &errorMessage ) )
			{
				if ( result = eae6320::Platform::ExecuteCommand( ( std::string( "\"" ) + path_shaderBuilder
					+ "\" \"" + shader_authored + "\" \"" + shader_built + "\" fragment" ).c_str(),
					&exitCode, &errorMessage ) )
				{
					if ( exitCode != EXIT_SUCCESS )
					{
						return exitCode;
					}
				}
				else
				{
					eae6320::Assets::OutputErrorMessageWithFileInfo( __FILE__, __LINE__, errorMessage.c_str() );
					goto OnExit;
				}
			}
			else
			{
				eae6320::Assets::OutputErrorMessageWithFileInfo( __FILE__, __LINE__, errorMessage.c_str() );
				goto OnExit;
			}
		}
#if defined( EAE6320_PLATFORM_D3D )
		{
			const auto shader_authored = engineSourceContentDir + "Shaders/Vertex/vertexInputLayout_geometry.hlsl";
			const auto shader_built = gameInstallDir + "data/Shaders/Vertex/vertexInputLayout_geometry.shd";
			if ( result = eae6320::Platform::CreateDirectoryIfItDoesntExist( shader_built, &errorMessage ) )
			{
				if ( result = eae6320::Platform::ExecuteCommand( ( std::string( "\"" ) + path_shaderBuilder
					+ "\" \"" + shader_authored + "\" \"" + shader_built + "\" vertex" ).c_str(),
					&exitCode, &errorMessage ) )
				{
					if ( exitCode != EXIT_SUCCESS )
					{
						return exitCode;
					}
				}
				else
				{
					eae6320::Assets::OutputErrorMessageWithFileInfo( __FILE__, __LINE__, errorMessage.c_str() );
					goto OnExit;
				}
			}
			else
			{
				eae6320::Assets::OutputErrorMessageWithFileInfo( __FILE__, __LINE__, errorMessage.c_str() );
				goto OnExit;
			}
		}
#endif
	}
	// Copy the licenses to the installation location
	{
		std::string errorMessage;
		if ( result = eae6320::Platform::CreateDirectoryIfItDoesntExist( gameLicenseDir, &errorMessage ) )
		{
			std::vector<std::string> licensePaths;
			constexpr auto searchRecursively = true;
			if ( result = eae6320::Platform::GetFilesInDirectory( licenseDir, licensePaths, searchRecursively, &errorMessage ) )
			{
				for ( const auto& sourcePath : licensePaths )
				{
					const auto pos_lastSlash = sourcePath.find_last_of( "/\\" );
					const auto fileName = ( pos_lastSlash != sourcePath.npos )
						? sourcePath.substr( pos_lastSlash + 1 ) : sourcePath;
					const auto targetPath = gameLicenseDir + fileName;
					constexpr auto dontFailIfTargetAlreadyExists = false;
					constexpr auto modifyTargetTime = true;
					if ( !( result = eae6320::Platform::CopyFile( sourcePath.c_str(), targetPath.c_str(),
						dontFailIfTargetAlreadyExists, modifyTargetTime, &errorMessage ) ) )
					{
						eae6320::Assets::OutputErrorMessageWithFileInfo( __FILE__, __LINE__, errorMessage.c_str() );
						goto OnExit;
					}
				}
			}
			else
			{
				eae6320::Assets::OutputErrorMessageWithFileInfo( __FILE__, __LINE__, errorMessage.c_str() );
				goto OnExit;
			}
		}
		else
		{
			eae6320::Assets::OutputErrorMessageWithFileInfo( __FILE__, __LINE__, errorMessage.c_str() );
			goto OnExit;
		}
	}

OnExit:

	return result ? EXIT_SUCCESS : EXIT_FAILURE;
}
