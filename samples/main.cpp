#include <mochapp/mochapp.h>
#include <thread>
#include <numeric>
#include <tuple>
#include <string>
using namespace mochapp;

int add(const std::initializer_list<int> &values)
{
	return std::accumulate(std::begin(values), std::end(values), 0);
}

int main(int argc, char ** argv){
	describe("add()", []() {
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
	describe("Hooks", [] {
		before([] {
			std::cerr << details::Color::Gray <<  "// runs before all tests in this block" << std::endl; 
		});
		after([] { std::cerr << details::Color::Gray << "// runs after all tests in this block" << std::endl; });
		beforeEach([] { std::cerr << details::Color::Gray << "  // runs before each test in this block" << std::endl; });
		afterEach([] { std::cerr << details::Color::Gray << "  // runs after each test in this block" << std::endl; });

		it("test case 1", [] {});
		it("test case 2", [] {});
	});
	describe("Uncaught exception", [] {
		it("Should catch any exception", [] {
			throw std::exception("really bad");
		});
	});
	describe("Long run", [] {
		it("Should should take some times", [] {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		});
		it("Should should take less times", [] {
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
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

    return 0;
}