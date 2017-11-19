// Include Files
//==============

#include "cMeshBuilder.h"

#include <Engine/Math/Functions.h>
#include <Engine/Graphics/sColor.h>
#include <Engine/Graphics/VertexFormats.h>
#include <Engine/Logging/Logging.h>
#include <Engine/Math/sVector.h>
#include <Engine/Math/sVector2d.h>
#include <Engine/Platform/Platform.h>
#include <External/Lua/Includes.h>
#include <Tools/AssetBuildLibrary/Functions.h>

#include <fstream>
#include <string>

// Helper Function Declarations
//=============================

namespace
{
    eae6320::cResult LoadIntegerArray(lua_State& io_luaState, const uint16_t i_integerCount, uint16_t* o_integerArray);
    eae6320::cResult LoadFloatArray(lua_State& io_luaState, const uint16_t i_floatCount, float* o_floatArray);
    eae6320::cResult LoadPosition(lua_State& io_luaState, eae6320::Math::sVector& o_position);
    eae6320::cResult LoadColor(lua_State& io_luaState, eae6320::Graphics::sColor& o_color);
    eae6320::cResult LoadUV(lua_State& io_luaState, eae6320::Math::sVector2d& o_position);
    eae6320::cResult LoadVertexDataArray(lua_State& io_luaState, uint16_t& o_vertexCount, eae6320::Graphics::VertexFormats::sMesh*& o_vertexData);
    eae6320::cResult LoadVertexData(lua_State& io_luaState, uint16_t& o_vertexCount, eae6320::Graphics::VertexFormats::sMesh*& o_vertexData);
    eae6320::cResult LoadIndexDataArray(lua_State& io_luaState, uint16_t& o_indexCount, uint16_t*& o_indices);
    eae6320::cResult LoadIndexData(lua_State& io_luaState, uint16_t& o_indexCount, uint16_t*& o_indices);
    eae6320::cResult WriteBuiltMesh(const char* i_path, const uint16_t i_vertexCount, const eae6320::Graphics::VertexFormats::sMesh* i_vertexData, const uint16_t i_indexCount, const uint16_t* i_indices);
}

// Inherited Implementation
//=========================

// Build
//------

eae6320::cResult eae6320::Assets::cMeshBuilder::Build(const std::vector<std::string>& i_arguments)
{
    auto result = eae6320::Results::Success;

    //std::string errorMessage;
    //// Copy the mesh from the source directory to the destination directory
    //if (!(result = eae6320::Platform::CopyFile(
    //    m_path_source,
    //    m_path_target,
    //    /*i_shouldFunctionFailIfTargetAlreadyExists = */ false,
    //    /*i_shouldTargetFileTimeBeModified = */ true,
    //    &errorMessage)))
    //{
    //    eae6320::Assets::OutputErrorMessageWithFileInfo(m_path_source, errorMessage.c_str());
    //}
    
    // Create a new Lua state
    lua_State* luaState = nullptr;
    {
        luaState = luaL_newstate();
        if (!luaState)
        {
            result = eae6320::Results::OutOfMemory;
            OutputErrorMessageWithFileInfo(m_path_source, "Failed to create a new Lua state while loading %s", m_path_source);
            goto OnExit;
        }
    }

    // Load the asset file as a "chunk",
    // meaning there will be a callable function at the top of the stack
    const auto stackTopBeforeLoad = lua_gettop(luaState);
    {
        const auto luaResult = luaL_loadfile(luaState, m_path_source);
        if (luaResult != LUA_OK)
        {
            result = eae6320::Results::Failure;
            OutputErrorMessageWithFileInfo(m_path_source, "%s while loading %s", lua_tostring(luaState, -1), m_path_source);
            // Pop the error message
            lua_pop(luaState, 1);
            goto OnExit;
        }
    }

    // Execute the "chunk", which should load the asset
    // into a table at the top of the stack
    {
        constexpr int argumentCount = 0;
        constexpr int returnValueCount = LUA_MULTRET;	// Return _everything_ that the file returns
        constexpr int noMessageHandler = 0;
        const auto luaResult = lua_pcall(luaState, argumentCount, returnValueCount, noMessageHandler);
        if (luaResult == LUA_OK)
        {
            // A well-behaved asset file will only return a single value
            const auto returnedValueCount = lua_gettop(luaState) - stackTopBeforeLoad;
            if (returnedValueCount == 1)
            {
                // A correct asset file _must_ return a table
                if (!lua_istable(luaState, -1))
                {
                    result = eae6320::Results::InvalidFile;
                    OutputErrorMessageWithFileInfo(m_path_source, "Asset files must return a table (instead of a %s)", luaL_typename(luaState, -1));
                    // Pop the returned non-table value
                    lua_pop(luaState, 1);
                    goto OnExit;
                }
            }
            else
            {
                result = eae6320::Results::InvalidFile;
                OutputErrorMessageWithFileInfo(m_path_source, "Asset files must return a single table (instead of %d values)", returnedValueCount);
                // Pop every value that was returned
                lua_pop(luaState, returnedValueCount);
                goto OnExit;
            }
        }
        else
        {
            result = eae6320::Results::InvalidFile;
            OutputErrorMessageWithFileInfo(m_path_source, lua_tostring(luaState, -1));
            // Pop the error message
            lua_pop(luaState, 1);
            goto OnExit;
        }
    }

    // If this code is reached the asset file was loaded successfully,
    // and its table is now at index -1
    uint16_t vertexCount = 0;
    eae6320::Graphics::VertexFormats::sMesh* vertexData = nullptr;

    if (!(result = LoadVertexData(*luaState, vertexCount, vertexData)))
    {
        result = Results::Failure;
        OutputErrorMessageWithFileInfo(m_path_source, "Couldn't parse vertex data for file %s", m_path_source);
        goto OnExit;
    }

    uint16_t indexCount = 0;
    uint16_t* indices = nullptr;

    if (!(result = LoadIndexData(*luaState, indexCount, indices)))
    {
        result = Results::Failure;
        OutputErrorMessageWithFileInfo(m_path_source, "Couldn't parse index data for file %s", m_path_source);
        goto OnExit;
    }

    // Pop the table
    lua_pop(luaState, 1);

    PerformPlatformSpecificFixup(vertexCount, vertexData, indexCount, indices);

    if (!(result = WriteBuiltMesh(m_path_target, vertexCount, vertexData, indexCount, indices)))
    {
        result = Results::Failure;
        OutputErrorMessageWithFileInfo(m_path_source, "Couldn't write built mesh for file %s", m_path_source);
        goto OnExit;
    }

OnExit:

    // Deallocate the vertices, UVs and indices
    {
        if (vertexData)
        {
            free(vertexData);
            vertexData = nullptr;
        }

        if (indices)
        {
            free(indices);
            indices = nullptr;
        }
    }

    if (luaState)
    {
        // If I haven't made any mistakes
        // there shouldn't be anything on the stack,
        // regardless of any errors encountered while loading the file:
        if (lua_gettop(luaState) != 0)
        {
            OutputErrorMessageWithFileInfo(m_path_source, "Lua stack wasn't empty when finished building mesh");
        }

        lua_close(luaState);
        luaState = nullptr;
    }

    return result;
}

// Helper Function Definitions
//============================

namespace
{
    eae6320::cResult LoadIntegerArray(lua_State& io_luaState, const uint16_t i_integerCount, uint16_t* o_integerArray)
    {
        // Check that the table we're about to read has the desired number of elements
        const auto intArraySize = luaL_len(&io_luaState, -1);
        if (intArraySize != i_integerCount)
        {
            eae6320::Assets::OutputErrorMessage("Found %d elements in the integer array(instead of %d)", intArraySize, i_integerCount);
            return eae6320::Results::InvalidFile;
        }

        for (auto i = 1; i <= intArraySize; ++i)
        {
            // After lua_gettable, the table containing the int array will be at -2,
            // and the i'th element of the int array will be at -1
            lua_pushinteger(&io_luaState, i);
            lua_gettable(&io_luaState, -2);

            // Check if the value in the int array is an integer
            if (lua_isinteger(&io_luaState, -1))
            {
                uint16_t valueAtI = static_cast<uint16_t>(lua_tointeger(&io_luaState, -1));
                o_integerArray[i - 1] = valueAtI;
                lua_pop(&io_luaState, 1);
            }
            else
            {
                // Right now, the table containing the int array will be at -2,
                // and the i'th element of the int array will be at -1
                lua_pop(&io_luaState, 1);

                eae6320::Assets::OutputErrorMessage("Each element in an int array must be a number (instead of a %s)", luaL_typename(&io_luaState, -1));
                return eae6320::Results::InvalidFile;
            }
        }

        return eae6320::Results::Success;
    }

    eae6320::cResult LoadFloatArray(lua_State& io_luaState, const uint16_t i_floatCount, float* o_floatArray)
    {
        // Check that the table we're about to read has the desired number of elements
        const auto floatArraySize = luaL_len(&io_luaState, -1);
        if (floatArraySize != i_floatCount)
        {
            eae6320::Assets::OutputErrorMessage("Found %d elements in the float array(instead of %d)", floatArraySize, i_floatCount);
            return eae6320::Results::InvalidFile;
        }

        for (auto i = 1; i <= floatArraySize; ++i)
        {
            // After lua_gettable, the table containing the float array will be at -2,
            // and the i'th element of the float array will be at -1
            lua_pushinteger(&io_luaState, i);
            lua_gettable(&io_luaState, -2);

            // Check if the value in the float array is a number
            if (lua_isnumber(&io_luaState, -1))
            {
                float valueAtI = static_cast<float>(lua_tonumber(&io_luaState, -1));
                o_floatArray[i - 1] = valueAtI;
                lua_pop(&io_luaState, 1);
            }
            else
            {
                // Right now, the table containing the float array will be at -2,
                // and the i'th element of the float array will be at -1
                lua_pop(&io_luaState, 1);

                eae6320::Assets::OutputErrorMessage("Each element in a float array must be a number (instead of a %s)", luaL_typename(&io_luaState, -1));
                return eae6320::Results::InvalidFile;
            }
        }

        return eae6320::Results::Success;
    }

    eae6320::cResult LoadPosition(lua_State& io_luaState, eae6320::Math::sVector& o_position)
    {
        auto result = eae6320::Results::Success;

        // After lua_gettable the asset table will be at -4,
        // the vertexData table will be at -3,
        // the i'th element of the vertexData table will be at -2
        // and the position table will be at -1
        const auto* const key = "position";
        lua_pushstring(&io_luaState, key);
        lua_gettable(&io_luaState, -2);

        // Make sure that the value is a table
        if (lua_istable(&io_luaState, -1))
        {
            constexpr uint16_t floatsPerPosition = 3;
            float floatArray[floatsPerPosition];

            // Load the float array representing position
            if (!(result = LoadFloatArray(io_luaState, floatsPerPosition, floatArray)))
            {
                result = eae6320::Results::InvalidFile;
                eae6320::Assets::OutputErrorMessage("Couldn't load the floats within a position");
                goto OnExit;
            }

            // Fill the output
            o_position.x = floatArray[0];
            o_position.y = floatArray[1];
            o_position.z = floatArray[2];
        }
        else
        {
            result = eae6320::Results::InvalidFile;
            eae6320::Assets::OutputErrorMessage("The position value must be a table (instead of a %s)", luaL_typename(&io_luaState, -1));
            goto OnExit;
        }

    OnExit:

        // Right now the asset table is at -4,
        // the vertexData table is at -3,
        // the i'th element of the vertexData table is at -2
        // and the position table is at -1
        lua_pop(&io_luaState, 1);

        return result;
    }

    eae6320::cResult LoadColor(lua_State& io_luaState, eae6320::Graphics::sColor& o_color)
    {
        auto result = eae6320::Results::Success;

        // After lua_gettable the asset table will be at -4,
        // the vertexData table will be at -3,
        // the i'th element of the vertexData table will be at -2
        // and the color table will be at -1
        const auto* const key = "color";
        lua_pushstring(&io_luaState, key);
        lua_gettable(&io_luaState, -2);

        // Make sure that the value is a table
        if (lua_istable(&io_luaState, -1))
        {
            constexpr uint16_t floatsPerColor = 4;
            float floatArray[floatsPerColor];

            // Load the float array representing color
            if (!(result = LoadFloatArray(io_luaState, floatsPerColor, floatArray)))
            {
                result = eae6320::Results::InvalidFile;
                eae6320::Assets::OutputErrorMessage("Couldn't load the floats within a color");
                goto OnExit;
            }

            // Fill the output
            o_color.r = floatArray[0];
            o_color.g = floatArray[1];
            o_color.b = floatArray[2];
            o_color.a = floatArray[3];
        }
        else
        {
            result = eae6320::Results::InvalidFile;
            eae6320::Assets::OutputErrorMessage("The color value must be a table (instead of a %s)", luaL_typename(&io_luaState, -1));
            goto OnExit;
        }

    OnExit:

        // Right now the asset table is at -4,
        // the vertexData table is at -3,
        // the i'th element of the vertexData table is at -2
        // and the color table is at -1
        lua_pop(&io_luaState, 1);

        return result;
    }

    eae6320::cResult LoadUV(lua_State& io_luaState, eae6320::Math::sVector2d& o_position)
    {
        auto result = eae6320::Results::Success;

        // After lua_gettable the asset table will be at -4,
        // the vertexData table will be at -3,
        // the i'th element of the vertexData table will be at -2
        // and the UV table will be at -1
        const auto* const key = "uv";
        lua_pushstring(&io_luaState, key);
        lua_gettable(&io_luaState, -2);

        // Make sure that the value is a table
        if (lua_istable(&io_luaState, -1))
        {
            constexpr uint16_t floatsPerUV = 2;
            float floatArray[floatsPerUV];

            // Load the float array representing position
            if (!(result = LoadFloatArray(io_luaState, floatsPerUV, floatArray)))
            {
                result = eae6320::Results::InvalidFile;
                eae6320::Assets::OutputErrorMessage("Couldn't load the floats within a UV");
                goto OnExit;
            }

            // Fill the output
            o_position.x = floatArray[0];
            o_position.y = floatArray[1];
        }
        else
        {
            result = eae6320::Results::InvalidFile;
            eae6320::Assets::OutputErrorMessage("The UV value must be a table (instead of a %s)", luaL_typename(&io_luaState, -1));
            goto OnExit;
        }

    OnExit:

        // Right now the asset table is at -4,
        // the vertexData table is at -3,
        // the i'th element of the vertexData table is at -2
        // and the UV table is at -1
        lua_pop(&io_luaState, 1);

        return result;
    }

    eae6320::cResult LoadVertexDataArray(lua_State& io_luaState, uint16_t& o_vertexCount, eae6320::Graphics::VertexFormats::sMesh*& o_vertexData)
    {
        auto result = eae6320::Results::Success;

        // Right now the asset table is at -2,
        // and the vertexData table is at -1.
        const auto vertexDataArraySize = luaL_len(&io_luaState, -1);
        if (vertexDataArraySize < 3)
        {
            result = eae6320::Results::InvalidFile;
            eae6320::Assets::OutputErrorMessage("The vertexData must have at least three elements");
            goto OnExit;
        }

        o_vertexCount = static_cast<uint16_t>(vertexDataArraySize);
        // reserve memory for the vertex data
        o_vertexData = static_cast<eae6320::Graphics::VertexFormats::sMesh*>(malloc(o_vertexCount * sizeof(eae6320::Graphics::VertexFormats::sMesh)));

        for (auto i = 1; i <= vertexDataArraySize; ++i)
        {
            // After lua_gettable the asset table will be at -3,
            // the vertexData table will be at -2,
            // and i'th element of the vertexData table will be at -1
            lua_pushinteger(&io_luaState, i);
            lua_gettable(&io_luaState, -2);

            // Make sure that the value is a table
            if (lua_istable(&io_luaState, -1))
            {
                // Load the vertex position
                {
                    eae6320::Math::sVector position;
                    if (!(result = LoadPosition(io_luaState, position)))
                    {
                        // Right now the asset table is at -3,
                        // the vertexData table is at -2,
                        // and i'th element of the vertexData table is at -1
                        lua_pop(&io_luaState, 1);

                        result = eae6320::Results::InvalidFile;
                        eae6320::Assets::OutputErrorMessage("Couldn't load the position for %d in the vertexData", i);
                        goto OnExit;
                    }
                    o_vertexData[i - 1].x = position.x;
                    o_vertexData[i - 1].y = position.y;
                    o_vertexData[i - 1].z = position.z;
                }

                // Load the vertex UV
                {
                    eae6320::Math::sVector2d uv;
                    if (!(result = LoadUV(io_luaState, uv)))
                    {
                        // Right now the asset table is at -3,
                        // the vertexData table is at -2,
                        // and i'th element of the vertexData table is at -1
                        lua_pop(&io_luaState, 1);

                        result = eae6320::Results::InvalidFile;
                        eae6320::Assets::OutputErrorMessage("Couldn't load the UV for %d in the vertexData", i);
                        goto OnExit;
                    }
                    o_vertexData[i - 1].u = uv.x;
                    o_vertexData[i - 1].v = uv.y;
                }
            }
            else
            {
                // Right now the asset table is at -3,
                // the vertexData table is at -2,
                // and i'th element of the vertexData table is at -1
                lua_pop(&io_luaState, 1);

                result = eae6320::Results::InvalidFile;
                eae6320::Assets::OutputErrorMessage("Each element in the vertexData table must be a table (instead of a %s)", luaL_typename(&io_luaState, -1));
                goto OnExit;
            }

            lua_pop(&io_luaState, 1);
        }

    OnExit:

        return result;
    }

    eae6320::cResult LoadVertexData(lua_State& io_luaState, uint16_t& o_vertexCount, eae6320::Graphics::VertexFormats::sMesh*& o_vertexData)
    {
        auto result = eae6320::Results::Success;

        // Right now the asset table is at -1.
        // After the following table operation it will be at -2
        // and the vertexData table will be at -1:
        constexpr auto* const key = "vertexData";
        lua_pushstring(&io_luaState, key);
        lua_gettable(&io_luaState, -2);

        // Right now the asset table is at -2,
        // and the vertexData table is at -1.
        if (lua_istable(&io_luaState, -1))
        {
            if (!(result = LoadVertexDataArray(io_luaState, o_vertexCount, o_vertexData)))
            {
                goto OnExit;
            }
        }
        else
        {
            result = eae6320::Results::InvalidFile;
            eae6320::Assets::OutputErrorMessage("The vertexData must be a table (instead of a %s)", luaL_typename(&io_luaState, -1));
            goto OnExit;
        }

    OnExit:

        // Right now the asset table is at -2,
        // and the vertexData table is at -1
        // Pop the vertexData table
        lua_pop(&io_luaState, 1);

        return result;
    }

    eae6320::cResult LoadIndexDataArray(lua_State& io_luaState, uint16_t& o_indexCount, uint16_t*& o_indices)
    {
        auto result = eae6320::Results::Success;

        // Right now the asset table is at -2,
        // and the indexData table is at -1.
        const auto indexDataArraySize = luaL_len(&io_luaState, -1);
        if (indexDataArraySize % 3 != 0)
        {
            result = eae6320::Results::InvalidFile;
            eae6320::Assets::OutputErrorMessage("The number of elements in an indexData table must be a multiple of 3 (instead of %d)", indexDataArraySize);
            goto OnExit;
        }

        o_indexCount = static_cast<uint16_t>(indexDataArraySize);
        // reserve memory for the indices
        {
            o_indices = static_cast<uint16_t*>(malloc(o_indexCount * sizeof(uint16_t)));
        }

        if (!(result = LoadIntegerArray(io_luaState, o_indexCount, o_indices)))
        {
            result = eae6320::Results::InvalidFile;
            eae6320::Assets::OutputErrorMessage("Couldn't load the indexData");
            goto OnExit;
        }

    OnExit:

        return result;
    }

    eae6320::cResult LoadIndexData(lua_State& io_luaState, uint16_t& o_indexCount, uint16_t*& o_indices)
    {
        auto result = eae6320::Results::Success;

        // Right now the asset table is at -1.
        // After the following table operation it will be at -2
        // and the indexData table will be at -1:
        constexpr auto* const key = "indexData";
        lua_pushstring(&io_luaState, key);
        lua_gettable(&io_luaState, -2);

        // Right now the asset table is at -2,
        // and the indexData table is at -1.
        if (lua_istable(&io_luaState, -1))
        {
            if (!(result = LoadIndexDataArray(io_luaState, o_indexCount, o_indices)))
            {
                goto OnExit;
            }
        }
        else
        {
            result = eae6320::Results::InvalidFile;
            eae6320::Assets::OutputErrorMessage("The indexData must be a table (instead of a %s)", luaL_typename(&io_luaState, -1));
            goto OnExit;
        }

    OnExit:

        // Right now the asset table is at -2,
        // and the indexData table is at -1
        // Pop the indexData table
        lua_pop(&io_luaState, 1);

        return result;
    }

    eae6320::cResult WriteBuiltMesh(const char* i_path, const uint16_t i_vertexCount, const eae6320::Graphics::VertexFormats::sMesh* i_vertexData, const uint16_t i_indexCount, const uint16_t* i_indices)
    {
        auto result = eae6320::Results::Success;

        std::ofstream outFile(i_path, std::ofstream::binary);
        const char* buffer = nullptr;

        buffer = reinterpret_cast<const char*>(&i_vertexCount);
        outFile.write(buffer, sizeof(uint16_t));

        buffer = reinterpret_cast<const char*>(i_vertexData);
        outFile.write(buffer, i_vertexCount * sizeof(eae6320::Graphics::VertexFormats::sMesh));

        buffer = reinterpret_cast<const char*>(&i_indexCount);
        outFile.write(buffer, sizeof(uint16_t));

        buffer = reinterpret_cast<const char*>(i_indices);
        outFile.write(buffer, i_indexCount * sizeof(uint16_t));
        
        outFile.close();

        return result;
    }
}
