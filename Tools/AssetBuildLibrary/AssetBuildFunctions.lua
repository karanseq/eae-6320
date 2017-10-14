--[[
    This file contains the logic for building assets
]]

-- Static Data Initialization
--===========================

-- Environment Variables
local EngineSourceContentDir, GameInstallDir, GameLicenseDir, GameSourceContentDir, LicenseDir, OutputDir 
do
    -- EngineSourceContentDir
    do
        local errorMessage
        EngineSourceContentDir, errorMessage = GetEnvironmentVariable( "EngineSourceContentDir" )
        if not EngineSourceContentDir then
            error( errorMessage )
        end
    end
    -- GameInstallDir
    do
        local errorMessage
        GameInstallDir, errorMessage = GetEnvironmentVariable( "GameInstallDir" )
        if not GameInstallDir then
            error( errorMessage )
        end
    end
    -- GameLicenseDir
    do
        local errorMessage
        GameLicenseDir, errorMessage = GetEnvironmentVariable( "GameLicenseDir" )
        if not GameLicenseDir then
            error( errorMessage )
        end
    end
    -- GameSourceContentDir
    do
        local errorMessage
        GameSourceContentDir, errorMessage = GetEnvironmentVariable( "GameSourceContentDir" )
        if not GameSourceContentDir then
            error( errorMessage )
        end
    end
    -- LicenseDir
    do
        local errorMessage
        LicenseDir, errorMessage = GetEnvironmentVariable( "LicenseDir" )
        if not LicenseDir then
            error( errorMessage )
        end
    end
    -- OutputDir
    do
        local errorMessage
        OutputDir, errorMessage = GetEnvironmentVariable( "OutputDir" )
        if not OutputDir then
            error( errorMessage )
        end
    end
end

-- Shaders
local VertexShaderDir = "Shaders/Vertex/"
local FragmentShaderDir = "Shaders/Fragment/"
local VertexShaderName = "sprite"
local BasicFragmentShaderName = "spriteBasic"
local AnimatedFragmentShaderName = "spriteAnimated"
local VertexInputLayoutShaderName = "vertexInputLayout_sprite"

-- Textures
local DustTextureDir = "Textures/Dust/"
local ForestTextureDir = "Textures/Forest/"
local RampsTextureDir = "Textures/Ramps/"
local FrameTexureSuffix = "frame_"
local NumberFrames = 6

-- External Interface
--===================

function BuildAssets()
    local wereThereErrors = false

    -- Build the shaders and copy them to the installation location
    do
        wereThereErrors = BuildShader(VertexShaderDir .. VertexShaderName, "vertex") or wereThereErrors
        wereThereErrors = BuildShader(FragmentShaderDir .. BasicFragmentShaderName, "fragment") or wereThereErrors
        wereThereErrors = BuildShader(FragmentShaderDir .. AnimatedFragmentShaderName, "fragment") or wereThereErrors

        if EAE6320_PLATFORM_D3D then
            wereThereErrors = BuildShader(VertexShaderDir .. VertexInputLayoutShaderName, "vertex") or wereThereErrors
        end
    end

    -- Build the textures and copy them to the installation location
    do
        local textureDirs = { DustTextureDir, ForestTextureDir, RampsTextureDir }

        for i, v in pairs(textureDirs) do
            for j = 0, NumberFrames-1 do
                wereThereErrors = BuildTexture(v .. FrameTexureSuffix .. j, ".png") or wereThereErrors
            end
        end
    end

    -- Copy the licenses to the installation location
    do
        CreateDirectoryIfItDoesntExist( GameLicenseDir )
        local sourceLicenses = GetFilesInDirectory( LicenseDir )
        for i, sourceLicense in ipairs( sourceLicenses ) do
            local sourceFileName = sourceLicense:sub( #LicenseDir + 1 )
            local targetPath = GameLicenseDir .. sourceFileName
            local result, errorMessage = CopyFile( sourceLicense, targetPath )
            if result then
                -- Display a message
                print( "Installed " .. sourceFileName )
            else
                wereThereErrors = true
                OutputErrorMessage( "The license \"" .. sourceLicense .. "\" couldn't be copied to \"" .. targetPath .. "\": " .. errorMessage )
            end
        end
    end

    -- Copy the game settings to the installation location
    do
        local settingsFileName = "settings.ini"
        local sourcePath = OutputDir .. settingsFileName
        local targetPath = GameInstallDir .. settingsFileName
        local result, errorMessage = CopyFile( sourcePath, targetPath )
        if result then
            -- Display a message
            print( "Copied " .. settingsFileName )
        else
            wereThereErrors = true
            OutputErrorMessage( "The settings file \"" .. settingsFileName .. "\" couldn't be copied to \"" .. targetPath .. "\": " .. errorMessage )
        end
    end

    return not wereThereErrors
end

function BuildShader(shaderName, shaderType)
    local wereThereErrors = false

    local path_shaderBuilder = OutputDir .. "ShaderBuilder.exe"
    do
        local shader_authored = EngineSourceContentDir .. shaderName .. ".xlsl"
        local shader_built = GameInstallDir .. "data/" .. shaderName .. ".shd"
        CreateDirectoryIfItDoesntExist( shader_built )
        local command = "\"" .. path_shaderBuilder .. "\""
            .. " \"" .. shader_authored .. "\" \"" .. shader_built .. "\" " .. shaderType
        local result, exitCode = ExecuteCommand( command )
        if result then
            if exitCode == 0 then
                -- Display a message for each asset
                print( "Built " .. shader_authored )
            else
                wereThereErrors = true
                -- The builder should already output a descriptive error message if there was an error
                -- (remember that you write the builder code,
                -- and so if the build process failed it means that _your_ code has returned an error code)
                -- but it can be helpful to still return an additional vague error message here
                -- in case there is a bug in the specific builder that doesn't output an error message
                OutputErrorMessage( "The command " .. command .. " failed with exit code " .. tostring( exitCode ), shader_authored )
            end
        else
            wereThereErrors = true
            -- If the command wasn't executed then the second return value is an error message
            OutputErrorMessage( "The command " .. command .. " couldn't be executed: " .. tostring( exitCode ), shader_authored )
        end
    end

    return wereThereErrors
end

function BuildTexture(textureName, textureExtension)
    local wereThereErrors = false

    local path_textureBuilder = OutputDir .. "TextureBuilder.exe"
    do
        local texture_authored = GameSourceContentDir .. textureName .. textureExtension
        local texture_built = GameInstallDir .. "data/" .. textureName .. ".tex"
        CreateDirectoryIfItDoesntExist( texture_built )
        local command = "\"" .. path_textureBuilder .. "\""
            .. " \"" .. texture_authored .. "\" \"" .. texture_built
        local result, exitCode = ExecuteCommand( command )
        if result then
            if exitCode == 0 then
                -- Disiplay a message for each asset
                print( "Built " .. texture_authored )
            else
                wereThereErrors = true
                -- The builder should already output a descriptive error message if there was an error
                -- (remember that you write the builder code,
                -- and so if the build process failed it means that _your_ code has returned an error code)
                -- but it can be helpful to still return an additional vague error message here
                -- in case there is a bug in the specific builder that doesn't output an error message
                OutputErrorMessage( "The command " .. command .. " failed with exit code " .. tostring( exitCode ), texture_authored )
            end
        else
            wereThereErrors = true
            -- If the command wasn't executed then the second return value is an error message
                OutputErrorMessage( "The command " .. command .. " couldn't be executed: " .. tostring( exitCode ), texture_authored )
        end
    end

    return wereThereErrors
end
