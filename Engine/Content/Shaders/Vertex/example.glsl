/*
	This is an example vertex shader used to render geometry
*/

#include <Shaders/shaders.inc>

// Constant Buffers
//=================

layout( std140, binding = 0 ) uniform g_constantBuffer_perFrame
{
	float g_elapsedSecondCount_systemTime;
	float g_elapsedSecondCount_simulationTime;
	// For vec4 alignment
	vec2 g_padding;
};

layout( std140, binding = 1 ) uniform g_constantBuffer_perMaterial
{
	vec4 g_color;
};

layout( std140, binding = 2 ) uniform g_constantBuffer_perDrawCall
{
	// This is a placeholder to prevent an empty constant buffer declaration
	vec4 DUMMY;
};

// Input
//======

// The locations assigned are arbitrary
// but must match the C calls to glVertexAttribPointer()

// These values come from one of the VertexFormats::sGeometry that the vertex buffer was filled with in C code
layout( location = 0 ) in vec2 i_position;

// Output
//=======

// The vertex shader must always output a position value,
// but unlike HLSL where the value is explicit
// GLSL has an automatically-required variable named "gl_Position"

// Entry Point
//============

void main()
{
	// Calculate the position of this vertex on screen
	{
		// This example shader sets the "out" position directly from the "in" position:
		gl_Position = vec4( i_position.x, i_position.y, 0.0, 1.0 );
		// Both of the following lines are exactly equivalent to the one above
		gl_Position = vec4( i_position.xy, 0.0, 1.0 );
		gl_Position = vec4( i_position, 0.0, 1.0 );
	}

	// EAE6320_TODO: Change the position based on time!
	// The value g_elapsedSecondCount_simulationTime constantly changes as the simulation progresses, and so by doing something like:
	//	sin( g_elapsedSecondCount_simulationTime ) or cos( g_elapsedSecondCount_simulationTime )
	// you can get a value that will oscillate between [-1,1].
	// You should change at least one position element so that the triangle animates.
	// For example, to change X you would do something kind of like:
	//		gl_Position.x = ? sin( g_elapsedSecondCount_simulationTime ) ?
	// You can change .x and .y (but leave .z as 0.0 and .w as 1.0).
	// The screen dimensions are already [1,1], so you may want to do some math
	// on the result of the sinusoid function to keep the triangle mostly on screen.
}
