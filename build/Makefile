QMAKE = /home/sci/mherron/Resources/Qt/5.13.2/gcc_64/bin/qmake

LIBS = Ansys Abaqus Comsol FEBio FEMLib FSCore GeomLib ImageLib LSDyna MathLib MeshIO MeshLib Nike3D XML XPLTLib

FEBioStudio: $(LIBS) MeshTools GLLib GLWLib PostGL PostLib CUILib
	cd ../FEBioStudio; $(QMAKE) FEBioStudio.pro;
#	$(MAKE) FEBIOStudioQMake
	$(MAKE) -C ../FEBioStudio/;

#FEBIOStudioQMake: FEBioStudio.pro
#	cd ../FEBioStudio; rm moc*; $(MAKE) distclean
#	cd ../FEBioStudio; $(QMAKE) FEBioStudio.pro;

$(LIBS):
	cd ../$@;	$(MAKE) -f ../build/makeLibs.mk
	
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
	cd ../FEBioStudio; rm moc*; $(MAKE) distclean

MeshToolsClean:
	cd ../MeshTools; rm moc*; $(MAKE) distclean

GLLibClean:
	cd ../GLLib; rm moc*; $(MAKE) distclean
	
GLWLibClean:
	cd ../GLWLib; rm moc*; $(MAKE) distclean

PostGLClean:
	cd ../PostGL; rm moc*; $(MAKE) distclean

PostLibClean:
	cd ../PostLib; rm moc*; $(MAKE) distclean

CUILibClean:
	cd ../CUILib; rm moc*; $(MAKE) distclean

clean: #MocClean
	for lib in $(LIBS) ; do \
		cd ../$$lib; $(MAKE) -f ../build/makeLibs.mk clean ;\
	done

	cd ../MeshTools/; $(MAKE) distclean
	cd ../GLLib/; $(MAKE) distclean
	cd ../GLWLib/; $(MAKE) distclean
	cd ../PostGL/; $(MAKE) distclean
	cd ../PostLib/; $(MAKE) distclean
	cd ../CUILib/; $(MAKE) distclean
	cd ../FEBioStudio/; $(MAKE) distclean
	
MocClean:
	cd ../; rm */moc*;
