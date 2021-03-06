// Copyright (C) 2002-2007 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "IrrCompileConfig.h"
#ifdef _IRR_COMPILE_WITH_MD2_LOADER_

#include "CMD2MeshFileLoader.h"
#include "CAnimatedMeshMD2.h"

namespace irr
{
namespace scene
{

//! Constructor
CMD2MeshFileLoader::CMD2MeshFileLoader()
{
}


//! returns true if the file maybe is able to be loaded by this class
//! based on the file extension (e.g. ".bsp")
bool CMD2MeshFileLoader::isALoadableFileExtension(const c8* filename)
{
	return strstr(filename, ".md2")!=0;
}



//! creates/loads an animated mesh from the file.
//! \return Pointer to the created mesh. Returns 0 if loading failed.
//! If you no longer need the mesh, you should call IAnimatedMesh::drop().
//! See IUnknown::drop() for more information.
IAnimatedMesh* CMD2MeshFileLoader::createMesh(irr::io::IReadFile* file)
{
	IAnimatedMesh* msh = 0;

	bool success = false;
	msh = new CAnimatedMeshMD2();
	success = ((CAnimatedMeshMD2*)msh)->loadFile(file);
	if (success)
		return msh;

	msh->drop();

	return 0;
}

} // end namespace scene
} // end namespace irr


#endif // _IRR_COMPILE_WITH_MD2_LOADER_
