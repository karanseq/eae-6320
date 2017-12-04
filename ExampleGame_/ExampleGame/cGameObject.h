/*
    TODO
*/

#ifndef EAE6320_GAMEOBJECT_H
#define EAE6320_GAMEOBJECT_H

// Include Files
//==============

#include <Engine/Graphics/cMesh.h>
#include <Engine/Graphics/cTexture.h>
#include <Engine/Physics/sRigidBodyState.h>
#include <Engine/Results/Results.h>

// Forward Declarations
//=====================

namespace eae6320
{
    namespace Graphics
    {
        class cEffect;
        struct sColor;
    }
    namespace Math
    {
        struct sVector;
    }
}

// Class Declaration
//==================

namespace eae6320
{
    struct sGameObjectinitializationParameters
    {
        const std::string*                  vertexShaderFilePath = nullptr;
        const std::string*                  fragmentShaderFilePath = nullptr;
        const std::string*                  meshFilePath = nullptr;
        const std::string*                  textureFilePath = nullptr;
        Math::sVector                       initialPosition = Math::sVector(0.0f, 0.0f, 0.0f);
        float                               maxVelocity = 1.5f;

        FORCEINLINE bool IsValid() const { return !(vertexShaderFilePath == nullptr || 
            fragmentShaderFilePath == nullptr ||
            meshFilePath == nullptr ||
            textureFilePath == nullptr); }
    };

    class cGameObject
    {

        // Interface
        //==========

    public:

        // Initialization / Clean Up
        //--------------------------

        static cResult Create(cGameObject*& o_gameObject, const sGameObjectinitializationParameters& i_initializationParameters);
        static cResult Destroy(cGameObject*& i_gameObject);

        // Behavior
        //---------

        void AddImpulse(const Math::sVector& i_impulse);

        // Update
        //-------

        void UpdateBasedOnTime(const float i_elapsedSecondCount_sinceLastUpdate);

        // Render
        //-------

        void SubmitDataToBeRendered(const float i_elapsedSecondCount_systemTime, const float i_elapsedSecondCount_sinceLastSimulationUpdate);

    private:

        cResult Initialize(const sGameObjectinitializationParameters& i_initializationParameters);
        cResult CleanUp();

        cGameObject() = default;
        ~cGameObject();

        // Data
        //=====

    public:
        static const float                  s_linearDamping;

    private:
        Graphics::cEffect*                  m_effect = nullptr;
        Graphics::cMesh::Handle             m_mesh;
        Graphics::cTexture::Handle          m_texture;
        Physics::sRigidBodyState            m_rigidBodyState;
        float                               m_maxVelocityLengthSquared;

    }; // class cGameObject

} // namespace eae6320

#endif // EAE6320_GAMEOBJECT_H
