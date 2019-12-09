QMAKE = /home/sci/mherron/Resources/Qt/5.13.2/gcc_64/bin/qmake

LIBS = Ansys Abaqus Comsol FEBio FEMLib FSCore GeomLib ImageLib LSDyna MathLib MeshIO MeshLib Nike3D XML XPLTLib

FEBioStudio: $(LIBS) MeshTools GLLib GLWLib PostGL PostLib CUILib
	cd ../FEBioStudio; $(QMAKE) FEBioStudio.pro;
	$(MAKE) -C ../FEBioStudio/;

$(LIBS):
	cd ../$@; $(MAKE) -f ../build/makeLibs.mk
	
MeshTools:
	cd ../MeshTools; $(QMAKE) MeshTools.pro;
	$(MAKE) -C ../MeshTools/;

GLLib:
	cd ../GLLib; $(QMAKE) GLLib.pro;
	$(MAKE) -C ../GLLib/;
	
GLWLib:
	cd ../GLWLib; $(QMAKE) GLWLib.pro;
	$(MAKE) -C ../GLWLib/;

PostGL:
	cd ../PostGL; $(QMAKE) PostGL.pro;
	$(MAKE) -C ../PostGL/;

PostLib:
	cd ../PostLib; $(QMAKE) PostLib.pro;
	$(MAKE) -C ../PostLib/;

CUILib:
	cd ../CUILib; $(QMAKE) CUILib.pro;
	$(MAKE) -C ../CUILib/;

FEBioStudioClean:
	cd ../FEBioStudio; $(MAKE) clean

MeshToolsClean:
	cd ../MeshTools; $(MAKE) clean

GLLibClean:
	cd ../GLLib; $(MAKE) clean
	
GLWLibClean:
	cd ../GLWLib; $(MAKE) clean

PostGLClean:
	cd ../PostGL; $(MAKE) clean

PostLibClean:
	cd ../PostLib; $(MAKE) clean

CUILibClean:
	cd ../CUILib; $(MAKE) clean

LibClean:
	for lib in $(LIBS) ; do \
		cd ../$$lib; $(MAKE) -f ../build/makeLibs.mk clean ;\
	done

clean: LibClean MeshToolsClean GLLibClean GLWLibClean PostGLClean PostLibClean CUILibClean FEBioStudioClean
