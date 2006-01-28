// $Id: 3D_BGMesh.cpp,v 1.42 2006-01-28 22:30:32 geuzaine Exp $
//
// Copyright (C) 1997-2006 C. Geuzaine, J.-F. Remacle
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

#include "Gmsh.h"
#include "Mesh.h"
#include "Views.h"
#include "Numeric.h"
#include "Context.h"
#include "OctreePost.h"

extern Mesh *THEM;
extern Context_T CTX;

static OctreePost *BGM_OCTREE = NULL;
static double BGM_MAX = 0.;

int BGMWithView(Post_View * ErrView)
{
  if(BGM_OCTREE) delete BGM_OCTREE;
  BGM_OCTREE = new OctreePost(ErrView);
  BGM_MAX = ErrView->Max;
  Create_BgMesh(ONFILE, .2, THEM);
  return 1 ;
}

double Lc_XYZ(double X, double Y, double Z, Mesh * m)
{
  double l = 0.;
  double fact[9] = {0.001, 0.0025, 0.005, 0.0075, 0.01, 0.025, 0.05, 0.075, 0.1};

  switch (m->BGM.Typ) {
  case FUNCTION:
    // for testing...
    l = 0.1 * fabs(cos(2 * 3.14159 * X) * cos( 2 * 3.14159 * Y))  + 0.01;
    break;
  case CONSTANT:
    l = m->BGM.lc;
    break;
  case ONFILE:
    if(!BGM_OCTREE->searchScalar(X, Y, Z, &l, 0)){
      // try really hard to find an element around the point
      for(int i = 0; i < 9; i++){
	double eps = CTX.lc * fact[i];
	if(BGM_OCTREE->searchScalar(X + eps, Y, Z, &l, 0)) break;
	if(BGM_OCTREE->searchScalar(X - eps, Y, Z, &l, 0)) break;
	if(BGM_OCTREE->searchScalar(X, Y + eps, Z, &l, 0)) break;
	if(BGM_OCTREE->searchScalar(X, Y - eps, Z, &l, 0)) break;
	if(BGM_OCTREE->searchScalar(X, Y, Z + eps, &l, 0)) break;
	if(BGM_OCTREE->searchScalar(X, Y, Z - eps, &l, 0)) break;
	if(BGM_OCTREE->searchScalar(X + eps, Y - eps, Z - eps, &l, 0)) break;
	if(BGM_OCTREE->searchScalar(X + eps, Y + eps, Z - eps, &l, 0)) break;
	if(BGM_OCTREE->searchScalar(X - eps, Y - eps, Z - eps, &l, 0)) break;
	if(BGM_OCTREE->searchScalar(X - eps, Y + eps, Z - eps, &l, 0)) break;
	if(BGM_OCTREE->searchScalar(X + eps, Y - eps, Z + eps, &l, 0)) break;
	if(BGM_OCTREE->searchScalar(X + eps, Y + eps, Z + eps, &l, 0)) break;
	if(BGM_OCTREE->searchScalar(X - eps, Y - eps, Z + eps, &l, 0)) break;
	if(BGM_OCTREE->searchScalar(X - eps, Y + eps, Z + eps, &l, 0)) break;
      }
    }
    if(l <= 0) l = BGM_MAX;
    break;
  case WITHPOINTS:
    Msg(GERROR, "We should never call Lc_XYZ with BGM.Typ == WITHPOINTS!");
    l = 1.0;
    break;
  }

  if(l <= 0.){
    Msg(WARNING, "Characteristic length <= 0 at point (%g,%g,%g)", X, Y, Z);
    l = CTX.lc / 10.;
  }

  return CTX.mesh.lc_factor * l;
}

// Export a mesh as a post-processing view (that can be modified into
// a background characteristic length field if desired)

static FILE *statfile = NULL;

void ExportStatistics(void *a, void *b)
{
  Element *ele = *(Element**)a;
  if(statfile) ele->ExportStatistics(statfile);
}

void ExportMeshStatistics(Mesh * M, char *filename, int volume, int surface)
{
  if(!Tree_Nbr(M->Volumes) && !Tree_Nbr(M->Surfaces)){
    Msg(GERROR, "No volumes or surfaces to save");
    return;
  }
  else if(volume && !surface && !Tree_Nbr(M->Volumes)){
    Msg(GERROR, "No volumes to save");
    return;
  }
  else if(!volume && surface && !Tree_Nbr(M->Surfaces)){
    Msg(GERROR, "No surfaces to save");
    return;
  }

  statfile = fopen(filename, "w");

  if(!statfile) {
    Msg(GERROR, "Unable to open file '%s'", filename);
    return;
  }

  if(volume && Tree_Nbr(M->Volumes)){
    List_T *l = Tree2List(M->Volumes);
    fprintf(statfile, "View \"Volume Statistics\" {\n");
    fprintf(statfile, "T2(1.e5,30,%d){\"Characteristic Length\", \"Gamma\", \"Eta\", "
	    "\"Rho\", \"Element Number\"};\n", (1<<16)|(4<<8));
    for(int i = 0; i < List_Nbr(l); i++) {
      Volume *vol;
      List_Read(l, i, &vol);
      Tree_Action(vol->Simplexes, ExportStatistics);
      Tree_Action(vol->SimplexesBase, ExportStatistics);
      Tree_Action(vol->Hexahedra, ExportStatistics);
      Tree_Action(vol->Prisms, ExportStatistics);
      Tree_Action(vol->Pyramids, ExportStatistics);
    }
    List_Delete(l);
    fprintf(statfile, "};\n");
  }
  
  if(surface && Tree_Nbr(M->Surfaces)){
    List_T *l = Tree2List(M->Surfaces);
    fprintf(statfile, "View \"Surface Statistics\" {\n");
    fprintf(statfile, "T2(1.e5,30,%d){\"Characteristic Length\", \"Gamma\", \"Eta\", "
	    "\"Rho\", \"Element Number\"};\n", (1<<16)|(4<<8));
    for(int i = 0; i < List_Nbr(l); i++) {
      Surface *surf;
      List_Read(l, i, &surf);
      Tree_Action(surf->Simplexes, ExportStatistics);
      Tree_Action(surf->SimplexesBase, ExportStatistics);
      Tree_Action(surf->Quadrangles, ExportStatistics);
    }
    List_Delete(l);
    fprintf(statfile, "};\n");
  }

  fclose(statfile);
  statfile = NULL;
}
