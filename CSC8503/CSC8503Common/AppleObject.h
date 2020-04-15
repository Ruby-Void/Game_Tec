#pragma once
#include "../../Plugins/OpenGLRendering/OGLMesh.h"
#include "../../Plugins/OpenGLRendering/OGLShader.h"
#include "../../Plugins/OpenGLRendering/OGLTexture.h"
#include "../../Common/TextureLoader.h"

#include "GameObject.h"
#include "SphereVolume.h"
#include "ObjectNames.h"
#include "Layers.h"

namespace NCL {
	namespace CSC8503 {
		class AppleObject : public GameObject {
		public:
			AppleObject(const Vector3 position);
			virtual ~AppleObject();

			void UpdateGameObject(float dt) override;
			void OnCollisionBegin(GameObject* o) override;

		protected:
			float size = 1.0f, inverseMass = 1.0f, movementSpeed = 20.0f;

			OGLMesh* appleMesh = nullptr;
			OGLTexture* appleTex = nullptr;
			OGLShader* appleShader = nullptr;		

			Vector3 applePos;

			void InitialiseApple();
		};
	}
}
