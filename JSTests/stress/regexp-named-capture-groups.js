
function shouldBe(actual, expected) {
    if (actual !== expected && !(actual !== null && typeof(expected) === "string" && actual.toString() == expected))
        throw new Error('expected: ' + expected + ', bad value: ' + actual);
}

function shouldThrowInvalidGroupSpecifierName(func) {
    var error = null;
    try {
        func();
    } catch (e) {
        error = e;
    }
    if (!error)
        throw new Error('not thrown');
    shouldBe(String(error), "SyntaxError: Invalid regular expression: invalid group specifier name");
}

/*
 Valid ID_Start / ID_Continue Unicode characters

 π  \u{1d453}  \ud835 \udc53
 π  \u{1d45c}  \ud835 \udc5c
 π₯  \u{id465}  \ud835 \udc65

 π  \u{1d4d3}  \ud835 \udcd3
 πΈ  \u{1d4f8}  \ud835 \udcf8
 π°  \u{1d4f0}  \ud835 \udcf0

 π  \u{1d4d1}  \ud835 \udcd1
 π»  \u{1d4fb}  \ud835 \udcfb
 πΈ  \u{1d4f8}  \ud835 \udcf8
 π  \u{1d500}  \ud835 \udd00
 π·  \u{1d4f7}  \ud835 \udcf7

 π°  \u{1d5b0}  \ud835 \uddb0
 π‘  \u{1d5a1}  \ud835 \udda1
 π₯  \u{1d5a5}  \ud835 \udda5

 (fox) ηΈ  \u{72f8}  \u72f8
 (dog) η  \u{72d7}  \u72d7  

 Valid ID_Continue Unicode characters (Can't be first identifier character.)

 π  \u{1d7da}  \ud835 \udfda

Invalid ID_Start / ID_Continue

 (fox face emoji) π¦  \u{1f98a}  \ud83e \udd8a
 (dog emoji)  π  \u{1f415}  \ud83d \udc15
*/

var string = "The quick brown fox jumped over the lazy dog's back";
var string2 = "It is a dog eat dog world.";

let match = null;

// Try valid names

// Unicode RegExp's
shouldBe(string.match(/(?<animal>fox|dog)/u).groups.animal, "fox");
shouldBe(string.match(/(?<the2>the)/u).groups.the2, "the");

match = string.match(/(?<πππ₯>fox).*(?<ππΈπ°>dog)/u);
shouldBe(match.groups.πππ₯, "fox");
shouldBe(match.groups.ππΈπ°, "dog");
shouldBe(match[1], "fox");
shouldBe(match[2], "dog");

match = string.match(/(?<ηΈ>fox).*(?<η>dog)/u);
shouldBe(match.groups.ηΈ, "fox");
shouldBe(match.groups.η, "dog");
shouldBe(match[1], "fox");
shouldBe(match[2], "dog");

shouldBe(string.match(/(?<ππ»πΈππ·>brown)/u).groups.ππ»πΈππ·, "brown");
shouldBe(string.match(/(?<ππ»πΈππ·>brown)/u).groups.\u{1d4d1}\u{1d4fb}\u{1d4f8}\u{1d500}\u{1d4f7}, "brown");
shouldBe(string.match(/(?<\u{1d4d1}\u{1d4fb}\u{1d4f8}\u{1d500}\u{1d4f7}>brown)/u).groups.ππ»πΈππ·, "brown");
shouldBe(string.match(/(?<\u{1d4d1}\u{1d4fb}\u{1d4f8}\u{1d500}\u{1d4f7}>brown)/u).groups.\u{1d4d1}\u{1d4fb}\u{1d4f8}\u{1d500}\u{1d4f7}, "brown");
shouldBe(string.match(/(?<\ud835\udcd1\ud835\udcfb\ud835\udcf8\ud835\udd00\ud835\udcf7>brown)/u).groups.ππ»πΈππ·, "brown");
shouldBe(string.match(/(?<\ud835\udcd1\ud835\udcfb\ud835\udcf8\ud835\udd00\ud835\udcf7>brown)/u).groups.\u{1d4d1}\u{1d4fb}\u{1d4f8}\u{1d500}\u{1d4f7}, "brown");

shouldBe(string.match(/(?<π°π‘π₯>q\w*\W\w*\W\w*)/u).groups.π°π‘π₯, "quick brown fox");
shouldBe(string.match(/(?<π°π‘\u{1d5a5}>q\w*\W\w*\W\w*)/u).groups.π°π‘π₯, "quick brown fox");
shouldBe(string.match(/(?<π°\u{1d5a1}π₯>q\w*\W\w*\W\w*)/u).groups.π°π‘π₯, "quick brown fox");
shouldBe(string.match(/(?<π°\u{1d5a1}\u{1d5a5}>q\w*\W\w*\W\w*)/u).groups.π°π‘π₯, "quick brown fox");
shouldBe(string.match(/(?<\u{1d5b0}π‘π₯>q\w*\W\w*\W\w*)/u).groups.π°π‘π₯, "quick brown fox");
shouldBe(string.match(/(?<\u{1d5b0}π‘\u{1d5a5}>q\w*\W\w*\W\w*)/u).groups.π°π‘π₯, "quick brown fox");
shouldBe(string.match(/(?<\u{1d5b0}\u{1d5a1}π₯>q\w*\W\w*\W\w*)/u).groups.π°π‘π₯, "quick brown fox");
shouldBe(string.match(/(?<\u{1d5b0}\u{1d5a1}\u{1d5a5}>q\w*\W\w*\W\w*)/u).groups.π°π‘π₯, "quick brown fox");

shouldBe(string.match(/(?<theπ>the)/u).groups.theπ, "the");
shouldBe(string.match(/(?<the\u{1d7da}>the)/u).groups.theπ, "the");
shouldBe(string.match(/(?<the\ud835\udfda>the)/u).groups.theπ, "the");

match = string2.match(/(?<dog>dog)(.*?)(\k<dog>)/u);
shouldBe(match.groups.dog, "dog");
shouldBe(match[1], "dog");
shouldBe(match[2], " eat ");
shouldBe(match[3], "dog");

match = string2.match(/(?<ππΈπ°>dog)(.*?)(\k<ππΈπ°>)/u);
shouldBe(match.groups.ππΈπ°, "dog");
shouldBe(match[1], "dog");
shouldBe(match[2], " eat ");
shouldBe(match[3], "dog");

match = string2.match(/(?<η>dog)(.*?)(\k<η>)/u);
shouldBe(match.groups.η, "dog");
shouldBe(match[1], "dog");
shouldBe(match[2], " eat ");
shouldBe(match[3], "dog");

// Non-unicode RegExp's
shouldBe(string.match(/(?<animal>fox|dog)/).groups.animal, "fox");

match = string.match(/(?<πππ₯>fox).*(?<ππΈπ°>dog)/);
shouldBe(match.groups.πππ₯, "fox");
shouldBe(match.groups.ππΈπ°, "dog");
shouldBe(match[1], "fox");
shouldBe(match[2], "dog");

match = string.match(/(?<ηΈ>fox).*(?<η>dog)/);
shouldBe(match.groups.ηΈ, "fox");
shouldBe(match.groups.η, "dog");
shouldBe(match[1], "fox");
shouldBe(match[2], "dog");

shouldBe(string.match(/(?<ππ»πΈππ·>brown)/).groups.ππ»πΈππ·, "brown");
shouldBe(string.match(/(?<ππ»πΈππ·>brown)/).groups.\u{1d4d1}\u{1d4fb}\u{1d4f8}\u{1d500}\u{1d4f7}, "brown");
shouldBe(string.match(/(?<\u{1d4d1}\u{1d4fb}\u{1d4f8}\u{1d500}\u{1d4f7}>brown)/).groups.ππ»πΈππ·, "brown");
shouldBe(string.match(/(?<\u{1d4d1}\u{1d4fb}\u{1d4f8}\u{1d500}\u{1d4f7}>brown)/).groups.\u{1d4d1}\u{1d4fb}\u{1d4f8}\u{1d500}\u{1d4f7}, "brown");
shouldBe(string.match(/(?<\ud835\udcd1\ud835\udcfb\ud835\udcf8\ud835\udd00\ud835\udcf7>brown)/).groups.ππ»πΈππ·, "brown");
shouldBe(string.match(/(?<\ud835\udcd1\ud835\udcfb\ud835\udcf8\ud835\udd00\ud835\udcf7>brown)/).groups.\u{1d4d1}\u{1d4fb}\u{1d4f8}\u{1d500}\u{1d4f7}, "brown");

shouldBe(string.match(/(?<π°π‘π₯>q\w*\W\w*\W\w*)/).groups.π°π‘π₯, "quick brown fox");
shouldBe(string.match(/(?<π°π‘\u{1d5a5}>q\w*\W\w*\W\w*)/).groups.π°π‘π₯, "quick brown fox");
shouldBe(string.match(/(?<π°\u{1d5a1}π₯>q\w*\W\w*\W\w*)/).groups.π°π‘π₯, "quick brown fox");
shouldBe(string.match(/(?<π°\u{1d5a1}\u{1d5a5}>q\w*\W\w*\W\w*)/).groups.π°π‘π₯, "quick brown fox");
shouldBe(string.match(/(?<\u{1d5b0}π‘π₯>q\w*\W\w*\W\w*)/).groups.π°π‘π₯, "quick brown fox");
shouldBe(string.match(/(?<\u{1d5b0}π‘\u{1d5a5}>q\w*\W\w*\W\w*)/).groups.π°π‘π₯, "quick brown fox");
shouldBe(string.match(/(?<\u{1d5b0}\u{1d5a1}π₯>q\w*\W\w*\W\w*)/).groups.π°π‘π₯, "quick brown fox");
shouldBe(string.match(/(?<\u{1d5b0}\u{1d5a1}\u{1d5a5}>q\w*\W\w*\W\w*)/).groups.π°π‘π₯, "quick brown fox");

shouldBe(string.match(/(?<theπ>the)/).groups.theπ, "the");
shouldBe(string.match(/(?<the\u{1d7da}>the)/).groups.theπ, "the");
shouldBe(string.match(/(?<the\ud835\udfda>the)/).groups.theπ, "the");

match = string2.match(/(?<dog>dog)(.*?)(\k<dog>)/);
shouldBe(match.groups.dog, "dog");
shouldBe(match[1], "dog");
shouldBe(match[2], " eat ");
shouldBe(match[3], "dog");

match = string2.match(/(?<ππΈπ°>dog)(.*?)(\k<ππΈπ°>)/);
shouldBe(match.groups.ππΈπ°, "dog");
shouldBe(match[1], "dog");
shouldBe(match[2], " eat ");
shouldBe(match[3], "dog");

match = string2.match(/(?<η>dog)(.*?)(\k<η>)/);
shouldBe(match.groups.η, "dog");
shouldBe(match[1], "dog");
shouldBe(match[2], " eat ");
shouldBe(match[3], "dog");

// Invalid identifiers

shouldThrowInvalidGroupSpecifierName(function() {
    return new RegExp("(?<π¦>fox)");
});

shouldThrowInvalidGroupSpecifierName(function() {
    return new RegExp("(?<\u{1f98a}>fox)");
});

shouldThrowInvalidGroupSpecifierName(function() {
    return new RegExp("(?<\ud83e\udd8a>fox)");
});

shouldThrowInvalidGroupSpecifierName(function() {
    return new RegExp("(?<π>dog)");
});

shouldThrowInvalidGroupSpecifierName(function() {
    return new RegExp("(?<\u{1f415}>dog)");
});

shouldThrowInvalidGroupSpecifierName(function() {
    return new RegExp("(?<\ud83d \udc15>dog)");
});

shouldThrowInvalidGroupSpecifierName(function() {
    return new RegExp("(?<πthe>the)");
});

shouldThrowInvalidGroupSpecifierName(function() {
    return new RegExp("(?<\u{1d7da}the>the)");
});

shouldThrowInvalidGroupSpecifierName(function() {
    return new RegExp("(?<\ud835\udfdathe>the)");
});
