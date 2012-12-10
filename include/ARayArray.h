// $Id: ARayArray.h 3 2010-11-26 17:17:31Z oxon $
// Author: Akira Okumura 2007/10/02

/******************************************************************************
 * Copyright (C) 2006-, Akira Okumura                                         *
 * All rights reserved.                                                       *
 *****************************************************************************/

#ifndef A_RAY_ARRAY_H
#define A_RAY_ARRAY_H

///////////////////////////////////////////////////////////////////////////////
//
// ARayArray
//
// Array of ARay
//
///////////////////////////////////////////////////////////////////////////////

#ifndef A_RAY_H
#include "ARay.h"
#endif
#ifndef ROOT_TObjArray
#include "TObjArray.h"
#endif

class ARayArray : public TObject {
 private:
  TObjArray fAbsorbed;  // Array of absorbed rays
  TObjArray fExited;    // Array of exited rays
  TObjArray fFocused;   // Array of focused rays
  TObjArray fRunning;   // Array of running rays
  TObjArray fStopped;   // Array of stopped rays
  TObjArray fSuspended; // Array of suspended rays
  
 public:
  ARayArray();
  virtual ~ARayArray();

  virtual void       Add(ARay* ray);
  virtual TObjArray* GetAbsorbed() { return &fAbsorbed;};
  virtual TObjArray* GetExited() { return &fExited;};
  virtual TObjArray* GetFocused() { return &fFocused;};
  virtual TObjArray* GetRunning() { return &fRunning;};
  virtual TObjArray* GetStopped() { return &fStopped;};
  virtual TObjArray* GetSuspended() { return &fSuspended;};
  virtual void       Merge(ARayArray* array);

  ClassDef(ARayArray, 1)
};

#endif // A_RAY_ARRAY_H
