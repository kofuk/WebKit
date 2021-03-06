// Copyright 2009 the Sputnik authors.  All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
info: |
    When String is called as a function rather than as a constructor, it
    performs a type conversion
es5id: 15.5.1.1_A1_T14
description: Call String(0) and String(-0)
---*/

var __str = String(0);

//////////////////////////////////////////////////////////////////////////////
//CHECK#1
if (typeof __str !== "string") {
  throw new Test262Error('#1: __str = String(0); typeof __str === "string". Actual: typeof __str ===' + typeof __str);
}
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//CHECK#2
if (__str !== "0") {
  throw new Test262Error('#2: __str = String(0); __str === "0". Actual: __str ===' + __str);
}
//
//////////////////////////////////////////////////////////////////////////////

__str = String(-0);

//////////////////////////////////////////////////////////////////////////////
//CHECK#1
if (typeof __str !== "string") {
  throw new Test262Error('#3: __str = String(-0); typeof __str === "string". Actual: typeof __str ===' + typeof __str);
}
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//CHECK#2
if (__str !== "0") {
  throw new Test262Error('#4: __str = String(-0); __str === "0". Actual: __str ===' + __str);
}
//
//////////////////////////////////////////////////////////////////////////////
