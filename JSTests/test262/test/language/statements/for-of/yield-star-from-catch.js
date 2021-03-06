// Copyright (C) 2015 the V8 project authors. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.
/*---
es6id: 13.6.4.13
description: >
    Control flow during body evaluation should honor `yield *` statements
    within the `catch` block of `try` statements.
features: [generators]
---*/

function* values() {
  yield 1;
  yield 1;
}
var dataIterator = values();
var controlIterator = (function*() {
  for (var x of dataIterator) {
    try {
      throw new Error();
      throw new Test262Error('This code is unreachable.');
    } catch (err) {
      i++;
      yield * values();
      j++;
    }
    k++;
  }

  l++;
})();
var i = 0;
var j = 0;
var k = 0;
var l = 0;

controlIterator.next();
assert.sameValue(i, 1, 'First iteration: pre-yield');
assert.sameValue(j, 0, 'First iteration: post-yield');
assert.sameValue(k, 0, 'First iteration: post-try');
assert.sameValue(l, 0, 'First iteration: post-for-of');

controlIterator.next();
assert.sameValue(i, 1, 'Second iteration: pre-yield');
assert.sameValue(j, 0, 'Second iteration: post-yield');
assert.sameValue(k, 0, 'Second iteration: post-try');
assert.sameValue(l, 0, 'Second iteration: post-for-of');

controlIterator.next();
assert.sameValue(i, 2, 'Third iteration: pre-yield');
assert.sameValue(j, 1, 'Third iteration: post-yield');
assert.sameValue(k, 1, 'Third iteration: post-try');
assert.sameValue(l, 0, 'Third iteration: post-for-of');

controlIterator.next();
assert.sameValue(i, 2, 'Fourth iteration: pre-yield');
assert.sameValue(j, 1, 'Fourth iteration: post-yield');
assert.sameValue(k, 1, 'Fourth iteration: post-try');
assert.sameValue(l, 0, 'Fourth iteration: post-for-of');

controlIterator.next();
assert.sameValue(i, 2, 'Fifth iteration: pre-yield');
assert.sameValue(j, 2, 'Fifth iteration: post-yield');
assert.sameValue(k, 2, 'Fifth iteration: post-try');
assert.sameValue(l, 1, 'Fifth iteration: post-for-of');
