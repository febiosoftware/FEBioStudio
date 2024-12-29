#include "stdafx.h"
#include "PostSessionFile.h"
#include <QtCore/QDir>
#include <XML/XMLWriter.h>
#include <XML/XMLReader.h>
#include <PostGL/GLModel.h>
#include <PostGL/GLMusclePath.h>
#include "PostDocument.h"
#include <PostLib/FEPostMesh.h>
#include <XPLTLib/xpltFileReader.h>
#include <PostLib/FELSDYNAimport.h>
#include <PostLib/FEKinemat.h>
#include "FEKinematFileReader.h"
#include <GeomLib/GObject.h>
#include <FEBio/FEBioExport.h> // for type_to_string<vec3d>
#include <FEBio/FEBioFormat.h>
#include <PostGL/GLLinePlot.h>
#include <PostGL/GLPlotGroup.h>
#include <FSCore/ClassDescriptor.h>
#include <sstream>

template <> std::string type_to_string<GLColor>(const GLColor& v)
{
	std::stringstream ss;
	int n[3] = { v.r, v.g, v.b };
	ss << n[0] << "," << n[1] << "," << n[2];
	return ss.str();
}

template <> void string_to_type<GLColor>(const std::string& s, GLColor& v);

PostSessionFileReader::PostSessionFileReader(CPostDocument* doc) : m_doc(doc)
{
	m_openFile = nullptr;
	m_fem = nullptr;
	m_pg = nullptr;
	m_szfile = nullptr;
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

void fsps_read_param(Param* p, XMLTag& tag)
{
	switch (p->GetParamType())
	{
	case Param_BOOL  : { bool b; tag.value(b); p->SetBoolValue(b); } break;
	case Param_INT   : { int n; tag.value(n); p->SetIntValue(n); } break;
	case Param_CHOICE: { int n; tag.value(n); p->SetIntValue(n); } break;
	case Param_FLOAT : { double g; tag.value(g); p->SetFloatValue(g); } break;
	case Param_VEC3D : { vec3d v; tag.value(v); p->SetVec3dValue(v); } break;
	case Param_COLOR : { GLColor c; tag.value(c); p->SetColorValue(c); } break;
	case Param_STRING: { std::string s; tag.value(s); p->SetStringValue(s); } break;
	}
}

void fsps_read_parameters(FSObject* po, XMLTag& tag)
{
	++tag;
	do {
		Param* p = po->GetParam(tag.Name());
		if (p)
		{
			fsps_read_param(p, tag);
		}
		else tag.skip();
		++tag;
	}
	while (!tag.isend());
}

bool PostSessionFileReader::Load(const char* szfile)
{
	if (szfile == nullptr) return false;
	m_szfile = szfile;
	m_pg = nullptr;

	XMLReader xml;
	if (xml.Open(szfile) == false) return errf("Failed opening post session file.");

	XMLTag tag;
	if (xml.FindTag("febiostudio_post_session", tag) == false)
	{
		return errf("This is not a valid post session file");
	}

	m_doc->SetGLModel(nullptr);
	m_fem = m_doc->GetFSModel();

	try {
		++tag;
		do
		{
			if (tag == "model")
			{
				if (parse_model(tag) == false) return false;
			}
			else if (tag == "open") // backward compatibility with 1.0
			{
				if (parse_open(tag) == false) return false;
			}
			else if (tag == "kinemat") // backward compatibility with 1.0
			{
				if (parse_kinemat(tag) == false) return false;
			}
			else if (tag == "material")
			{
				if (parse_material(tag) == false) return false;
			}
			else if (tag == "datafield")
			{
				if (parse_datafield(tag) == false) return false;
			}
			else if (tag == "mesh:nodeset")
			{
				if (parse_mesh_nodeset(tag) == false) return false;
			}
			else if (tag == "mesh:edgeset")
			{
				if (parse_mesh_edgeset(tag) == false) return false;
			}
			else if (tag == "mesh:surface")
			{
				if (parse_mesh_surface(tag) == false) return false;
			}
			else if (tag == "mesh:elementset")
			{
				if (parse_mesh_elementset(tag) == false) return false;
			}
			else if (tag == "mesh:part") // backward compatibility with 1.0
			{
				if (parse_mesh_elementset(tag) == false) return false;
			}
			else if (tag == "plot")
			{
				if (parse_plot(tag) == false) return false;
			}
			else if (tag == "view")
			{
				if (parse_view(tag) == false) return false;
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

bool PostSessionFileReader::parse_model(XMLTag& tag)
{
	// we'll use this for converting to absolute file paths.
	QFileInfo fi(m_szfile);
	QDir currentDir(fi.absolutePath());

	// first look for the type attribute
	const char* sztype = tag.AttributeValue("type", true);
	if (sztype)
	{
		if (strcmp(sztype, "kinemat") == 0)
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
		else return errf("Don't know type attribute value");
	}
	else
	{
		const char* szfile = tag.AttributeValue("file");
		std::string modelFile = currentDir.absoluteFilePath(szfile).toStdString();

		// check the extension
		const char* szext = strrchr(szfile, '.');
		if (strcmp(szext, ".xplt") == 0)
		{
			xpltFileReader* xplt = new xpltFileReader(m_fem);
			m_openFile = xplt; // assign this before we load so that we can monitor progress.
		}
		else if (strcmp(szext, ".k") == 0)
		{
			Post::FELSDYNAimport* reader = new Post::FELSDYNAimport(m_fem);
			m_openFile = reader;
		}
		else return errf("Don't know how to read file.");

		if (m_openFile == nullptr) return errf("No file reader allocated.");

		if (m_openFile->Load(modelFile.c_str()) == false)
		{
			return errf("Failed loading model file\n%s", modelFile.c_str());
		}

		// now create a GL model
		m_doc->SetGLModel(new Post::CGLModel(m_fem));

		// initialize
		if (m_doc->Initialize() == false) return errf("Failed initializing document");
		m_doc->SetInitFlag(true);
	}

	return true;
}

bool PostSessionFileReader::parse_open(XMLTag& tag)
{
	// we'll use this for converting to absolute file paths.
	QFileInfo fi(m_szfile);
	QDir currentDir(fi.absolutePath());

	const char* szfile = tag.AttributeValue("file");
	std::string modelFile = currentDir.absoluteFilePath(szfile).toStdString();

	// check the extension
	const char* szext = strrchr(szfile, '.');
	if (strcmp(szext, ".xplt") == 0)
	{
		xpltFileReader* xplt = new xpltFileReader(m_fem);
		m_openFile = xplt; // assign this before we load so that we can monitor progress.
	}
	else if (strcmp(szext, ".k") == 0)
	{
		Post::FELSDYNAimport* reader = new Post::FELSDYNAimport(m_fem);
		m_openFile = reader;
	}
	else return errf("Don't know how to read file.");

	if (m_openFile == nullptr) return errf("No file reader allocated.");

	if (m_openFile->Load(modelFile.c_str()) == false)
	{
		return errf("Failed loading model file\n%s", modelFile.c_str());
	}

	// now create a GL model
	m_doc->SetGLModel(new Post::CGLModel(m_fem));

	// initialize
	if (m_doc->Initialize() == false) return errf("Failed to initialize document");
	m_doc->SetInitFlag(true);

	return true;
}

bool PostSessionFileReader::parse_kinemat(XMLTag& tag)
{
	// we'll use this for converting to absolute file paths.
	QFileInfo fi(m_szfile);
	QDir currentDir(fi.absolutePath());

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
	kine->Load(nullptr); // this class doesn't use the file name passed, so we just pass nullptr

	return true;
}

bool PostSessionFileReader::parse_material(XMLTag& tag)
{
	if (m_fem == nullptr) return false;

	const char* szid = tag.AttributeValue("id");
	int nid = atoi(szid) - 1;
	if ((nid >= 0) && (nid < m_fem->Materials()))
	{
		Post::Material* mat = m_fem->GetMaterial(nid);

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
	else return errf("Invalid material ID");

	return true;
}

bool PostSessionFileReader::parse_datafield(XMLTag& tag)
{
	const char* szname = tag.AttributeValue("name");
	Post::FEPostModel& fem = *m_doc->GetFSModel();
	Post::FEDataManager& dm = *fem.GetDataManager();
	int n = dm.FindDataField(szname);
	if (n < 0) return errf("Failed finding data field %s", szname);

	Post::ModelDataField* data = *dm.DataField(n);
	fsps_read_parameters(data, tag);

	return true;
}

bool PostSessionFileReader::parse_mesh_nodeset(XMLTag& tag)
{
	string name = tag.AttributeValue("name");
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
	if (po && po->GetFEMesh())
	{
		FSMesh* pm = po->GetFEMesh();
		FSNodeSet* pg = new FSNodeSet(pm, nodeList);
		pg->SetName(name);
		pm->AddFENodeSet(pg);
	}

	return true;
}

bool PostSessionFileReader::parse_mesh_edgeset(XMLTag& tag)
{
	string name = tag.AttributeValue("name");
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
	if (po && po->GetFEMesh())
	{
		FSMesh* pm = po->GetFEMesh();
		FSEdgeSet* pg = new FSEdgeSet(pm, edgeList);
		pg->SetName(name);
		pm->AddFEEdgeSet(pg);
	}

	return true;
}

bool PostSessionFileReader::parse_mesh_surface(XMLTag& tag)
{
	string name = tag.AttributeValue("name");
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
	if (po && po->GetFEMesh())
	{
		FSMesh* pm = po->GetFEMesh();
		FSSurface* pg = new FSSurface(pm, faceList);
		pg->SetName(name);
		pm->AddFESurface(pg);
	}

	return true;
}

bool PostSessionFileReader::parse_mesh_elementset(XMLTag& tag)
{
	string name = tag.AttributeValue("name");
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
	if (po && po->GetFEMesh())
	{
		FSMesh* pm = po->GetFEMesh();
		FSElemSet* pg = new FSElemSet(pm, elemList);
		pg->SetName(name);
		pm->AddFEElemSet(pg);
	}

	return true;
}

bool PostSessionFileReader::parse_plot(XMLTag& tag)
{
	const char* sztype = tag.AttributeValue("type");

	// temporary hack
	if (strcmp(sztype, "muscle-path-group") == 0) sztype = "plot-group";

	Post::CGLPlot* plot = FSCore::CreateClass<Post::CGLPlot>(CLASS_PLOT, sztype);

	const char* szname = tag.AttributeValue("name", true);
	if (szname) plot->SetName(szname);

	if (m_pg) m_pg->AddPlot(plot);
	else {
		Post::CGLModel* glm = m_doc->GetGLModel();
		glm->AddPlot(plot);
	}

	++tag;
	do
	{
		Param* p = plot->GetParam(tag.Name());
		if (p)
		{
			fsps_read_param(p, tag);
		}
		else if (tag == "source")
		{
			Post::CGLLinePlot* linePlot = dynamic_cast<Post::CGLLinePlot*>(plot);
			if (linePlot)
			{
				const char* sztype = tag.AttributeValue("type");
				if (strcmp(sztype, "ang2") == 0)
				{
					Post::LineDataModel* lineData = new Post::LineDataModel(m_fem);
					Post::LineDataSource* src = new Post::Ang2LineDataSource(lineData);
					fsps_read_parameters(src, tag);
					src->UpdateData(true);
					src->Reload();
					linePlot->SetLineDataModel(lineData);
				}
			}
			else tag.skip();
		}
		else if (tag == "init_path")
		{
			Post::GLMusclePath* mp = dynamic_cast<Post::GLMusclePath*>(plot);
			if (mp)
			{
				vector<vec3d> path;
				++tag;
				do
				{
					vec3d v;
					tag.value(v);
					path.push_back(v);
					++tag;
				} while (!tag.isend());
				mp->SetInitPath(path);
			}
		}
		else if (tag == "muscle_path")
		{
			// This is obsolete, but we'll read it for now
			Post::GLPlotGroup* pg = dynamic_cast<Post::GLPlotGroup*>(plot);
			if (pg)
			{
				const char* szname = tag.AttributeValue("name");
				Post::GLMusclePath* mp = new Post::GLMusclePath();
				pg->AddPlot(mp);
				mp->SetName(szname);
				++tag;
				do
				{
					Param* p = mp->GetParam(tag.Name());
					if (p)
					{
						fsps_read_param(p, tag);
					}
					else if (tag == "init_path")
					{
						vector<vec3d> path;
						++tag;
						do
						{
							vec3d v;
							tag.value(v);
							path.push_back(v);
							++tag;
						} while (!tag.isend());
						mp->SetInitPath(path);
					}
					++tag;
				} while (!tag.isend());
			}
			else tag.skip();
		}
		else if (tag == "plot")
		{
			m_pg = dynamic_cast<Post::GLPlotGroup*>(plot);
			if (parse_plot(tag) == false) return false;
			if (dynamic_cast<Post::GLPlotGroup*>(plot)) m_pg = nullptr;
		}
		++tag;
	} while (!tag.isend());

	plot->UpdateData(true);

	return true;
}

bool PostSessionFileReader::parse_view(XMLTag& tag)
{
	CGView& view = *m_doc->GetView();

	++tag;
	do
	{
		if (tag == "viewpoint")
		{
			GLCameraTransform vp;
			quatd q = vp.rot;
			float w = q.GetAngle() * 180.f / PI;
			vec3d v = q.GetVector() * w;

			vec3d r = vp.pos;
			float d = vp.trg.z;

			const char* szname = tag.AttributeValue("name", true);
			string name;
			if (szname) name = szname;
			else
			{
				std::stringstream ss; ss << "ViewPoint" << view.CameraKeys();
				name = ss.str();
			}

			++tag;
			do {
				if      (tag == "x-angle") tag.value(v.x);
				else if (tag == "y-angle") tag.value(v.y);
				else if (tag == "z-angle") tag.value(v.z);
				else if (tag == "x-target") tag.value(r.z);
				else if (tag == "y-target") tag.value(r.y);
				else if (tag == "z-target") tag.value(r.z);
				else if (tag == "target_distance") tag.value(d);
				++tag;
			} 
			while (!tag.isend());

			w = PI * v.Length() / 180.f; v.Normalize();
			q = quatd(w, v);
			vp.rot = q;
			vp.pos = r;
			vp.trg.z = d;

			view.AddCameraKey(vp, name);
		}
		else return false;
		++tag;
	} while (!tag.isend());
	return true;
}

//=============================================================================
// helper function for writing parameters
void fsps_write_parameters(FSObject* po, XMLWriter& xml)
{
	if (po == nullptr) return;

	for (int i = 0; i < po->Parameters(); ++i)
	{
		Param& pi = po->GetParam(i);
		const char* sz = pi.GetShortName();

		XMLElement el(sz);
		switch (pi.GetParamType())
		{
		case Param_BOOL  : { bool    b = pi.GetBoolValue (); el.value(b); } break;
		case Param_INT   : { int     n = pi.GetIntValue  (); el.value(n); } break;
		case Param_CHOICE: { int     n = pi.GetIntValue  (); el.value(n); } break;
		case Param_FLOAT : { double  v = pi.GetFloatValue(); el.value(v); } break;
		case Param_VEC3D : { vec3d   v = pi.GetVec3dValue(); el.value(v); } break;
		case Param_COLOR : { GLColor c = pi.GetColorValue(); int v[3] = { c.r, c.g, c.b }; el.value(v, 3); } break;
		case Param_STRING: { string s = pi.GetStringValue(); el.value(s); } break;
		}

		xml.add_leaf(el);
	}
}

//-----------------------------------------------------------------------------
PostSessionFileWriter::PostSessionFileWriter(CPostDocument* doc) : m_doc(doc)
{
	m_xml = new XMLWriter;
}

PostSessionFileWriter::~PostSessionFileWriter()
{
	delete m_xml;
}

bool PostSessionFileWriter::Write(const char* szfile)
{
	if (m_doc == nullptr) return false;
	if (szfile == nullptr) return false;

	Post::FEPostModel* fem = m_doc->GetFSModel();
	if (fem == nullptr) return false;

	XMLWriter& xml = *m_xml;
	if (xml.open(szfile) == false) return false;
	m_fileName = szfile;

	XMLElement root("febiostudio_post_session");
	root.add_attribute("version", "2.0");
	xml.add_branch(root);
	{
		WriteModel();
		WriteMaterials();
		WriteDataFields();
		WriteMeshSelections();

		// save post model components
		Post::CGLModel* glm = m_doc->GetGLModel();
		if (glm)
		{
			WritePlots();
			WriteView();
		}
	}
	xml.close_branch(); // root

	xml.close();

	return true;
}

void PostSessionFileWriter::WriteModel()
{
	XMLWriter& xml = *m_xml;

	// we'll use this for converting to relative file paths.
	QFileInfo fi(QString::fromStdString(m_fileName));
	QDir currentDir(fi.absolutePath());

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
			std::string kineFile = currentDir.relativeFilePath(QString::fromStdString(kine->GetKineFile())).toStdString();

			XMLElement el("model");
			el.add_attribute("type", "kinemat");
			xml.add_branch(el);
			{
				xml.add_leaf("model_file", modelFile);
				xml.add_leaf("kine_file", kineFile);
				int n[3] = { kine->GetMin(), kine->GetMax(), kine->GetStep() };
				xml.add_leaf("range", n, 3);
			}
			xml.close_branch();
		}
		else if (openFileReader)
		{
			// save plot file
			std::string plotFile = currentDir.relativeFilePath(QString::fromStdString(openFileReader->GetFileName())).toStdString();
			XMLElement plt("model");
			plt.add_attribute("file", plotFile);
			xml.add_empty(plt);
		}
	}
	else if (dynamic_cast<FEKinematFileReader*>(reader))
	{
		FEKinematFileReader* kine = dynamic_cast<FEKinematFileReader*>(reader);

		// create absolute file names for model and kine files
		std::string modelFile = currentDir.relativeFilePath(QString::fromStdString(kine->GetModelFile())).toStdString();
		std::string kineFile = currentDir.relativeFilePath(QString::fromStdString(kine->GetKineFile())).toStdString();

		XMLElement el("model");
		el.add_attribute("type", "kinemat");
		xml.add_branch(el);
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
		XMLElement plt("model");
		plt.add_attribute("file", plotFile);
		xml.add_empty(plt);
	}
}

void PostSessionFileWriter::WriteMaterials()
{
	Post::FEPostModel* fem = m_doc->GetFSModel();
	XMLWriter& xml = *m_xml;

	// save material settings
	for (int i = 0; i < fem->Materials(); ++i)
	{
		Post::Material* mat = fem->GetMaterial(i);
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
}

void PostSessionFileWriter::WriteDataFields()
{
	XMLWriter& xml = *m_xml;

	// save data field settings
	Post::FEPostModel& fem = *m_doc->GetFSModel();
	Post::FEDataManager& dm = *fem.GetDataManager();
	for (int i = 0; i < dm.DataFields(); ++i)
	{
		Post::ModelDataField* data = *dm.DataField(i);
		if (data && (data->Parameters()))
		{
			XMLElement el("datafield");
			el.add_attribute("name", data->GetName());
			xml.add_branch(el);
			fsps_write_parameters(data, xml);
			xml.close_branch();
		}
	}

}

void PostSessionFileWriter::WriteMeshSelections()
{
	XMLWriter& xml = *m_xml;

	// save selections
	GObject* po = m_doc->GetActiveObject();
	if (po)
	{
		for (int i = 0; i < po->FENodeSets(); ++i)
		{
			FSNodeSet* pg = po->GetFENodeSet(i);

			XMLElement el("mesh:nodeset");
			el.add_attribute("name", pg->GetName());
			xml.add_branch(el);
			{
				std::vector<int> items = pg->CopyItems();
				std::vector<int>::iterator it = items.begin();
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
			FSEdgeSet* pg = po->GetFEEdgeSet(i);

			XMLElement el("mesh:edgeset");
			el.add_attribute("name", pg->GetName());
			xml.add_branch(el);
			{
				std::vector<int> items = pg->CopyItems();
				std::vector<int>::iterator it = items.begin();
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
			FSSurface* pg = po->GetFESurface(i);

			XMLElement el("mesh:surface");
			el.add_attribute("name", pg->GetName());
			xml.add_branch(el);
			{
				std::vector<int> items = pg->CopyItems();
				std::vector<int>::iterator it = items.begin();
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

			for (int i = 0; i < po->FEElemSets(); ++i)
			{
				FSElemSet* pg = po->GetFEElemSet(i);

			XMLElement el("mesh:elementset");
			el.add_attribute("name", pg->GetName());
			xml.add_branch(el);
			{
				std::vector<int> items = pg->CopyItems();
				std::vector<int>::iterator it = items.begin();
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
}

void PostSessionFileWriter::WritePlots()
{
	XMLWriter& xml = *m_xml;
	Post::CGLModel* glm = m_doc->GetGLModel();

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
			fsps_write_parameters(plot, xml);

			if (dynamic_cast<Post::CGLLinePlot*>(plot))
			{
				Post::CGLLinePlot* linePlot = dynamic_cast<Post::CGLLinePlot*>(plot);

				Post::LineDataModel* lineData = linePlot->GetLineDataModel();
				if (lineData)
				{
					Post::LineDataSource* src = lineData->GetLineDataSource();
					if (src)
					{
						const char* sztype = src->GetTypeString();
						XMLElement el("source");
						el.add_attribute("type", sztype);
						xml.add_branch(el);
						{
							fsps_write_parameters(src, xml);
						}
						xml.close_branch();
					}
				}
			}
			if (dynamic_cast<Post::GLMusclePath*>(plot))
			{
				Post::GLMusclePath* mp = dynamic_cast<Post::GLMusclePath*>(plot);
				if (mp->OverrideInitPath())
				{
					std::vector<vec3d> path = mp->GetInitPath();
					if (path.empty() == false)
						xml.add_branch("init_path");
					for (vec3d p : path)
					{
						xml.add_leaf("point", p);
					}
					xml.close_branch();
				}
			}
			if (dynamic_cast<Post::GLPlotGroup*>(plot))
			{
				Post::GLPlotGroup* pg = dynamic_cast<Post::GLPlotGroup*>(plot);
				for (int n = 0; n < pg->Plots(); ++n)
				{
					Post::CGLPlot* pn = pg->GetPlot(n);

					string typeStr_n = pn->GetTypeString();
					string name_n = pn->GetName();
					XMLElement el("plot");
					el.add_attribute("type", typeStr_n);
					if (name_n.empty() == false) el.add_attribute("name", name_n);
					xml.add_branch(el);
					{
						fsps_write_parameters(pn, xml);

						Post::GLMusclePath* mp = dynamic_cast<Post::GLMusclePath*>(pn);
						if (mp && mp->OverrideInitPath())
						{
							std::vector<vec3d> path = mp->GetInitPath();
							if (path.empty() == false)
								xml.add_branch("init_path");
							for (vec3d p : path)
							{
								xml.add_leaf("point", p);
							}
							xml.close_branch();
						}
					}
					xml.close_branch();
				}
			}
		}
		xml.close_branch();
	}
}

void PostSessionFileWriter::WriteView()
{
	XMLWriter& xml = *m_xml;
	Post::CGLModel* glm = m_doc->GetGLModel();

	CGView& view = *m_doc->GetView();
	if (view.CameraKeys() > 0)
	{
		xml.add_branch("view");
		{
			for (int i = 0; i < view.CameraKeys(); ++i)
			{
				CGViewKey& vp = view.GetKey(i);

				quatd q = vp.transform.rot;
				float w = q.GetAngle() * 180.f / PI;
				vec3d v = q.GetVector() * w;
				vec3d r = vp.transform.pos;
				float d = vp.transform.trg.z;

				XMLElement el("viewpoint");
				el.add_attribute("name", vp.GetName());
				xml.add_branch(el);
				{
					xml.add_leaf("x-angle", v.x);
					xml.add_leaf("y-angle", v.y);
					xml.add_leaf("z-angle", v.z);
					xml.add_leaf("x-target", r.x);
					xml.add_leaf("y-target", r.y);
					xml.add_leaf("z-target", r.z);
					xml.add_leaf("target_distance", d);
				}
				xml.close_branch();
			}
		}
		xml.close_branch();
	}
}
