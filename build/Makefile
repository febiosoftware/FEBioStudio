QMAKE = /home/sci/mherron/Resources/Qt/5.12.0/gcc_64/bin/qmake

LIBS = Ansys Abaqus Comsol FEBio FEMLib FSCore GeomLib ImageLib LSDyna MathLib MeshIO MeshLib XML XPLTLib

FEBioStudio: $(LIBS) MeshTools GLWLib PostGL PostLib
	cd ../FEBioStudio; $(QMAKE) FEBioStudio.pro;
	$(MAKE) -C ../FEBioStudio/;

$(LIBS):
	cd ../$@;	$(MAKE) -f ../build/makeLibs.mk
	
MeshTools:
	cd ../MeshTools; $(QMAKE) MeshTools.pro;
	$(MAKE) -C ../MeshTools/;
	
GLWLib:
	cd ../GLWLib; $(QMAKE) GLWLib.pro;
	$(MAKE) -C ../GLWLib/;

PostGL:
	cd ../PostGL; $(QMAKE) PostGL.pro;
	$(MAKE) -C ../PostGL/;

PostLib:
	cd ../PostLib; $(QMAKE) PostLib.pro;
	$(MAKE) -C ../PostLib/;

FEBioStudioClean:
	cd ../FEBioStudio; rm moc*; $(MAKE) distclean

MeshToolsClean:
	cd ../MeshTools; rm moc*; $(MAKE) distclean
	
GLWLibClean:
	cd ../GLWLib; rm moc*; $(MAKE) distclean

PostGLClean:
	cd ../PostGL; rm moc*; $(MAKE) distclean

PostLibClean:
	cd ../PostLib; rm moc*; $(MAKE) distclean

clean: MocClean
	for lib in $(LIBS) ; do \
		cd ../$$lib; $(MAKE) -f ../build/makeLibs.mk clean ;\
	done

	cd ../MeshTools/; $(MAKE) distclean
	cd ../GLWLib/; $(MAKE) distclean
	cd ../PostGL/; $(MAKE) distclean
	cd ../PostLib/; $(MAKE) distclean
	cd ../FEBioStudio/; $(MAKE) distclean
	
MocClean:
	cd ../; rm */moc*;
