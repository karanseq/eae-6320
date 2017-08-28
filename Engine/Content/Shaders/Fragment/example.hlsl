/*
	This is an example fragment shader used to render geometry
*/

#include <Shaders/shaders.inc>

// Constant Buffers
//=================

cbuffer g_constantBuffer_perFrame : register( b0 )
{
	float g_elapsedSecondCount_systemTime;
	float g_elapsedSecondCount_simulationTime;
	// For float4 alignment
	float2 g_padding;
}

cbuffer g_constantBuffer_perMaterial : register( b1 )
{
	float4 g_color;
}

// Entry Point
//============

void main(

	// Output
	//=======

	// Whatever color value is output from the fragment shader
	// will determine the color of the corresponding pixel on the screen
	out float4 o_color : SV_TARGET

	)
{
	// Output solid white
	o_color = float4(
		// RGB
		1.0, 1.0, 1.0,
		// Alpha
		1.0 );

	// EAE6320_TODO: Change the color based on time!
	// The value g_elapsedSecondCount_simulationTime constantly changes as the simulation progresses, and so by doing something like:
	//	sin( g_elapsedSecondCount_simulationTime ) or cos( g_elapsedSecondCount_simulationTime )
	// you can get a value that will oscillate between [-1,1].
	// You should change at least one "channel" so that the color animates.
	// For example, to change red ("r") you would do something kind of like:
	//		o_color.r = ? sin( g_elapsedSecondCount_simulationTime ) ?
	// You can change .r, .g, and .b (but leave .a as 1.0).
	// Remember that your final values should be [0,1], so you will have to do some math.
}
