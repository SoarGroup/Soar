

// return a transformation describing how the from FOR relates to the to FOR in
// the FOR of the frame
//
// if frame==from, this returns the local transformation from from to to
// if frame==identity, it is the global transformation from from to to
Transformation relateInFrame(Transformation from, Transformation to, Transformation frame);
// this can be copied in from SVS, it is SVSTransformationBuilder::buildInFOR
