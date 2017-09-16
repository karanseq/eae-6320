// Include Files
//==============

#include "cExampleGame.h"

#include <Engine/Asserts/Asserts.h>
#include <Engine/Graphics/Graphics.h>
#include <Engine/Logging/Logging.h>
#include <Engine/UserInput/UserInput.h>

// Inherited Implementation
//=========================

// Run
//----

void eae6320::cExampleGame::UpdateBasedOnInput()
{
	// Is the user pressing the ESC key?
	if ( UserInput::IsKeyPressed( UserInput::KeyCodes::Escape ) )
	{
		// Exit the application
		const auto result = Exit( EXIT_SUCCESS );
		EAE6320_ASSERT( result );
	}
}

void eae6320::cExampleGame::UpdateBasedOnTime(const float i_elapsedSecondCount_sinceLastUpdate)
{
	// Animate the background
	{
		static float systemTime = 0.0f;
		systemTime += i_elapsedSecondCount_sinceLastUpdate;
		m_backgroundColor.r = 1.0f + (cosf(systemTime) * 0.5f) - 0.5f;
		m_backgroundColor.g = 1.0f + (sinf(systemTime) * 0.5f) - 0.5f;
		m_backgroundColor.b = 1.0f + (acosf(systemTime) * 0.5f) - 0.5f;
	}
}

void eae6320::cExampleGame::SubmitDataToBeRendered(const float i_elapsedSecondCount_systemTime, const float i_elapsedSecondCount_sinceLastSimulationUpdate)
{
	eae6320::Graphics::SubmitBackgroundColor(m_backgroundColor);
}

// Initialization / Clean Up
//--------------------------

eae6320::cResult eae6320::cExampleGame::Initialize()
{
	return Results::Success;
}

eae6320::cResult eae6320::cExampleGame::CleanUp()
{
	return Results::Success;
}
