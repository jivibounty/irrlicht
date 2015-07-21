
#pragma once

#include <irrlicht.h>
#include <vector>

#ifdef _MSC_VER
#pragma comment(lib, "Irrlicht.lib")
#endif

#define NUM_CASCADES 4

namespace irr
{

	class CascadedShadowMaps : public irr::IReferenceCounted
	{
	public:
		CascadedShadowMaps(irr::video::IVideoDriver* videoDriver, irr::scene::ISceneManager* smgr, irr::scene::ICameraSceneNode* viewCamera);

		virtual ~CascadedShadowMaps();

		void setLightDirection(const irr::core::vector3df& dir);

		const irr::core::vector3df& getLightDirection() const { return m_LightDirection; }

		void setParallelSplitInterpolation(float fParallelSplitInterpolation);

		float getParallelSplitInterpolation() const { return m_fParallelSplitInterpolation; }

		void setMaximumViewDistance(float fMaximumViewDistance);

		float getMaximumViewDistance() const { return m_fMaximumViewDistance; }

		void setBehindCameraBias(float fBehindCameraBias);

		float getBehindCameraBias() const { return m_fBehindCameraBias; }

		void setShadowMapSize(unsigned int size);

		unsigned int getShadowMapSIze() const { return m_ShadowMapSize; }

		void setDirty(bool bIsDirty);

		bool isDirty() const { return m_bIsDirty; }

		void render();

		void initialize();
	protected:
		void update();

		float calcUniformSplitDistance(float splitRatio, float zNear, float zFar) const;

		float calcLogarithmicSplitDistance(float splitRatio, float zNear, float zFar) const;
	protected:
		irr::video::ITexture* m_pRgbTexture;

		irr::video::ITexture* m_ppDepthTextures[NUM_CASCADES];

		irr::scene::ICameraSceneNode* m_ppCameras[NUM_CASCADES];

		irr::core::vector3df m_LightDirection;

		unsigned int m_ShadowMapSize;

		float m_fParallelSplitInterpolation;

		float m_fMaximumViewDistance;

		float m_fBehindCameraBias;

		bool m_bIsDirty;

		irr::video::IVideoDriver* m_pVideoDriver;
	
		irr::scene::ISceneManager* m_pSceneManager;

		irr::scene::ICameraSceneNode* m_pViewCamera;

		struct ImportanceStruct
		{
			ImportanceStruct()
				: m_fImportance(0.0f)
				, m_fAge(0.0f)
				, m_SortedPosition(0)
			{}
			float m_fImportance;
			float m_fAge;
			unsigned int m_SortedPosition;
		};

		ImportanceStruct m_pImportance[NUM_CASCADES];

		unsigned int m_pSortHelper[NUM_CASCADES];

		bool m_pIsCulled[NUM_CASCADES];

		irr::core::matrix4 m_TexProjectionMatrix[NUM_CASCADES];

		unsigned int m_PassesOffset;

		unsigned int m_NumPasses;
	};

}
