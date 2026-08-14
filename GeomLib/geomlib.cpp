/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2021 University of Utah, The Trustees of Columbia University in
the City of New York, and others.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

#include "geomlib.h"
#include <stdio.h>
#include <math.h>
#include <FSCore/ClassDescriptor.h>
#include "GPrimitive.h"
#include "GFoamObject.h"
#include "GOCCObject.h"

void InitGeomLib()
{
	static bool binit = false;
	if (binit) return;
	binit = true;

	REGISTER_CLASS2(GBox               , CLASS_OBJECT, "box"            , ":/icons/box.png"          , 0);
	REGISTER_CLASS2(GCylinder          , CLASS_OBJECT, "cylinder"       , ":/icons/cylinder.png"     , 0);
	REGISTER_CLASS2(GTube              , CLASS_OBJECT, "tube"           , ":/icons/tube.png"         , 0);
	REGISTER_CLASS2(GSphere            , CLASS_OBJECT, "sphere"         , ":/icons/sphere.png"       , 0);
	REGISTER_CLASS2(GCone              , CLASS_OBJECT, "cone"           , ":/icons/cone.png"         , 0);
	REGISTER_CLASS2(GTruncatedEllipsoid, CLASS_OBJECT, "ellipsoid"      , ":/icons/ellipsoid.png"    , 0);
	REGISTER_CLASS2(GTorus             , CLASS_OBJECT, "torus"          , ":/icons/torus.png"        , 0);
	REGISTER_CLASS2(GSlice             , CLASS_OBJECT, "slice"          , ":/icons/slice.png"        , 0);
	REGISTER_CLASS2(GSolidArc          , CLASS_OBJECT, "solid_arc"      , ":/icons/solidarc.png"     , 0);
	REGISTER_CLASS2(GHexagon           , CLASS_OBJECT, "hexagon"        , ":/icons/hexagon.png"      , 0);
	REGISTER_CLASS2(GQuartDogBone      , CLASS_OBJECT, "dog_bone"       , ":/icons/dogbone.png"      , 0);
	REGISTER_CLASS2(GCylinderInBox     , CLASS_OBJECT, "cylinder_in_box", ":/icons/cylinderinbox.png", 0);
	REGISTER_CLASS2(GSphereInBox       , CLASS_OBJECT, "sphere_in_box"  , ":/icons/sphereinbox.png"  , 0);
	REGISTER_CLASS2(GHollowSphere      , CLASS_OBJECT, "hollow_sphere"  , ":/icons/hollowsphere.png" , 0);
	REGISTER_CLASS2(GBoxInBox          , CLASS_OBJECT, "box_in_box"     , ":/icons/boxinbox.png"     , 0);
	REGISTER_CLASS2(GThinTube          , CLASS_OBJECT, "thin_tube"      , ":/icons/thintube.png"     , 0);
	REGISTER_CLASS2(GPatch             , CLASS_OBJECT, "patch"          , ":/icons/square.png"       , 0);
	REGISTER_CLASS2(GDisc              , CLASS_OBJECT, "disc"           , ":/icons/disc.png"         , 0);
	REGISTER_CLASS2(GRing              , CLASS_OBJECT, "ring"           , ":/icons/ring.png"         , 0);

#ifndef NDEBUG
	REGISTER_CLASS2(GFoamObject, CLASS_OBJECT, "foam", ":/icons/foam.png", 0);
#endif

#ifndef NDEBUG
	REGISTER_CLASS2(GCylindricalPatch  , CLASS_OBJECT, "cylindrical_patch", ":/icons/cylpatch.png"     , 0);

#ifdef HAS_OCC
	REGISTER_CLASS2(GOCCBottle         , CLASS_OBJECT, "bottle"    , ":/icons/bottle.png", 0);
	REGISTER_CLASS2(GOCCBox            , CLASS_OBJECT, "occ_box"   , ":/icons/box.png", 0);
#endif

#endif // NDEBUG
}
