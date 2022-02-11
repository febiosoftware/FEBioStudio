#include "stdafx.h"
#include "PostSessionFile.h"
#include <QtCore/QDir>
#include <XML/XMLWriter.h>
#include <XML/XMLReader.h>
#include <PostGL/GLModel.h>
#include "PostDocument.h"
#include <PostLib/FEPostMesh.h>
#include <XPLTLib/xpltFileReader.h>
#include <PostLib/FELSDYNAimport.h>
#include <PostLib/FEKinemat.h>
#include "FEKinematFileReader.h"
#include <GeomLib/GObject.h>

PostSessionFileReader::PostSessionFileReader(CPostDocument* doc) : m_doc(doc)
{
	m_openFile = nullptr;
}

PostSessionFileReader::~PostSessionFileReader()
{
	delete m_openFile;
}

FileReader* PostSessionFileReader::GetOpenFileReader()
{
	return m_openFile;
}

float PostSessionFileReader::GetFileProgress() const
{
	if (m_openFile == nullptr) return 0.f;
	else m_openFile->GetFileProgress();
    return 1.f;
}

bool PostSessionFileReader::Load(const char* szfile)
{
	if (szfile == nullptr) return false;

	// we'll use this for converting to absolute file paths.
	QFileInfo fi(szfile);
	QDir currentDir(fi.absolutePath());

	XMLReader xml;
	if (xml.Open(szfile) == false) return false;

	XMLTag tag;
	if (xml.FindTag("febiostudio_post_session", tag) == false)
	{
		return false;
	}

	m_doc->SetGLModel(nullptr);
	Post::FEPostModel* fem = m_doc->GetFEModel();

	try {
		++tag;
		do
		{
			if (tag == "open")
			{
				const char* szfile = tag.AttributeValue("file");
				xpltFileReader* xplt = new xpltFileReader(fem);
				m_openFile = xplt; // assign this before we load so that we can monitor progress.
				if (xplt->Load(szfile) == false)
				{
					return false;
				}

				// now create a GL model
				m_doc->SetGLModel(new Post::CGLModel(fem));

				// initialize
				if (m_doc->Initialize() == false) return false;
				m_doc->SetInitFlag(true);
			}
			else if (tag == "kinemat")
			{
				int n[3] = { 1,999,1 };
				std::string modelFile, kineFile;
				++tag;
				do
				{
					if (tag == "model_file") tag.value(modelFile);
					if (tag == "kine_file") tag.value(kineFile);
					if (tag == "range") tag.value(n, 3);
					++tag;
				} while (!tag.isend());

				// create absolute file names for model and kine files
				modelFile = currentDir.absoluteFilePath(QString::fromStdString(modelFile)).toStdString();
				kineFile = currentDir.absoluteFilePath(QString::fromStdString(kineFile)).toStdString();

				FEKinematFileReader* kine = new FEKinematFileReader(m_doc);
				kine->SetModelFile(modelFile);
				kine->SetKineFile(kineFile);
				kine->SetRange(n[0], n[1], n[2]);

				m_openFile = kine; // assign this before we load so that we can monitor progress.
				kine->Load(nullptr); // this class doesn't use the file name passed, so we just pass nullptr.
			}
			else if (tag == "material")
			{
				const char* szid = tag.AttributeValue("id");
				int nid = atoi(szid) - 1;
				if ((nid >= 0) && (nid < fem->Materials()))
				{
					Post::FEMaterial* mat = fem->GetMaterial(nid);

					++tag;
					do
					{
						if (tag == "diffuse") tag.value(mat->diffuse);
						if (tag == "ambient") tag.value(mat->ambient);
						if (tag == "specular") tag.value(mat->specular);
						if (tag == "emission") tag.value(mat->emission);
						if (tag == "mesh_color") tag.value(mat->meshcol);
						if (tag == "shininess") tag.value(mat->shininess);
						if (tag == "transparency") tag.value(mat->transparency);
						++tag;
					} while (!tag.isend());
				}
			}
			else if (tag == "mesh:nodeset")
			{
				const char* szname = tag.AttributeValue("name");
				vector<int> nodeList;
				++tag;
				do {
					if (tag == "nodes")
					{
						int l[16];
						int m = tag.value(l, 16);
						for (int i = 0; i < m; ++i) nodeList.push_back(l[i] - 1);
					}
					++tag;
				} while (!tag.isend());

				GObject* po = m_doc->GetActiveObject();
				if (po)
				{
					FENodeSet* pg = new FENodeSet(po, nodeList);
					pg->SetName(szname);
					po->AddFENodeSet(pg);
				}
			}
			else if (tag == "mesh:edgeset")
			{
				const char* szname = tag.AttributeValue("name");
				vector<int> edgeList;
				++tag;
				do {
					if (tag == "edges")
					{
						int l[16];
						int m = tag.value(l, 16);
						for (int i = 0; i < m; ++i) edgeList.push_back(l[i] - 1);
					}
					++tag;
				} while (!tag.isend());

				GObject* po = m_doc->GetActiveObject();
				if (po)
				{
					FEEdgeSet* pg = new FEEdgeSet(po, edgeList);
					pg->SetName(szname);
					po->AddFEEdgeSet(pg);
				}
			}
			else if (tag == "mesh:surface")
			{
				const char* szname = tag.AttributeValue("name");
				vector<int> faceList;
				++tag;
				do {
					if (tag == "faces")
					{
						int l[16];
						int m = tag.value(l, 16);
						for (int i = 0; i < m; ++i) faceList.push_back(l[i] - 1);
					}
					++tag;
				} while (!tag.isend());

				GObject* po = m_doc->GetActiveObject();
				if (po)
				{
					FESurface* pg = new FESurface(po, faceList);
					pg->SetName(szname);
					po->AddFESurface(pg);
				}
			}
			else if (tag == "mesh:part")
			{
				const char* szname = tag.AttributeValue("name");
				vector<int> elemList;
				++tag;
				do {
					if (tag == "elems")
					{
						int l[16];
						int m = tag.value(l, 16);
						for (int i = 0; i < m; ++i) elemList.push_back(l[i] - 1);
					}
					++tag;
				} while (!tag.isend());

				GObject* po = m_doc->GetActiveObject();
				if (po)
				{
					FEPart* pg = new FEPart(po, elemList);
					pg->SetName(szname);
					po->AddFEPart(pg);
				}
			}
			else if (tag == "plot")
			{
				const char* sztype = tag.AttributeValue("type");
				Post::CGLPlot* plot = FSCore::CreateClass<Post::CGLPlot>(CLASS_PLOT, sztype);

				const char* szname = tag.AttributeValue("name", true);
				if (szname) plot->SetName(szname);

				Post::CGLModel* glm = m_doc->GetGLModel();
				glm->AddPlot(plot);

				++tag;
				do
				{
					Param* p = plot->GetParam(tag.Name());
					if (p)
					{
						switch (p->GetParamType())
						{
						case Param_BOOL: { bool b; tag.value(b); p->SetBoolValue(b); } break;
						case Param_INT: { int n; tag.value(n); p->SetIntValue(n); } break;
						case Param_CHOICE: { int n; tag.value(n); p->SetIntValue(n); } break;
						case Param_FLOAT: { double g; tag.value(g); p->SetFloatValue(g); } break;
						case Param_VEC3D: { vec3d v; tag.value(v); p->SetVec3dValue(v); } break;
						case Param_COLOR: { GLColor c; tag.value(c); p->SetColorValue(c); } break;
						}
					}
					++tag;
				} while (!tag.isend());

				plot->UpdateData(true);
			}
			//			else xml.SkipTag(tag);
			else return false;
			++tag;
		} while (!tag.isend());
	}
	catch (...)
	{
		// TODO: We need this for catching end-of-file. 
	}

	xml.Close();

	return true;
}

//=============================================================================
PostSessionFileWriter::PostSessionFileWriter(CPostDocument* doc) : m_doc(doc)
{

}

bool PostSessionFileWriter::Write(const char* szfile)
{
	if (m_doc == nullptr) return false;
	if (szfile == nullptr) return false;

	Post::FEPostModel* fem = m_doc->GetFEModel();
	if (fem == nullptr) return false;

	XMLWriter xml;
	if (xml.open(szfile) == false) return false;

	// we'll use this for converting to relative file paths.
	QFileInfo fi(szfile);
	QDir currentDir(fi.absolutePath());

	XMLElement root("febiostudio_post_session");
	root.add_attribute("version", "1.0");
	xml.add_branch(root);
	{
		// we need to see if this document was opended with the PostSessionFileReader
		FileReader* reader = m_doc->GetFileReader();
		if (dynamic_cast<PostSessionFileReader*>(reader))
		{
			PostSessionFileReader* sessionReader = dynamic_cast<PostSessionFileReader*>(reader);

			// see if the data was read from a kinemat reader
			FileReader* openFileReader = sessionReader->GetOpenFileReader();
			FEKinematFileReader* kine = dynamic_cast<FEKinematFileReader*>(openFileReader);
			if (kine)
			{
				// create absolute file names for model and kine files
				std::string modelFile = currentDir.relativeFilePath(QString::fromStdString(kine->GetModelFile())).toStdString();
				std::string kineFile  = currentDir.relativeFilePath(QString::fromStdString(kine->GetKineFile ())).toStdString();

				xml.add_branch("kinemat");
				{
					xml.add_leaf("model_file", modelFile);
					xml.add_leaf("kine_file", kineFile);
					int n[3] = { kine->GetMin(), kine->GetMax(), kine->GetStep() };
					xml.add_leaf("range", n, 3);
				}
				xml.close_branch();
			}
			else
			{
				// save plot file
				std::string plotFile = m_doc->GetDocFilePath();
				XMLElement plt("open");
				plt.add_attribute("file", plotFile);
				xml.add_empty(plt);
			}
		}
		else
		{
			// save plot file
			std::string plotFile = m_doc->GetDocFilePath();
			XMLElement plt("open");
			plt.add_attribute("file", plotFile);
			xml.add_empty(plt);
		}

		// save material settings
		for (int i = 0; i < fem->Materials(); ++i)
		{
			Post::FEMaterial* mat = fem->GetMaterial(i);
			XMLElement el("material");
			el.add_attribute("id", i + 1);
			el.add_attribute("name", mat->GetName());
			xml.add_branch(el);
			{
				xml.add_leaf("diffuse", mat->diffuse);
				xml.add_leaf("ambient", mat->ambient);
				xml.add_leaf("specular", mat->specular);
				xml.add_leaf("emission", mat->emission);
				xml.add_leaf("mesh_color", mat->meshcol);
				xml.add_leaf("shininess", mat->shininess);
				xml.add_leaf("transparency", mat->transparency);
			}
			xml.close_branch(); // material
		}

		// save selections
		GObject* po = m_doc->GetActiveObject();
		if (po)
		{
			for (int i = 0; i < po->FENodeSets(); ++i)
			{
				FENodeSet* pg = po->GetFENodeSet(i);

				XMLElement el("mesh:nodeset");
				el.add_attribute("name", pg->GetName());
				xml.add_branch(el);
				{
					std::list<int> items = pg->CopyItems();
					std::list<int>::iterator it = items.begin();
					int N = items.size();
					int l[16];
					for (int n = 0; n < N; n += 16)
					{
						int m = (n + 16 <= N ? 16 : N - n);
						for (int k = 0; k < m; ++k) l[k] = 1 + (*it++);
						xml.add_leaf("nodes", l, m);
					}
				}
				xml.close_branch();
			}

			for (int i = 0; i < po->FEEdgeSets(); ++i)
			{
				FEEdgeSet* pg = po->GetFEEdgeSet(i);

				XMLElement el("mesh:edgeset");
				el.add_attribute("name", pg->GetName());
				xml.add_branch(el);
				{
					std::list<int> items = pg->CopyItems();
					std::list<int>::iterator it = items.begin();
					int N = items.size();
					int l[16];
					for (int n = 0; n < N; n += 16)
					{
						int m = (n + 16 <= N ? 16 : N - n);
						for (int k = 0; k < m; ++k) l[k] = 1 + (*it++);
						xml.add_leaf("edges", l, m);
					}
				}
				xml.close_branch();
			}

			for (int i = 0; i < po->FESurfaces(); ++i)
			{
				FESurface* pg = po->GetFESurface(i);

				XMLElement el("mesh:surface");
				el.add_attribute("name", pg->GetName());
				xml.add_branch(el);
				{
					std::list<int> items = pg->CopyItems();
					std::list<int>::iterator it = items.begin();
					int N = items.size();
					int l[16];
					for (int n = 0; n < N; n += 16)
					{
						int m = (n + 16 <= N ? 16 : N - n);
						for (int k = 0; k < m; ++k) l[k] = 1 + (*it++);
						xml.add_leaf("faces", l, m);
					}
				}
				xml.close_branch();
			}

			for (int i = 0; i < po->FEParts(); ++i)
			{
				FEPart* pg = po->GetFEPart(i);

				XMLElement el("mesh:part");
				el.add_attribute("name", pg->GetName());
				xml.add_branch(el);
				{
					std::list<int> items = pg->CopyItems();
					std::list<int>::iterator it = items.begin();
					int N = items.size();
					int l[16];
					for (int n = 0; n < N; n += 16)
					{
						int m = (n + 16 <= N ? 16 : N - n);
						for (int k = 0; k < m; ++k) l[k] = 1 + (*it++);
						xml.add_leaf("elems", l, m);
					}
				}
				xml.close_branch();
			}
		}

		// save post model components
		Post::CGLModel* glm = m_doc->GetGLModel();
		if (glm)
		{
			for (int i = 0; i < glm->Plots(); ++i)
			{
				Post::CGLPlot* plot = glm->Plot(i);

				std::string typeStr = plot->GetTypeString();
				std::string name = plot->GetName();

				XMLElement el("plot");
				el.add_attribute("type", typeStr);
				if (name.empty() == false) el.add_attribute("name", name);
				xml.add_branch(el);
				{
					for (int i = 0; i < plot->Parameters(); ++i)
					{
						Param& pi = plot->GetParam(i);
						const char* sz = pi.GetShortName();

						el.name(sz);
						switch (pi.GetParamType())
						{
						case Param_BOOL: { bool b = pi.GetBoolValue(); el.value(b); } break;
						case Param_INT: { int n = pi.GetIntValue(); el.value(n); } break;
						case Param_CHOICE: { int n = pi.GetIntValue(); el.value(n); } break;
						case Param_FLOAT: { double v = pi.GetFloatValue(); el.value(v); } break;
						case Param_VEC3D: { vec3d v = pi.GetVec3dValue(); el.value(v); } break;
						case Param_COLOR: { GLColor c = pi.GetColorValue(); int v[3] = { c.r, c.g, c.b }; el.value(v, 3); } break;
						}

						xml.add_leaf(el);
					}
				}
				xml.close_branch();
			}
		}
	}
	xml.close_branch(); // root

	xml.close();

	return true;
}
