/*
    TODO
*/

#ifndef EAE6320_GAMEOBJECT_H
#define EAE6320_GAMEOBJECT_H

// Include Files
//==============

#include <Engine/Graphics/cMesh.h>
#include <Engine/Graphics/cTexture.h>
#include <Engine/Math/Constants.h>
#include <Engine/Math/sVector2d.h>
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
    class cGameObject;
    struct sGameObjectinitializationParameters
    {
        const std::string*                  vertexShaderFilePath = nullptr;
        const std::string*                  fragmentShaderFilePath = nullptr;
        const std::string*                  meshFilePath = nullptr;
        const std::string*                  textureFilePath = nullptr;
        Math::sVector                       initialPosition = Math::sVector(0.0f, 0.0f, 0.0f);
        float                               maxVelocity = 1.5f;
        float                               angularSpeed = Math::Pi * 0.1f;
        float                               linearDamping = 0.1f;
        float                               angularDamping = Math::Pi * 0.05f;

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
        void AddYaw(float i_delta);
        void AddPitch(float i_delta);


        // Update
        //-------

        void UpdateBasedOnTime(const float i_elapsedSecondCount_sinceLastUpdate);

        // Render
        //-------

        void SubmitDataToBeRendered(const float i_elapsedSecondCount_systemTime, const float i_elapsedSecondCount_sinceLastSimulationUpdate);

        // Data
        //=====

        FORCEINLINE const Physics::sRigidBodyState& GetRigidBodyState() const { return m_rigidBodyState; }

        FORCEINLINE const float GetMaxVelocityLengthSquared() const { return m_maxVelocityLengthSquared; }
        FORCEINLINE void SetMaxVelocityLengthSquared(const float i_maxVelocityLengthSquared) { m_maxVelocityLengthSquared = i_maxVelocityLengthSquared; }

        FORCEINLINE const float GetAngularSpeed() const { return m_angularSpeed; }
        FORCEINLINE void SetAngularSpeed(const float i_angularSpeed) { m_angularSpeed = i_angularSpeed; }

        FORCEINLINE const float GetLinearDamping() const { return m_linearDamping; }
        FORCEINLINE void SetLinearDamping(const float i_linearDamping) { m_linearDamping = i_linearDamping; }

        FORCEINLINE const float GetAngularDamping() const { return m_angularDamping; }
        FORCEINLINE void SetAngularDamping(const float i_angularDamping) { m_angularDamping = i_angularDamping; }

    private:

        cResult Initialize(const sGameObjectinitializationParameters& i_initializationParameters);
        cResult CleanUp();

        cGameObject() = default;
        ~cGameObject();

        // Data
        //=====

    public:
        static const float                  s_linearDamping;
        static const float                  s_angularDamping;

    private:
        Graphics::cEffect*                  m_effect = nullptr;
        Graphics::cMesh::Handle             m_mesh;
        Graphics::cTexture::Handle          m_texture;
        Physics::sRigidBodyState            m_rigidBodyState;
        Math::sVector2d                     m_angularImpulseReceived;
        float                               m_maxVelocityLengthSquared;
        float                               m_angularSpeed;
        float                               m_linearDamping;
        float                               m_angularDamping;

    }; // class cGameObject

} // namespace eae6320

#endif // EAE6320_GAMEOBJECT_H
