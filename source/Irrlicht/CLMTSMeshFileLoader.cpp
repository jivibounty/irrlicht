// Copyright (C) 2002-2007 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h
// This file was written by Jonas Petersen and modified by Nikolaus Gebhardt.
// See CLMTSMeshFileLoder.h for details.
/*

CLMTSMeshFileLoader.cpp

LMTSMeshFileLoader
Written by Jonas Petersen (a.k.a. jox)

Version 1.5 - 15 March 2005

Get the latest version here: http://development.mindfloaters.de/

This is an addon class for the Irrlicht engine by Nikolaus Gebhardt (http://irrlicht.sourceforge.net).
With this release the Irrlicht engine is at version 0.6

This class allows loading meshes with lightmaps (*.lmts + *.tga files) that were created
using Pulsar LMTools by Lord Trancos (http://www.geocities.com/dxlab/index_en.html)

Notes:
- This version does not recognice/support user data in the *.lmts files.
- The lightmap TGA's generated by LMTools doesn't work in Irrlicht for some reason (the
  lightmaps look messed up). Opening and resaving them in a graphics app will solve
  the problem (tested only with Photoshop).


License:
--------

It's free. You are encouraged to give me credit if you use it in your software.

Version History:
----------------

Version 1.5 - 15 March 2005
- Added the switch LMTS_INTEGRATED_IN_IRRLICHT. This was needed because
  of access problems to the irrlicht Logger.
- Did a better cleanup. No memory leaks in case of an loading error.
- Added "#include <stdio.h>" for sprintf.

Version 1.4 - 12 March 2005
- Fixed bug in texture and subset loading code that would possibly cause crash.
- Fixed memory cleanup to avoid leak when loading more then one mesh
- Used the irrlicht Logger instead of cerr to output warnings and errors.
  For this I had to change the constructor
  from:
	CLMTSMeshFileLoader(io::IFileSystem* fs, video::IVideoDriver* driver)
  to:
	CLMTSMeshFileLoader(IrrlichtDevice* device)

Version 1.3 - 15 February 2005
- Fixed bug that prevented loading more than one different lmts files.
- Removed unnecessary "#include <os.h>".
- Added "std::" in front of "cerr". This was necessary for Visual Studio .NET,
  I hope it's not disturbing other compilers.
- Added warning message when a texture can not be loaded.
- Changed the documentation a bit (minor).

Version 1.2
- To avoid confusion I skipped version 1.2 because the website was offering
version 1.2 even though it was only version 1.1. Sorry about that.

Version 1.1 - 29 July 2004
- Added setTexturePath() function
- Minor improvements

Version 1.0 - 29 July 2004
- Initial release


*/
//////////////////////////////////////////////////////////////////////

#include "SMeshBufferLightMap.h"
#include "SAnimatedMesh.h"
#include "SMeshBuffer.h"
#include "irrString.h"
#include "IAttributes.h"
#include "ISceneManager.h"
#include "CLMTSMeshFileLoader.h"
#if LMTS_INTEGRATED_IN_IRRLICHT
#include "os.h"
#endif

namespace irr
{
namespace scene
{

#if LMTS_INTEGRATED_IN_IRRLICHT

CLMTSMeshFileLoader::CLMTSMeshFileLoader(io::IFileSystem* fs, video::IVideoDriver* driver,
										 io::IAttributes* parameters)
	: Textures(0), TextureIDs(0), Subsets(0), Triangles(0), Mesh(0),
	NumTextures(0), NumLightMaps(0), Parameters(parameters), Driver(driver), FileSystem(fs)
{
	if (Driver)
		Driver->grab();

	if (FileSystem)
		FileSystem->grab();
}

#else

CLMTSMeshFileLoader::CLMTSMeshFileLoader(IrrlichtDevice* device)
: Textures(0), TextureIDs(0), Subsets(0), Triangles(0), Mesh(0),
	NumTextures(0), NumLightMaps(0), Parameters(0), Logger(0)
{
	FileSystem = device->getFileSystem();
	FileSystem->grab();

	Driver = device->getVideoDriver();
	Driver->grab();

	Logger = device->getLogger();
	Parameters = device->getSceneManager()->getParameters();
}

#endif

CLMTSMeshFileLoader::~CLMTSMeshFileLoader()
{
	if (Mesh)
		Mesh->drop();

	if (Driver)
		Driver->drop();

	if (FileSystem)
		FileSystem->drop();
}

void CLMTSMeshFileLoader::cleanup() {
	delete [] Textures;
	delete [] TextureIDs;
	delete [] Subsets;
	delete [] Triangles;
}

bool CLMTSMeshFileLoader::isALoadableFileExtension(const c8* filename) {
	return strstr(filename, ".lmts") != 0;
}

IAnimatedMesh* CLMTSMeshFileLoader::createMesh(irr::io::IReadFile* file) {

	u32 i;
	u32 id;

	// HEADER

	file->read(&Header, sizeof(SLMTSHeader));
	if (Header.MagicID != 0x53544D4C) { // "LMTS"
		LMTS_LOG("LMTS ERROR: wrong header magic id!", ELL_ERROR);
		return 0;
	}

	// TEXTURES

	file->read(&id, sizeof(u32));
	if (id != 0x54584554) { // "TEXT"
		LMTS_LOG("LMTS ERROR: wrong texture magic id!", ELL_ERROR);
		return 0;
	}

	Textures = new SLMTSTextureInfoEntry[Header.TextureCount];
	TextureIDs = new u16[Header.TextureCount];

	NumLightMaps = NumTextures = 0;

	for (i=0; i<Header.TextureCount; i++) {
		file->read(&Textures[i], sizeof(SLMTSTextureInfoEntry));
		if (Textures[i].Flags & 1) {
			TextureIDs[i] = NumLightMaps;
			NumLightMaps++;
		} else {
			TextureIDs[i] = NumTextures;
			NumTextures++;
		}

	}

	// SUBSETS

	file->read(&id, sizeof(u32));
	if (id != 0x53425553) { // "SUBS"
		LMTS_LOG("LMTS ERROR: wrong subset magic id!", ELL_ERROR);
		cleanup();
		return 0;
	}

	Subsets = new SLMTSSubsetInfoEntry[Header.SubsetCount];

	for (i=0; i<Header.SubsetCount; i++) {
		file->read(&Subsets[i], sizeof(SLMTSSubsetInfoEntry));
	}

	// TRIANGLES

	file->read(&id, sizeof(u32));
	if (id != 0x53495254) { // "TRIS"
		LMTS_LOG("LMTS ERROR: wrong triangle magic id!", ELL_ERROR);
		cleanup();
		return 0;
	}

	Triangles = new SLMTSTriangleDataEntry[(Header.TriangleCount*3)];

	for (i=0; i<(Header.TriangleCount*3); i++) {
		file->read(&Triangles[i], sizeof(SLMTSTriangleDataEntry));
	}

	/////////////////////////////////////////////////////////////////

	constructMesh();

	loadTextures();

	cleanup();

	SAnimatedMesh* am = new SAnimatedMesh();
	am->Type = EAMT_LMTS; // not unknown to irrlicht anymore

	am->addMesh(Mesh);
	am->recalculateBoundingBox();
	Mesh->drop();
	Mesh = 0;
	return am;
}

void CLMTSMeshFileLoader::constructMesh()
{
	s32 i;

	if (Mesh)
		Mesh->drop();

	Mesh = new SMesh();

	for (i=0; i<Header.SubsetCount; i++) {

		scene::SMeshBufferLightMap* meshBuffer = new scene::SMeshBufferLightMap();

		meshBuffer->Material.MaterialType = video::EMT_LIGHTMAP; // EMT_LIGHTMAP_M2/EMT_LIGHTMAP_M4 also possible
		meshBuffer->Material.Wireframe = false;
		meshBuffer->Material.Lighting = false;
		meshBuffer->Material.BilinearFilter = true;

		Mesh->addMeshBuffer(meshBuffer);

		meshBuffer->drop();

		u32 offs = Subsets[i].Offset * 3;

		for (u32 sc=0; sc<Subsets[i].Count; sc++) {

			u32 idx = meshBuffer->getVertexCount();

			for (s32 vu=0; vu<3; ++vu)
			{
				video::S3DVertex2TCoords currentVertex;
				SLMTSTriangleDataEntry *v = &Triangles[offs+(3*sc)+vu];

				currentVertex.Color.set(255,255,255,255);

				currentVertex.Pos.X = v->X;
				currentVertex.Pos.Y = v->Y;
				currentVertex.Pos.Z = v->Z;
				currentVertex.TCoords.X = v->U1;
				currentVertex.TCoords.Y = v->V1;
				currentVertex.TCoords2.X = v->U2;
				currentVertex.TCoords2.Y = v->V2;

				meshBuffer->Vertices.push_back(currentVertex);
			}

			meshBuffer->Indices.push_back(idx);
			meshBuffer->Indices.push_back(idx+1);
			meshBuffer->Indices.push_back(idx+2);
		}
	}

	for (u32 j=0; j<Mesh->MeshBuffers.size(); ++j)
		((SMeshBufferLightMap*)Mesh->MeshBuffers[j])->recalculateBoundingBox();

	Mesh->recalculateBoundingBox();
}

void CLMTSMeshFileLoader::loadTextures()
{
	if (!Driver || !FileSystem)
		return;

	core::stringc s;

	// load textures

	core::array<video::ITexture*> tex;
	tex.set_used(NumTextures);

	core::array<video::ITexture*> lig;
	lig.set_used(NumLightMaps);

	s32 t;
	s32 tx_count = 0;
	s32 lm_count = 0;
	core::stringc Path = Parameters->getAttributeAsString(LMTS_TEXTURE_PATH);

	for (t=0; t<Header.TextureCount; t++)
	{
		video::ITexture* tmptex = 0;
		s = Path;
		s.append(Textures[t].Filename);

		if (FileSystem->existFile(s.c_str()))
			tmptex = Driver->getTexture(s.c_str());
		else
		{
			char buf[300]; // filenames may be 256 bytes long
			sprintf(buf, "LMTS WARNING: Texture does not exist: %s", s.c_str());
			LMTS_LOG(buf, ELL_WARNING);
		}


		if (Textures[t].Flags & 1) {
			lig[lm_count++] = tmptex;
		} else {
			tex[tx_count++] = tmptex;
		}

	}

	// attach textures to materials.

	s32 i;
	for (i=0; i<Header.SubsetCount; i++)
		{
			SMeshBufferLightMap* b = (SMeshBufferLightMap*)Mesh->getMeshBuffer(i);

			if (Subsets[i].TextID1 < Header.TextureCount)
				b->Material.Textures[0] = tex[TextureIDs[Subsets[i].TextID1]];
			if (Subsets[i].TextID2 < Header.TextureCount)
				b->Material.Textures[1] = lig[TextureIDs[Subsets[i].TextID2]];

			if (!b->Material.Textures[1])
				b->Material.MaterialType = video::EMT_SOLID;
		}

}


} // end namespace scene
} // end namespace irr
