
#include "CascadedShadowMaps.h"
#include "Plane.h"
#include "Box.h"
#include <math.h>
#include <string.h>

#define PI_RAD 0.01745329251994329576923690768489f

namespace irr
{

	CascadedShadowMaps::CascadedShadowMaps(irr::video::IVideoDriver* videoDriver, irr::scene::ISceneManager* smgr, irr::scene::ICameraSceneNode* viewCamera)
		: m_pRgbTexture(NULL)
		, m_ShadowMapSize(1024)
		, m_fParallelSplitInterpolation(0.92f)
		, m_fMaximumViewDistance(300.0f)
		, m_fBehindCameraBias(300.0f)
		, m_bIsDirty(true)
		, m_PassesOffset(4)
		, m_NumPasses(4)
	{
		for(unsigned int n = 0; n < NUM_CASCADES; ++n)
		{
			m_ppDepthTextures[n] = NULL;
			m_ppCameras[n] = NULL;
		}
		m_pVideoDriver = videoDriver;
		if(m_pVideoDriver)
		{
			m_pVideoDriver->grab();
		}
		m_pSceneManager = smgr;
		if(m_pSceneManager)
		{
			m_pSceneManager->grab();
			for(unsigned int n = 0; n < NUM_CASCADES; ++n)
			{
				m_ppCameras[n] = m_pSceneManager->addCameraSceneNode(0, core::vector3df(0,0,0), core::vector3df(1,0,0));;
			}
		}
		m_pViewCamera = viewCamera;
		if(m_pViewCamera)
		{
			m_pViewCamera->grab();
		}
	}

	CascadedShadowMaps::~CascadedShadowMaps()
	{
		if(m_pVideoDriver)
		{
			m_pVideoDriver->drop();
			m_pVideoDriver = NULL;
		}
		if(m_pSceneManager)
		{
			m_pSceneManager->drop();
			m_pSceneManager = NULL;
		}
		if(m_pViewCamera)
		{
			m_pViewCamera->drop();
			m_pViewCamera = NULL;
		}
	}

	void CascadedShadowMaps::setLightDirection(const irr::core::vector3df& dir)
	{
		m_LightDirection = dir;
	}

	void CascadedShadowMaps::setParallelSplitInterpolation(float fParallelSplitInterpolation)
	{
		m_fParallelSplitInterpolation = fParallelSplitInterpolation;
	}

	void CascadedShadowMaps::setMaximumViewDistance(float fMaximumViewDistance)
	{
		m_fMaximumViewDistance = fMaximumViewDistance;
	}

	void CascadedShadowMaps::setBehindCameraBias(float fBehindCameraBias)
	{
		m_fBehindCameraBias = fBehindCameraBias;
	}

	void CascadedShadowMaps::setShadowMapSize(unsigned int size)
	{
		m_ShadowMapSize = size;
		setDirty(true);
	}

	void CascadedShadowMaps::setDirty(bool bIsDirty)
	{
		m_bIsDirty = bIsDirty;
	}

	void CascadedShadowMaps::render()
	{
		if(!m_pVideoDriver && !m_pSceneManager)
		{
			return;
		}
		if(isDirty())
		{
			initialize();
		}
		if(m_pRgbTexture)
		{
			for(unsigned int cascadeIndex = 0; cascadeIndex < NUM_CASCADES; ++cascadeIndex)
			{
				if (m_ppDepthTextures[cascadeIndex])
				{
					// draw scene into render target

					// set render target texture
					m_pVideoDriver->setRenderTarget(m_pRgbTexture, m_ppDepthTextures[cascadeIndex], true, true, video::SColor(0,0,0,255));

					m_pSceneManager->setActiveCamera(m_ppCameras[cascadeIndex]);

					// draw whole scene into render buffer
					m_pSceneManager->drawSolid();

					// set back old render target
					// The buffer might have been distorted, so clear it
					m_pVideoDriver->setRenderTarget(0, true, true, 0);
				}
			}
		}
	}

	void CascadedShadowMaps::initialize()
	{
		if(!m_pVideoDriver)
		{
			return;
		}
		if(m_pRgbTexture)
		{
			m_pRgbTexture->drop();
		}
		m_pRgbTexture = m_pVideoDriver->addRenderTargetTexture(core::dimension2d<u32>(m_ShadowMapSize, m_ShadowMapSize), "RGB_RTT");
		for(unsigned int cascadeIndex = 0; cascadeIndex < NUM_CASCADES; ++cascadeIndex)
		{
			if (m_ppDepthTextures[cascadeIndex])
			{
				m_ppDepthTextures[cascadeIndex]->drop();
			}
			m_ppDepthTextures[cascadeIndex] = m_pVideoDriver->addRenderTargetTexture(core::dimension2d<u32>(m_ShadowMapSize, m_ShadowMapSize), "DEPTH_RTT", irr::video::ECF_D32);
		}
	}

	void CascadedShadowMaps::update()
	{
			if(!m_pViewCamera)
			{
				return;
			}
			for(unsigned int cascadeIndex = 0; cascadeIndex < NUM_CASCADES; ++cascadeIndex)
			{
				if (m_ppCameras[cascadeIndex])
				{
					return;
				}
			}
			const float ndBiasMultiplier = 0.1f;
			const float frustumDimensionBiasMultiplier = 1.05f;
			float nd = m_pViewCamera->getNearValue();
			nd -= (nd * ndBiasMultiplier);
			float fd = std::min(m_pViewCamera->getFarValue(), nd + m_fMaximumViewDistance);
			float fov = m_pViewCamera->getFOV();
			float aspect = m_pViewCamera->getAspectRatio();
			float tanfov = tan(0.5f * fov * PI_RAD);
			float halfnh = tanfov * nd * frustumDimensionBiasMultiplier;
			float halfnw = halfnh * aspect;
			float halffh = tanfov * fd* frustumDimensionBiasMultiplier;
			float halffw = halffh * aspect;
			irr::core::matrix4 camMatrix = m_pViewCamera->getViewMatrix();
			irr::core::vector3df camPos = m_pViewCamera->getAbsolutePosition();
			irr::core::vector3df camNorm = irr::core::vector3df(-camMatrix[2], -camMatrix[6], -camMatrix[10]);
			irr::core::vector3df camUp = irr::core::vector3df(camMatrix[1], camMatrix[5], camMatrix[9]);
			irr::core::vector3df camTangent = irr::core::vector3df(camMatrix[0], camMatrix[4], camMatrix[8]);
			
			irr::core::vector3df v[8];

			irr::core::vector3df nearPos = camPos + camNorm * nd;
			irr::core::vector3df nearUp = camUp * halfnh;
			irr::core::vector3df nearTangent = camTangent * halfnw;

			v[0] = nearPos + nearUp + nearTangent; 
			v[1] = nearPos - nearUp + nearTangent; 
			v[2] = nearPos + nearUp - nearTangent; 
			v[3] = nearPos - nearUp - nearTangent; 
			
			irr::core::vector3df farPos = camPos + camNorm * fd;
			irr::core::vector3df farUp = camUp * halffh;
			irr::core::vector3df farTangent = camTangent * halffw;

			v[4] = farPos + farUp + farTangent; 
			v[5] = farPos - farUp + farTangent; 
			v[6] = farPos + farUp - farTangent; 
			v[7] = farPos - farUp - farTangent; 
			
			irr::core::vector3df lightDirection = m_LightDirection;
			irr::core::vector3df Z = -lightDirection;
			irr::core::vector3df U = (abs(Z.Y) > 0.9f) ? irr::core::vector3df(0.0f, 0.0f, 1.0f) : irr::core::vector3df(0.0f, 1.0f, 0.0f);
			irr::core::vector3df sideVec = U.crossProduct(Z);
			irr::core::vector3df side = sideVec.normalize();
			U = Z.crossProduct(side);
			irr::core::vector3df upVector = U.normalize();
			
			irr::Plane xp(irr::core::vector3df(0.0f), side);
			irr::Plane yp(irr::core::vector3df(0.0f), upVector);
			irr::Plane zp(irr::core::vector3df(0.0f), lightDirection);

			irr::Box lightBox;
			lightBox.resetUpdateExtents();
			for(unsigned int n = 0; n < 8; ++n)
			{
				irr::core::vector3df dist;
				dist.X = xp.signedDistanceTo(v[n]);
				dist.Y = yp.signedDistanceTo(v[n]);
				dist.Z = zp.signedDistanceTo(v[n]);
				lightBox.updateExtents(dist);
			}
			irr::core::vector3df halfExtent = lightBox.getHalfExtent();
			float behindCameraBias = m_fBehindCameraBias;
			irr::core::vector3df lightPos = lightBox.m_Max;
			lightPos -= (lightDirection * behindCameraBias);

			xp.makePlane(lightPos, side);
			yp.makePlane(lightPos, upVector);
			zp.makePlane(lightPos, lightDirection);
			irr::core::matrix4 lightViewMatrix;
			lightViewMatrix.buildCameraLookAtMatrixLH(lightPos, lightPos + lightDirection, upVector);

			const irr::f32 biasMat[16] = {
				0.5, 0.0, 0.0, 0.0,
				0.0, 0.5, 0.0, 0.0,
				0.0, 0.0, 0.5, 0.0,
				0.5, 0.5, 0.5, 1.0
			};

			const irr::core::matrix4 biasMatrix;
			memcpy((irr::f32*)biasMatrix.pointer(), biasMat, sizeof(irr::f32) * 16);
			irr::core::vector3df vCascade[8];
			unsigned int numSplits = (unsigned int)NUM_CASCADES;
			unsigned int shadowMapSize = m_ShadowMapSize;
			float interp = m_fParallelSplitInterpolation;
			float oneMinusInterp = 1.0f - interp;
			float viewDistance = fd - nd;
			float inverseViewDistance = 1.0f / viewDistance;
			float farbias = viewDistance * 0.01f;
			for(unsigned int cascadeIndex = 0; cascadeIndex < numSplits; ++cascadeIndex)
			{
				m_pImportance[cascadeIndex].m_fAge += 1.0f;
				m_pImportance[cascadeIndex].m_fImportance = (float)(numSplits - cascadeIndex) * m_pImportance[cascadeIndex].m_fAge;
			}
			for(unsigned int cascadeIndex = 0; cascadeIndex < m_PassesOffset; ++cascadeIndex)
			{
				m_pImportance[cascadeIndex].m_SortedPosition = cascadeIndex;
			}
			for(unsigned int cascadeIndex = m_PassesOffset; cascadeIndex < numSplits; ++cascadeIndex)
			{
				m_pSortHelper[cascadeIndex] = cascadeIndex;
			}
			for(unsigned int cascadeIndex = m_PassesOffset; cascadeIndex < numSplits; ++cascadeIndex)
			{
				for(unsigned int cascadeIndexB = cascadeIndex + 1; cascadeIndexB < numSplits; ++cascadeIndexB)
				{
					if(m_pImportance[m_pSortHelper[cascadeIndexB]].m_fImportance >
						m_pImportance[m_pSortHelper[cascadeIndex]].m_fImportance)
					{
						unsigned int temp = m_pSortHelper[cascadeIndex];
						m_pSortHelper[cascadeIndex] = m_pSortHelper[cascadeIndexB];
						m_pSortHelper[cascadeIndexB] = temp;
					}
				}
			}
			for(unsigned int cascadeIndex = m_PassesOffset; cascadeIndex < numSplits; ++cascadeIndex)
			{
				m_pImportance[m_pSortHelper[cascadeIndex]].m_SortedPosition = cascadeIndex;
			}

			for(unsigned int cascadeIndex = 0; cascadeIndex < numSplits; ++cascadeIndex)
			{
				bool bIsCulled = true;
				if(m_pImportance[cascadeIndex].m_SortedPosition < m_NumPasses)
				{
					bIsCulled = false;
					m_pImportance[cascadeIndex].m_fAge = 0.0f;
				}
				m_pIsCulled[cascadeIndex] = bIsCulled;
				float splitNearRatio = (float)cascadeIndex / (float)numSplits;
				float splitFarRatio = (float)(cascadeIndex + 1)/ (float)numSplits;
				float zNear = 
					calcUniformSplitDistance(splitNearRatio, nd, fd) * oneMinusInterp
					+ calcLogarithmicSplitDistance(splitNearRatio, nd, fd) * interp;
				float zFar = 
					calcUniformSplitDistance(splitFarRatio, nd, fd) * oneMinusInterp
					+ calcLogarithmicSplitDistance(splitFarRatio, nd, fd) * interp + farbias;

				float interpNear = (zNear - nd) * inverseViewDistance;
				float oneMinusInterpNear = 1.0f - interpNear;
				for(unsigned int n = 0; n < 4; ++n)
				{
					vCascade[n] = v[n] * oneMinusInterpNear + v[n + 4] * interpNear;
				}
				float interpFar = (zFar - nd) * inverseViewDistance;
				float oneMinusInterpFar = 1.0f - interpFar;
				for(unsigned int n = 4; n < 8; ++n)
				{
					vCascade[n] = v[n - 4] * oneMinusInterpFar + v[n] * interpFar;
				}
				irr::Box lightCascadeBox;
				lightCascadeBox.resetUpdateExtents();
				for(unsigned int n = 0; n < 8; ++n)
				{
					irr::core::vector3df dist;
					dist.X = xp.signedDistanceTo(vCascade[n]);
					dist.Y = yp.signedDistanceTo(vCascade[n]);
					dist.Z = zp.signedDistanceTo(vCascade[n]);
					lightCascadeBox.updateExtents(dist);
				}
				irr::core::matrix4 lightProjMatrix;
				lightProjMatrix.buildProjectionMatrixOrthoLH(lightCascadeBox.m_Max.X - lightCascadeBox.m_Min.X, lightCascadeBox.m_Max.Y - lightCascadeBox.m_Min.Y, 0.0f, lightCascadeBox.m_Max.Z * 1.05f);

				irr::core::matrix4 splitModelViewProjMatrix = lightProjMatrix * lightViewMatrix;
				m_ppCameras[cascadeIndex]->setProjectionMatrix(lightProjMatrix, true);
				m_ppCameras[cascadeIndex]->setPosition(lightPos);
				m_ppCameras[cascadeIndex]->setTarget(lightPos + lightDirection);
				m_ppCameras[cascadeIndex]->setUpVector(upVector);
				m_TexProjectionMatrix[cascadeIndex] = biasMatrix * splitModelViewProjMatrix;
			}
		}

		float CascadedShadowMaps::calcUniformSplitDistance(float splitRatio, float zNear, float zFar) const
		{
			return zNear + (zFar - zNear) * splitRatio;
		}

		float CascadedShadowMaps::calcLogarithmicSplitDistance(float splitRatio, float zNear, float zFar) const
		{
			return zNear * powf((zFar / zNear), splitRatio);
		}

}
