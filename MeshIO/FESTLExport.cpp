#include "FESTLExport.h"
#include <GeomLib/GObject.h>
#include <MeshTools/GModel.h>

FESTLExport::FESTLExport(void)
{
}

FESTLExport::~FESTLExport(void)
{
}

bool FESTLExport::Export(FEProject& prj, const char* szfile)
{
	FILE* fp = fopen(szfile, "wt");
	if (fp == 0) return false;

	int i, j, n;

	FEModel* ps = &prj.GetFEModel();
	GModel& model = ps->GetModel();

	// only the selected object is exported, unless no object is selected, 
	// then we export them all. So, we first need to know how many are selected.
	int nsel = 0;
	for (n=0; n<model.Objects(); ++n)
	{
		GObject* po = model.Object(n);
		if (po->IsSelected()) nsel++;
	}

	// export the selected objects (or all, if none are selected)
	for (n=0; n<model.Objects(); ++n)
	{
		GObject* po = model.Object(n);
		if ((nsel==0) || po->IsSelected())
		{
			FEMeshBase* pm = po->GetEditableMesh();
			if (pm == 0) return errf("Not all objects are meshed.");
			const char* szname = po->GetName().c_str();
			if (strlen(szname) == 0) szname = "object";

			fprintf(fp, "solid %s\n", szname);

			for (i=0; i<pm->Faces(); ++i)
			{
				FEFace& face = pm->Face(i);
				fprintf(fp, "facet normal %g %g %g\n", face.m_fn.x, face.m_fn.y, face.m_fn.z);
				fprintf(fp, "outer loop\n");
				for (j=0; j<face.Nodes(); ++j)
				{
					vec3d& p = pm->Node(face.n[j]).r;
					vec3d r = pm->LocalToGlobal(p);
					fprintf(fp, "vertex %g %g %g\n", r.x, r.y, r.z);
				}
				fprintf(fp, "endloop\n");
				fprintf(fp, "endfacet\n");
			}
			fprintf(fp, "endsolid\n");
		}
	}

	fclose(fp);

	return true;
}
