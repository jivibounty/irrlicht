// Copyright (C) 2002-2009 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h
// Code for this scene node has been contributed by Anders la Cour-Harbo (alc)

#include "CSkyDomeSceneNode.h"
#include "IVideoDriver.h"
#include "ISceneManager.h"
#include "ICameraSceneNode.h"
#include "IAnimatedMesh.h"
#include "os.h"

namespace irr
{
namespace scene
{

/* horiRes and vertRes:
	Controls the number of faces along the horizontal axis (30 is a good value)
	and the number of faces along the vertical axis (8 is a good value).

	texturePercentage:
	Only the top texturePercentage of the image is used, e.g. 0.8 uses the top 80% of the image,
	1.0 uses the entire image. This is useful as some landscape images have a small banner
	at the bottom that you don't want.

	spherePercentage:
	This controls how far around the sphere the sky dome goes. For value 1.0 you get exactly the upper
	hemisphere, for 1.1 you get slightly more, and for 2.0 you get a full sphere. It is sometimes useful
	to use a value slightly bigger than 1 to avoid a gap between some ground place and the sky. This
	parameters stretches the image to fit the chosen "sphere-size". */

CSkyDomeSceneNode::CSkyDomeSceneNode(video::ITexture* sky, u32 horiRes, u32 vertRes,
			f32 texturePercentage, f32 spherePercentage, f32 radius,
			ISceneNode* parent, ISceneManager* mgr, s32 id)
			: ISceneNode(parent, mgr, id), Buffer(0)
{
	#ifdef _DEBUG
	setDebugName("CSkyDomeSceneNode");
	#endif

	f32 azimuth, azimuth_step;
	f32 elevation, elevation_step;
	u32 k;

	video::S3DVertex vtx;

	setAutomaticCulling ( scene::EAC_OFF );

	Buffer = new SMeshBuffer();
	Buffer->Material.Lighting = false;
	Buffer->Material.ZBuffer = video::ECFN_NEVER;
	Buffer->Material.ZWriteEnable = false;
	Buffer->Material.setTexture(0, sky);
	Buffer->BoundingBox.MaxEdge.set(0,0,0);
	Buffer->BoundingBox.MinEdge.set(0,0,0);

	azimuth_step = (core::PI * 2.f ) /horiRes;
	if (spherePercentage<0.)
		spherePercentage=-spherePercentage;
	if (spherePercentage>2.)
		spherePercentage=2.;
	elevation_step = spherePercentage*core::HALF_PI/ (f32) vertRes;

	Buffer->Vertices.reallocate((horiRes+1)*(vertRes+1));
	Buffer->Indices.reallocate(3*(2*vertRes-1)*horiRes);

	vtx.Color.set(255,255,255,255);
	vtx.Normal.set(0.0f,-1.f,0.0f);

	const f32 tcV = texturePercentage / vertRes;
	for (k = 0, azimuth = 0; k <= horiRes; ++k)
	{
		elevation = core::HALF_PI;
		const f32 tcU = (f32) k / (f32) horiRes;
		const f32 sinA = sinf(azimuth);
		const f32 cosA = cosf(azimuth);
		for (u32 j = 0; j <= vertRes; ++j)
		{
			const f32 cosEr = radius*cosf(elevation);
			vtx.Pos.set( cosEr*sinA,radius*sinf(elevation)+0.0f,cosEr*cosA);
			vtx.TCoords.set(tcU, j*tcV);

			vtx.Normal = -vtx.Pos;
			vtx.Normal.normalize();

			Buffer->Vertices.push_back(vtx);
			elevation -= elevation_step;
		}
		azimuth += azimuth_step;
	}

	for (k = 0; k < horiRes; ++k)
	{
		Buffer->Indices.push_back(vertRes+2+(vertRes+1)*k);
		Buffer->Indices.push_back(1+(vertRes+1)*k);
		Buffer->Indices.push_back(0+(vertRes+1)*k);

		for (u32 j = 1; j < vertRes; ++j)
		{
			Buffer->Indices.push_back(vertRes+2+(vertRes+1)*k+j);
			Buffer->Indices.push_back(1+(vertRes+1)*k+j);
			Buffer->Indices.push_back(0+(vertRes+1)*k+j);

			Buffer->Indices.push_back(vertRes+1+(vertRes+1)*k+j);
			Buffer->Indices.push_back(vertRes+2+(vertRes+1)*k+j);
			Buffer->Indices.push_back(0+(vertRes+1)*k+j);
		}
	}
}


CSkyDomeSceneNode::~CSkyDomeSceneNode()
{
	if (Buffer)
		Buffer->drop();
}


//! renders the node.
void CSkyDomeSceneNode::render()
{
	video::IVideoDriver* driver = SceneManager->getVideoDriver();
	scene::ICameraSceneNode* camera = SceneManager->getActiveCamera();

	if (!camera || !driver)
		return;

	if ( !camera->isOrthogonal() )
	{
		core::matrix4 mat(AbsoluteTransformation);
		mat.setTranslation(camera->getAbsolutePosition());

		driver->setTransform(video::ETS_WORLD, mat);

		driver->setMaterial(Buffer->Material);
		driver->drawMeshBuffer(Buffer);
	}

	// for debug purposes only:
	if ( DebugDataVisible )
	{
		video::SMaterial m;
		m.Lighting = false;
		driver->setMaterial(m);

		if ( DebugDataVisible & scene::EDS_NORMALS )
		{
			IAnimatedMesh * arrow = SceneManager->addArrowMesh (
					"__debugnormal2", 0xFFECEC00,
					0xFF999900, 4, 8, 1.f * 40.f, 0.6f * 40.f, 0.05f * 40.f, 0.3f * 40.f);
			if ( 0 == arrow )
			{
				arrow = SceneManager->getMesh ( "__debugnormal2" );
			}
			IMesh *mesh = arrow->getMesh(0);

			// find a good scaling factor
			core::matrix4 m2;

			// draw normals
			const scene::IMeshBuffer* mb = Buffer;
			const u32 vSize = video::getVertexPitchFromType(mb->getVertexType());
			const video::S3DVertex* v = ( const video::S3DVertex*)mb->getVertices();
			for ( u32 i=0; i != mb->getVertexCount(); ++i )
			{
				// align to v->Normal
				core::quaternion quatRot(v->Normal.X, 0.f, -v->Normal.X, 1+v->Normal.Y);
				quatRot.normalize();
				quatRot.getMatrix(m2, v->Pos);

				m2 = AbsoluteTransformation * m2;

				driver->setTransform(video::ETS_WORLD, m2);
				for (u32 a = 0; a != mesh->getMeshBufferCount(); ++a)
					driver->drawMeshBuffer(mesh->getMeshBuffer(a));

				v = (const video::S3DVertex*) ( (u8*) v + vSize );
			}
			driver->setTransform(video::ETS_WORLD, AbsoluteTransformation);
		}

		// show mesh
		if ( DebugDataVisible & scene::EDS_MESH_WIRE_OVERLAY )
		{
			m.Wireframe = true;
			driver->setMaterial(m);

			driver->drawMeshBuffer( Buffer );
		}
	}

}


//! returns the axis aligned bounding box of this node
const core::aabbox3d<f32>& CSkyDomeSceneNode::getBoundingBox() const
{
	return Buffer->BoundingBox;
}


void CSkyDomeSceneNode::OnRegisterSceneNode()
{
	if (IsVisible)
	{
		SceneManager->registerNodeForRendering(this, ESNRP_SKY_BOX );
	}

	ISceneNode::OnRegisterSceneNode();
}


//! returns the material based on the zero based index i. To get the amount
//! of materials used by this scene node, use getMaterialCount().
//! This function is needed for inserting the node into the scene hirachy on a
//! optimal position for minimizing renderstate changes, but can also be used
//! to directly modify the material of a scene node.
video::SMaterial& CSkyDomeSceneNode::getMaterial(u32 i)
{
	return Buffer->Material;
}


//! returns amount of materials used by this scene node.
u32 CSkyDomeSceneNode::getMaterialCount() const
{
	return 1;
}


}
}

