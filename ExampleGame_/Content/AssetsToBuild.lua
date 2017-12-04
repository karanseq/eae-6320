--[[
    This file lists every asset that must be built by the AssetBuildSystem
]]

return
{
    meshes =
    {
        "Meshes/Bat.mesh",
        "Meshes/Coin.mesh",
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
        "Textures/Soccer/Wood.png"
    }
}
