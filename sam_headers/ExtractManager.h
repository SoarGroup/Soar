
// This singleton class handles connecting extract command watchers to the
// functions that actually do the geometry processing
//
// This should probably all be static. Some internal state is needed to
// associate strings to function ptrs.

// This helps insulate qualitative/SVSObject representations in the system from
// geometric representations.

class ExtractManager {
public:
  // this simply looks up the appropriate function based on the type, and calls
  // it with the parameters
  Result extract(string type, ParameterSet parameters);

private:
  // table of relationships to extractor functions goes here
};

// example extraction header

Result rccDR(ParameterSet& parameters);
             // should contain two object parameters

// extractors get parameters where the objects are ungrounded, but can call
// parameters.ground() immediately if they only want to deal with geometry
//
// some extractors can benefit from ungrounded SVSObject parameters, since they
// can call SVSObject::possiblyCollidesWith() to solve simple cases
