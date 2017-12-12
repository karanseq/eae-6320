--[[
    This file lists every asset that must be built by the AssetBuildSystem
]]

return
{
    meshes =
    {
        "Meshes/Ring.mesh",
        "Meshes/Ship.mesh",
        "Meshes/SkyBox.mesh"
    },
    shaders =
    {
        { path = "Shaders/Vertex/vertexInputLayout_mesh.xlsl", arguments = { "vertex" } },
        { path = "Shaders/Vertex/mesh.xlsl", arguments = { "vertex" } },
        { path = "Shaders/Fragment/mesh.xlsl", arguments = { "fragment" } }
    },
    textures =
    {
        "Textures/Ship.png",
        "Textures/SkyBox.png",
        "Textures/Ring.png"
    }
}
