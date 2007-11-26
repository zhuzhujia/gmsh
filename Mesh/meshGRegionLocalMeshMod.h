#ifndef _MESHGREGIONLOCALMESHMOD_H_
#define _MESHGREGIONLOCALMESHMOD_H_

// Copyright (C) 1997-2007 C. Geuzaine, J.-F. Remacle
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
// 
// Please report all bugs and problems to <gmsh@geuz.org>.

#include "meshGRegionDelaunayInsertion.h"
#include "qualityMeasures.h"

void gmshEdgeSwap (std::vector<MTet4 *> &newTets,
		   MTet4 *tet, 
		   int iLocalEdge,
		   const gmshQualityMeasure4Tet &cr);
void gmshFaceSwap (std::vector<MTet4 *> &newTets,
		   MTet4 *tet, 
		   int iLocalFace,
		   const gmshQualityMeasure4Tet &cr);
void gmshSmoothVertex ( MTet4 *t, 
			int iLocalVertex);
  
#endif


