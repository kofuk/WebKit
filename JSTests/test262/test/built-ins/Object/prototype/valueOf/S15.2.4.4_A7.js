// Copyright 2009 the Sputnik authors.  All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
info: Object.prototype.valueOf can't be used as a constructor
es5id: 15.2.4.4_A7
description: Checking if creating "new Object.prototype.valueOf" fails
---*/

assert.throws(TypeError, function() {
  new Object.prototype.valueOf();
}, '`new Object.prototype.valueOf()` throws a TypeError exception');
