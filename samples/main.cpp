#include <mochapp/mochapp.h>
#include <thread>
#include <numeric>
#include <tuple>
#include <string>
#include <future>
using namespace mochapp;

int add(const std::initializer_list<int> &values)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(30*values.size()));
	return std::accumulate(std::begin(values), std::end(values), 0);
	 
}

struct Reporter : public details::IReporter {
	void endReport()override {
		std::cout << std::accumulate(count.begin(), count.end(), 0) << "tests runned in "<<
			std::accumulate(durations.begin(), durations.end(), 0)
			<<"ms." << std::endl;
	}
};

using namespace std;

enum Toto {
Totot_YO
	};
int main(){
	

	details::installReporter<Reporter>("custom");

	describe.only("Add", []() {
		using ret_t = int;
		using arg_t = std::initializer_list<int>;
		using test_t = std::pair<arg_t, ret_t>;
		std::initializer_list<test_t> tests = {
			{ arg_t{ 1, 2 }, ret_t{ 3 } },
			{ arg_t{ 1, 2, 3 }, ret_t{ 6 } },
			{ arg_t{ 1, 2, 3, 4 }, ret_t{ 10 } },			
		};
		for (const auto &test : tests) {
			it("correctly adds " + std::to_string(test.first.size()) + " args", [&] {
				auto res = add(test.first);
				Assert::equals(res, test.second);
			});
		}
		
	});
	describe("Loop", []() {
		for (int i = 0; i < 50; ++i) {
			it("test #" + std::to_string(i), [] {
				std::this_thread::sleep_for(std::chrono::milliseconds(50 + rand() % 50));
				Assert::isTrue(rand()%2 == 0);
			});
		}
	});
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
	describe("Uncaught exception", [] {
		it("Should catch any exception", [] {
			throw std::exception();
		});
	});
	describe("Long run", [] {
		it("Should should take some times", [] {
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			Assert::isTrue(true);
		});
		it("Should should take less times", [] {
			std::this_thread::sleep_for(std::chrono::milliseconds(101));
		});
	});
	describe("Assert", [] {
		describe("#throws<int>()", [] {
			it("Should pass if an int is thrown", [] {
				Assert::throws<int>([] {throw 0; });
			});
			it("Should failed if there is no exception thrown", [] {
				Assert::throws<int>([] {});
			});
			it("Should failed if std::exception is thrown", [] {
				Assert::throws<int>([] {throw std::exception(); });
			});
		});
		describe("#equals()", [] {
			it("Should failed", [] {
				Assert::equals(5, 4);
			});
			it("Should pass", [] {
				Assert::equals(4, 4);
			});
		});
		describe("#isFalse()", [] {
			it("Should failed", [] {
				Assert::isFalse(true);
			});
			it("Should pass", [] {
				Assert::isFalse(false);
			});
		});
		describe("#isTrue()", [] {
			it("Should pass", [] {
				Assert::isTrue(true);
			});
			it("Should failed", [] {
				Assert::isTrue(false);
			});
		});
	});

    return Totot_YO;
}
