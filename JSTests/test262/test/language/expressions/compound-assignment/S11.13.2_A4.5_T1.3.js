// Copyright 2009 the Sputnik authors.  All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
info: The production x -= y is the same as x = x - y
es5id: 11.13.2_A4.5_T1.3
description: Type(x) and Type(y) vary between primitive string and String object
---*/

var x;

//CHECK#1
x = "1";
x -= "1";
if (x !== 0) {
  throw new Test262Error('#1: x = "1"; x -= "1"; x === 0. Actual: ' + (x));
}

//CHECK#2
x = new String("1");
x -= "1";
if (x !== 0) {
  throw new Test262Error('#2: x = new String("1"); x -= "1"; x === 0. Actual: ' + (x));
}

//CHECK#3
x = "1";
x -= new String("1");
if (x !== 0) {
  throw new Test262Error('#3: x = "1"; x -= new String("1"); x === 0. Actual: ' + (x));
}

//CHECK#4
x = new String("1");
x -= new String("1");
if (x !== 0) {
  throw new Test262Error('#4: x = new String("1"); x -= new String("1"); x === 0. Actual: ' + (x));
}

//CHECK#5
x = "x";
x -= "1";
if (isNaN(x) !== true) {
  throw new Test262Error('#5: x = "x"; x -= "1"; x === Not-a-Number. Actual: ' + (x));
}

//CHECK#6
x = "1";
x -= "x";
if (isNaN(x) !== true) {
  throw new Test262Error('#6: x = "1"; x -= "x"; x === Not-a-Number. Actual: ' + (x));
}
