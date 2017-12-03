--[[
    This file lists every asset that must be built by the AssetBuildSystem
]]

return
{
    meshes =
    {
        "Meshes/Bat.mesh",
        "Meshes/Crate.mesh",
        "Meshes/Floor.mesh"
    },
    shaders =
    {
        { path = "Shaders/Vertex/vertexInputLayout_mesh.xlsl", arguments = { "vertex" } },
        { path = "Shaders/Vertex/vertexInputLayout_sprite.xlsl", arguments = { "vertex" } },
        { path = "Shaders/Vertex/mesh.xlsl", arguments = { "vertex" } },
        { path = "Shaders/Vertex/sprite.xlsl", arguments = { "vertex" } },
        { path = "Shaders/Fragment/mesh.xlsl", arguments = { "fragment" } },
        { path = "Shaders/Fragment/meshTranslucent.xlsl", arguments = { "fragment" } },
        { path = "Shaders/Fragment/spriteAnimated.xlsl", arguments = { "fragment" } },
        { path = "Shaders/Fragment/spriteBasic.xlsl", arguments = { "fragment" } }
    },
    textures =
    {
        "Textures/Ramps/frame_0.png",
        "Textures/Ramps/frame_1.png",
        "Textures/Ramps/frame_2.png",
        "Textures/Ramps/frame_3.png",
        "Textures/Ramps/frame_4.png",
        "Textures/Ramps/frame_5.png",
        "Textures/Soccer/Crate.png",
        "Textures/Soccer/Grass.png",
        "Textures/Soccer/Wood.png"
    }
}
