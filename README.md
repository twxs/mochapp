# mochapp
Simple C++11 Header Only Test Framework Inspired by Mocha 


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
        Assert::equals(4, 4);
    });
});
}
```