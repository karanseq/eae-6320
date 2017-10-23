/*
    TODO
*/

#ifndef EAE6320_GAMEOBJECT_H
#define EAE6320_GAMEOBJECT_H

// Include Files
//==============

#include <Engine/Math/sVector.h>
#include <Engine/Results/Results.h>

// Forward Declarations
//=====================

namespace eae6320
{
    namespace Graphics
    {
        class cEffect;
        class cMesh;
        struct sColor;
    }
}

// Class Declaration
//==================

namespace eae6320
{
    class cGameObject
    {
        // Interface
        //==========

    public:

        // Initialization / Clean Up
        //--------------------------

        static cResult Create(cGameObject*& o_gameObject, const Math::sVector& i_position, const Graphics::sColor& i_innerColor, const Graphics::sColor& i_outerColor);
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

        cResult Initialize(const Math::sVector& i_position, const Graphics::sColor& i_innerColor, const Graphics::sColor& i_outerColor);
        cResult CleanUp();

        cGameObject() = default;
        ~cGameObject();

        // Data
        //=====

    public:
        static const float                  s_maxVelocity;
        static const float                  s_linearDamping;

    private:
        Graphics::cEffect*                  m_effect = nullptr;
        Graphics::cMesh*                    m_mesh = nullptr;
        Math::sVector                       m_position;
        Math::sVector                       m_velocity;

    }; // class cGameObject

} // namespace eae6320

#endif // EAE6320_GAMEOBJECT_H
