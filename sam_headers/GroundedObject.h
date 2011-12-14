
// A GroundedObject is a shape that has a definite location in a coordinate system,
// like the spatial scene. GroundedObject are implicit in the scene graph, this is
// an explicit representation. These are cached in SVSObjects for efficiency.

// This is a base class that will get specialized for each interpretation:
// point2/3,bbox2/3,poly2/3

// I'm assuming for now that local geometry is never needed (i.e., access to
// CGAL structures where the points are the points in the local FOR as the
// scene graph stores). If that is needed, copy the polyhedron and
// transform by the inverse of Wm3 global transform.
//
// In SVS2, all objects will have the local FOR origin at their centroid. This will require
// some preprocessing of imported models before they can be used, but
// simplifies things internally.

class GroundedObject {  
  public:

    // ALL of these are pure virtual:
    
    // constructor taking a SVSObject*: this poly will be the convex hull of the
    // scene graph below the given node, at the global location.
    virtual GroundedObject(SVSObject* shape) = 0;

    // constructor taking a (generic) GroundedObject
    virtual GroundedObject(GroundedObject*) = 0;
    // The last will be used, e.g., to get a centroid:
    // GroundedObjectPoint3 centroid(Some3dPolyhedronGroundedObject);
    
    // in addition, each derived must have its own constructor taking, E.g., a
    // CGALPolygon2 for the 2d polygon object class
    

    // The GroundedObject must contain all info necessary to build an
    // SVSObject. This means it must be able to determine the
    // local coordinate frame of the object once it is placed in the scene
    // graph. To simplify this, the origin of the local frame is always the
    // centroid of the object (which is implicit in its grounded coordinates).
    // The frontOffsetPoint summarizes the rest of the transform (rotation and
    // scaling). 
    //
    // The frontOffsetPoint, when added to the centroid of the shape, produces
    // the point that will be considered one unit in front of the centroid in
    // its local coordinate frame in the scene graph (local coordinate 0,1,0).
    //
    // It can be any value other than the centroid. If it is unimportant, the
    // value 0,1,0 allows the simplest computations.
    //
    // This point not only allows the GroundedObject to be converted back to an
    // SVSObject, but also allows processes that access only GroundedObjects to
    // know what the front of a given object is and manipulate that.
    //
    virtual CGALPoint getCentroid() = 0;
    virtual CGALPoint getFrontOffsetPoint() = 0;
    virtual void setFrontOffsetPoint(CGALPoint point) = 0;


    // Build a WM3 geometry object, where the coordinates are in the FOR where
    // 0,0,0 is the centroid and 0,1,0 points to the front.
    // Caller owns the new object.
    virtual Geometry* makeNewGeometry() = 0;

    // Get a WM3 transformation, going from the global FOR to place the
    // object at its grounded location.
    virtual Transformation getGlobalTransformation();
    
    // Move the shape through space
    // This is used to efficiently update grounded shapes (e.g., do not recalculate the
    // convex hull of the Wm3 structure at every movement).
    // To apply this, apply the inverse of the global transformation to each
    // point, then apply newTx to each.
    virtual void setGlobalTransformation(Transformation newTx) = 0;

  private:
    CGALPoint frontOffsetPoint;
    
    // again, the globalTransform is implicit in the centroid and the
    // frontOffsetPoint, but it should probably be cached, since
    // setGlobalTransformation() will probably be called often.
    Transformation globalTransform;

};

struct GroundedObjectSet {
  GroundedObjectPolyhedron3d* poly3;
  GroundedObjectPolygon2d* poly2;
  GroundedObjectBoundingBox3* bbox3;
  GroundedObjectBoundingBox2* bbox2;
  GroundedObjectPoint3* point3;
  GroundedObjectPoint2* point2;
  int minimumGoalLevel;
};


// TODO: fill these all in
// they should all cache internal state and update when transform() is called

class GroundedObjectPolyhedron3d : public GroundedObject {
};
class GroundedObjectPolygon2d : public GroundedObject {
};
class GroundedObjectBoundingBox3d : public GroundedObject {
};
class GroundedObjectBoundingBox2d : public GroundedObject {
};
class GroundedObjectPoint3d : public GroundedObject {
};
class GroundedObjectPoint2d : public GroundedObject {
};
