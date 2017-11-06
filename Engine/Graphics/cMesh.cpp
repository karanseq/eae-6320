// Include Files
//==============

#include "cMesh.h"

#include "VertexFormats.h"

#include <Engine/Asserts/Asserts.h>
#include <Engine/Graphics/sColor.h>
#include <Engine/Logging/Logging.h>
#include <Engine/Math/sVector.h>
#include <External/Lua/Includes.h>

#include <new>

// Helper Function Declarations
//=============================

namespace
{
    eae6320::cResult LoadIntegerArray(lua_State& io_luaState, const uint16_t i_integerCount, uint16_t* o_integerArray);
    eae6320::cResult LoadFloatArray(lua_State& io_luaState, const uint16_t i_floatCount, float* o_floatArray);
    eae6320::cResult LoadPosition(lua_State& io_luaState, eae6320::Math::sVector& o_position);
    eae6320::cResult LoadColor(lua_State& io_luaState, eae6320::Graphics::sColor& o_color);
    eae6320::cResult LoadVertexDataArray(lua_State& io_luaState, uint16_t& o_vertexCount, eae6320::Math::sVector*& o_vertices, eae6320::Graphics::sColor*& o_colors);
    eae6320::cResult LoadVertexData(lua_State& io_luaState, uint16_t& o_vertexCount, eae6320::Math::sVector*& o_vertices, eae6320::Graphics::sColor*& o_colors);
    eae6320::cResult LoadIndexDataArray(lua_State& io_luaState, uint16_t& o_indexCount, uint16_t*& o_indices);
    eae6320::cResult LoadIndexData(lua_State& io_luaState, uint16_t& o_indexCount, uint16_t*& o_indices);
}

// Interface
//==========

// Initialization / Clean Up
//--------------------------

eae6320::cResult eae6320::Graphics::cMesh::Load(const char* const i_path, cMesh*& o_mesh)
{
    auto result = Results::Success;

    // Create a new Lua state
    lua_State* luaState = nullptr;
    {
        luaState = luaL_newstate();
        if (!luaState)
        {
            result = eae6320::Results::OutOfMemory;
            EAE6320_ASSERTF(false, "Failed to create a new Lua state while loading %s", i_path);
            Logging::OutputError("Failed to create a new Lua state while loading %s", i_path);
            goto OnExit;
        }
    }

    // Load the asset file as a "chunk",
    // meaning there will be a callable function at the top of the stack
    const auto stackTopBeforeLoad = lua_gettop(luaState);
    {
        const auto luaResult = luaL_loadfile(luaState, i_path);
        if (luaResult != LUA_OK)
        {
            result = eae6320::Results::Failure;
            EAE6320_ASSERTF(false, "%s while loading %s", lua_tostring(luaState, -1), i_path);
            Logging::OutputError("%s while loading %s", lua_tostring(luaState, -1), i_path);
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
                    EAE6320_ASSERTF(false, "Asset files must return a table (instead of a %s)", luaL_typename(luaState, -1));
                    Logging::OutputError("Asset files must return a table (instead of a %s)", luaL_typename(luaState, -1));
                    // Pop the returned non-table value
                    lua_pop(luaState, 1);
                    goto OnExit;
                }
            }
            else
            {
                result = eae6320::Results::InvalidFile;
                EAE6320_ASSERTF(false, "Asset files must return a single table (instead of %d values)", returnedValueCount);
                Logging::OutputError("Asset files must return a single table (instead of %d values)", returnedValueCount);
                // Pop every value that was returned
                lua_pop(luaState, returnedValueCount);
                goto OnExit;
            }
        }
        else
        {
            result = eae6320::Results::InvalidFile;
            EAE6320_ASSERTF(false, lua_tostring(luaState, -1));
            Logging::OutputError(lua_tostring(luaState, -1));
            // Pop the error message
            lua_pop(luaState, 1);
            goto OnExit;
        }
    }

    // If this code is reached the asset file was loaded successfully,
    // and its table is now at index -1
    uint16_t vertexCount = 0;
    eae6320::Math::sVector* vertices = nullptr;
    eae6320::Graphics::sColor* colors = nullptr;

    if (!(result = LoadVertexData(*luaState, vertexCount, vertices, colors)))
    {
        result = Results::Failure;
        EAE6320_ASSERTF(false, "Couldn't parse vertex data for file %s", i_path);
        Logging::OutputError("Couldn't parse vertex data for file %s", i_path);
        goto OnExit;
    }

    uint16_t indexCount = 0;
    uint16_t* indices = nullptr;

    if (!(result = LoadIndexData(*luaState, indexCount, indices)))
    {
        result = Results::Failure;
        EAE6320_ASSERTF(false, "Couldn't parse index data for file %s", i_path);
        Logging::OutputError("Couldn't parse index data for file %s", i_path);
        goto OnExit;
    }

    // Pop the table
    lua_pop(luaState, 1);

    cMesh* newMesh = nullptr;

    // Allocate a new mesh
    {
        newMesh = new (std::nothrow) cMesh();
        if (newMesh == nullptr)
        {
            result = Results::OutOfMemory;
            EAE6320_ASSERTF(false, "Couldn't allocate memory for the new mesh!");
            Logging::OutputError("Failed to allocated memory for the new mesh!");
            goto OnExit;
        }
    }

    // Initialize the new mesh's geometry
    if (!(result = newMesh->Initialize(vertexCount, vertices, colors, indexCount, indices)))
    {
        EAE6320_ASSERTF(false, "Could not initialize the new mesh!");
        goto OnExit;
    }

    // Deallocate the vertices, colors and indices
    {
        if (vertices)
        {
            free(vertices);
            vertices = nullptr;
        }

        if (colors)
        {
            free(colors);
            colors = nullptr;
        }

        if (indices)
        {
            free(indices);
            indices = nullptr;
        }
    }

OnExit:

    if (luaState)
    {
        // If I haven't made any mistakes
        // there shouldn't be anything on the stack,
        // regardless of any errors encountered while loading the file:
        EAE6320_ASSERT(lua_gettop(luaState) == 0);

        lua_close(luaState);
        luaState = nullptr;
    }

    if (result)
    {
        EAE6320_ASSERT(newMesh);
        o_mesh = newMesh;
    }
    else
    {
        if (newMesh)
        {
            newMesh->DecrementReferenceCount();
            newMesh = nullptr;
        }
        o_mesh = nullptr;
    }

    return result;
}

eae6320::cResult eae6320::Graphics::cMesh::Create(cMesh*& o_mesh, const uint16_t i_vertexCount, const eae6320::Math::sVector* i_vertices, const eae6320::Graphics::sColor* i_colors, const uint16_t i_indexCount, const uint16_t* i_indices)
{
    auto result = Results::Success;

    // Validate inputs
    {
        if (i_vertexCount < 3)
        {
            result = Results::Failure;
            EAE6320_ASSERTF(false, "The vertex count for a mesh must be at least 3!");
            Logging::OutputError("The vertex count for a mesh must be at least of 3!");
            goto OnExit;
        }
        else if (i_vertices == nullptr)
        {
            result = Results::Failure;
            EAE6320_ASSERTF(false, "Invalid vertex data!");
            Logging::OutputError("Invalid vertex data!");
            goto OnExit;
        }
        else if (i_colors == nullptr)
        {
            result = Results::Failure;
            EAE6320_ASSERTF(false, "Invalid color data!");
            Logging::OutputError("Invalid color data!");
            goto OnExit;
        }
        else if (i_indexCount % 3 != 0)
        {
            result = Results::Failure;
            EAE6320_ASSERTF(false, "The index count for a mesh must be a multiple of 3!");
            Logging::OutputError("The index count for a mesh must be a multiple of 3!");
            goto OnExit;
        }
        else if (i_indices == nullptr)
        {
            result = Results::Failure;
            EAE6320_ASSERTF(false, "Invalid index data!");
            Logging::OutputError("Invalid index data!");
            goto OnExit;
        }
    }

    cMesh* newMesh = nullptr;

    // Allocate a new mesh
    {
        newMesh = new (std::nothrow) cMesh();
        if (newMesh == nullptr)
        {
            result = Results::OutOfMemory;
            EAE6320_ASSERTF(false, "Couldn't allocate memory for the new mesh!");
            Logging::OutputError("Failed to allocated memory for the new mesh!");
            goto OnExit;
        }
    }

    // Initialize the new mesh's geometry
    if (!(result = newMesh->Initialize(i_vertexCount, i_vertices, i_colors, i_indexCount, i_indices)))
    {
        EAE6320_ASSERTF(false, "Could not initialize the new mesh!");
        goto OnExit;
    }

OnExit:

    if (result)
    {
        EAE6320_ASSERT(newMesh);
        o_mesh = newMesh;
    }
    else
    {
        if (newMesh)
        {
            newMesh->DecrementReferenceCount();
            newMesh = nullptr;
        }
        o_mesh = nullptr;
    }

    return result;
}

eae6320::Graphics::cMesh::~cMesh()
{
    CleanUp();
}

// Implementation
//===============

void eae6320::Graphics::cMesh::GetVertexBufferData(VertexFormats::sMesh* o_vertexData, const uint16_t i_vertexCount, const eae6320::Math::sVector* i_vertices, const eae6320::Graphics::sColor* i_colors) const
{
    EAE6320_ASSERT(o_vertexData != nullptr && i_vertices != nullptr && i_colors != nullptr && i_vertexCount > 0);

    for (uint16_t i = 0; i < i_vertexCount; ++i)
    {
        o_vertexData[i].x = i_vertices[i].x;
        o_vertexData[i].y = i_vertices[i].y;
        o_vertexData[i].z = i_vertices[i].z;
        o_vertexData[i].r = static_cast<uint8_t>(i_colors[i].r * 255.0f);
        o_vertexData[i].g = static_cast<uint8_t>(i_colors[i].g * 255.0f);
        o_vertexData[i].b = static_cast<uint8_t>(i_colors[i].b * 255.0f);
        o_vertexData[i].a = static_cast<uint8_t>(i_colors[i].a * 255.0f);
    }
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
            EAE6320_ASSERTF(false, "Found %d elements in the integer array(instead of %d)", intArraySize, i_integerCount);
            eae6320::Logging::OutputError("Found %d elements in the integer array(instead of %d)", intArraySize, i_integerCount);
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

                EAE6320_ASSERTF(false, "Each element in an int array must be a number (instead of a %s)", luaL_typename(&io_luaState, -1));
                eae6320::Logging::OutputError("Each element in an int array must be a number (instead of a %s)", luaL_typename(&io_luaState, -1));
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
            EAE6320_ASSERTF(false, "Found %d elements in the float array(instead of %d)", floatArraySize, i_floatCount);
            eae6320::Logging::OutputError("Found %d elements in the float array(instead of %d)", floatArraySize, i_floatCount);
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

                EAE6320_ASSERTF(false, "Each element in a float array must be a number (instead of a %s)", luaL_typename(&io_luaState, -1));
                eae6320::Logging::OutputError("Each element in a float array must be a number (instead of a %s)", luaL_typename(&io_luaState, -1));
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
                EAE6320_ASSERTF(false, "Couldn't load the floats within a position");
                eae6320::Logging::OutputError("Couldn't load the floats within a position");
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
            EAE6320_ASSERTF(false, "The position value must be a table (instead of a %s)", luaL_typename(&io_luaState, -1));
            eae6320::Logging::OutputError("The position value must be a table (instead of a %s)", luaL_typename(&io_luaState, -1));
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
        // and the position table will be at -1
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
                EAE6320_ASSERTF(false, "Couldn't load the floats within a color");
                eae6320::Logging::OutputError("Couldn't load the floats within a color");
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
            EAE6320_ASSERTF(false, "The color value must be a table (instead of a %s)", luaL_typename(&io_luaState, -1));
            eae6320::Logging::OutputError("The color value must be a table (instead of a %s)", luaL_typename(&io_luaState, -1));
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

    eae6320::cResult LoadVertexDataArray(lua_State& io_luaState, uint16_t& o_vertexCount, eae6320::Math::sVector*& o_vertices, eae6320::Graphics::sColor*& o_colors)
    {
        auto result = eae6320::Results::Success;

        // Right now the asset table is at -2,
        // and the vertexData table is at -1.
        const auto vertexDataArraySize = luaL_len(&io_luaState, -1);
        if (vertexDataArraySize < 3)
        {
            result = eae6320::Results::InvalidFile;
            EAE6320_ASSERTF(false, "The vertexData must have at least three elements");
            eae6320::Logging::OutputError("The vertexData must have at least three elements");
            goto OnExit;
        }

        o_vertexCount = static_cast<uint16_t>(vertexDataArraySize);
        // reserve memory for the vertices & colors
        {
            o_vertices = static_cast<eae6320::Math::sVector*>(malloc(o_vertexCount * sizeof(eae6320::Math::sVector)));
            o_colors = static_cast<eae6320::Graphics::sColor*>(malloc(o_vertexCount * sizeof(eae6320::Graphics::sColor)));
        }

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
                if (!(result = LoadPosition(io_luaState, o_vertices[i - 1])))
                {
                    // Right now the asset table is at -3,
                    // the vertexData table is at -2,
                    // and i'th element of the vertexData table is at -1
                    lua_pop(&io_luaState, 1);

                    result = eae6320::Results::InvalidFile;
                    EAE6320_ASSERTF(false, "Couldn't load the position for %d in the vertexTable", i);
                    eae6320::Logging::OutputError("Couldn't load the position for %d in the vertexTable", i);
                    goto OnExit;
                }

                if (!(result = LoadColor(io_luaState, o_colors[i - 1])))
                {
                    // Right now the asset table is at -3,
                    // the vertexData table is at -2,
                    // and i'th element of the vertexData table is at -1
                    lua_pop(&io_luaState, 1);

                    result = eae6320::Results::InvalidFile;
                    EAE6320_ASSERTF(false, "Couldn't load the color for %d in the vertexTable", i);
                    eae6320::Logging::OutputError("Couldn't load the color for %d in the vertexTable", i);
                    goto OnExit;
                }
            }
            else
            {
                // Right now the asset table is at -3,
                // the vertexData table is at -2,
                // and i'th element of the vertexData table is at -1
                lua_pop(&io_luaState, 1);

                result = eae6320::Results::InvalidFile;
                EAE6320_ASSERTF(false, "Each element in the vertexData table must be a table (instead of a %s)", luaL_typename(&io_luaState, -1));
                eae6320::Logging::OutputError("Each element in the vertexData table must be a table (instead of a %s)", luaL_typename(&io_luaState, -1));
                goto OnExit;
            }

            lua_pop(&io_luaState, 1);
        }

    OnExit:

        return result;
    }

    eae6320::cResult LoadVertexData(lua_State& io_luaState, uint16_t& o_vertexCount, eae6320::Math::sVector*& o_vertices, eae6320::Graphics::sColor*& o_colors)
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
            if (!(result = LoadVertexDataArray(io_luaState, o_vertexCount, o_vertices, o_colors)))
            {
                goto OnExit;
            }
        }
        else
        {
            result = eae6320::Results::InvalidFile;
            EAE6320_ASSERTF(false, "The vertexData must be a table (instead of a %s)", luaL_typename(&io_luaState, -1));
            eae6320::Logging::OutputError("The vertexData must be a table (instead of a %s)", luaL_typename(&io_luaState, -1));
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
            EAE6320_ASSERTF(false, "The number of elements in an indexData table must be a multiple of 3 (instead of %d)", indexDataArraySize);
            eae6320::Logging::OutputError("The number of elements in an indexData table must be a multiple of 3 (instead of %d)", indexDataArraySize);
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
            EAE6320_ASSERTF(false, "Couldn't load the indexData");
            eae6320::Logging::OutputError("Couldn't load the indexData");
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
            EAE6320_ASSERTF(false, "The indexData must be a table (instead of a %s)", luaL_typename(&io_luaState, -1));
            eae6320::Logging::OutputError("The indexData must be a table (instead of a %s)", luaL_typename(&io_luaState, -1));
            goto OnExit;
        }

    OnExit:

        // Right now the asset table is at -2,
        // and the indexData table is at -1
        // Pop the indexData table
        lua_pop(&io_luaState, 1);

        return result;
    }
}
