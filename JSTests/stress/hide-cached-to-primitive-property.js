function shouldBe(actual, expected) {
    if (actual !== expected)
        throw new Error('bad value: ' + actual);
}

var object = {};
object.__proto__ = {};
shouldBe(object + 42, `[object Object]42`);
shouldBe(object + 42, `[object Object]42`);
object.__proto__[Symbol.toPrimitive] = function () {
    return 42;
};
shouldBe(object + 42, 84);
