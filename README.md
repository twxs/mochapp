# mochapp
Simple C++11 Header Only Test Framework Inspired by Mocha 

[![Build Status](https://travis-ci.org/twxs/mochapp.svg?branch=master)](https://travis-ci.org/twxs/mochapp)

## Usage :

```cpp
#include <mochapp/mochapp.h>
using namspace mochapp;

int main(){
describe("#equals()", [] {
    it("Should failed", [] {
        Assert::equals(5, 4);
    });
    it("Should pass", [] {
        Assert::isTrue(true);
    });
});
return 0;
}
```

Will output :

```
  #equals()
    ✘ Should failed (0ms)
    ✔ Should pass (0ms)
```

## Hooks

```cpp
describe("Hooks", [] {
		
    before([] {
        // runs before all tests in this block
    });
    after([] { 
        // runs after all tests in this block
    });
    beforeEach([] { 
        // runs before each test in this block 
    });
    afterEach([] { 
        // runs after each test in this block
    });

    // test cases

});
```


## Reporters

