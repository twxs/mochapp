#pragma once

#include <exception>
#include <iostream>
#include <iomanip>
#include <functional>
#include <chrono>
#include <stack>
#include <string>
#ifdef WIN32
#include <Windows.h>
#endif
namespace mochapp {

namespace details {

enum class Color {
  Black = 0,
  DarkBlue,
  DarkGreen,
  DarkCyan,
  DarkRed,
  DarkPurple,
  DarkYellow,
  DarkWhite,
  Gray,
  Blue,
  Green,
  Cyan,
  Red,
  Purple,
  Yellow,
  White,
};

static std::ostream &operator<<(std::ostream &out, Color c) {
#ifdef WIN32
  HANDLE hstdout = GetStdHandle(STD_OUTPUT_HANDLE);
  SetConsoleTextAttribute(hstdout, static_cast<int>(c));
#endif
  return out;
}

struct Mocha {
	Mocha() {}
	~Mocha() {

		std::cout << details::Color::DarkGreen << passed << " tests complete" << details::Color::White << " (" << duration_passed << "ms)" << std::endl;
		std::cout << details::Color::DarkRed << failed << " tests failed  " << details::Color::White << " (" << duration_failed << "ms)" << std::endl;
		std::cout << details::Color::DarkPurple << pending << " tests pending " << details::Color::White << " (" << duration_pending << "ms)" << std::endl;
		std::cout << "Finished" << std::endl;
	}
	int passed{ 0 };
	int failed{ 0 };
	int pending{ 0 };
	int duration_passed{ 0 };
	int duration_failed{ 0 };
	int duration_pending{ 0 };
	struct Context {
		std::function<void()> before;
		std::function<void()> after;
		std::function<void()> beforeEach;
		std::function<void()> afterEach;
	};
	std::stack<Context> contexts;
};
static Mocha mocha;


class StopWatch {
  using clock = std::chrono::high_resolution_clock;
  clock::time_point start;
  int &duration;

public:
  StopWatch(int &ms) : start(clock::now()), duration(ms) {}
  ~StopWatch() {
    auto now = clock::now();
    duration = static_cast<int>(
        std::chrono::duration_cast<std::chrono::milliseconds>(now - start)
            .count());
  }
};

// Helper ostream auto indent
class IndentScope : public std::streambuf {
  const int indent{2};
  std::ostream &out;
  std::streambuf *buf;
  bool startOfLine{true};
  int overflow(int ch) override {
    if (startOfLine && ch != '\n') {
      for (int i = 0; i < indent; ++i)
        buf->sputc(' ');
    }
    startOfLine = ch == '\n';
    return buf->sputc(ch);
  }
  IndentScope() = delete;

public:
  explicit IndentScope(std::ostream &out) : out(out), buf(out.rdbuf()) {
    out.rdbuf(this);
  }
  ~IndentScope() { out.rdbuf(buf); }
};
class TestFailure {};
class TestSuccess {};
class TestPending {};
}

void before(std::function<void()> &&f) {
  details::mocha.contexts.top().before = std::move(f);
}
void after(std::function<void()> &&f) {
  details::mocha.contexts.top().after = std::move(f);
}
void beforeEach(std::function<void()> &&f) {
  details::mocha.contexts.top().beforeEach = std::move(f);
}
void afterEach(std::function<void()> &&f) {
  details::mocha.contexts.top().afterEach = std::move(f);
}

void describe(const char *name, std::function<void()> body) {
  struct Ctx {
    Ctx() { details::mocha.contexts.emplace(); }
    ~Ctx() {
      auto &after = details::mocha.contexts.top().after;
      if (after)
        after();
      details::mocha.contexts.pop();
    }
  } _ctx;
  details::IndentScope _(std::cout);

  std::cout << details::Color::DarkWhite<< name << std::endl;
  body();
}
void it(const std::string &name, std::function<void()> test) {
  auto &ctx = details::mocha.contexts.top();
  if (ctx.before) {
    ctx.before();
    ctx.before = nullptr;
  }
  if (ctx.beforeEach) {
    ctx.beforeEach();
  }
  details::IndentScope _(std::cout);
  int duration;
  try {
    {
      details::StopWatch _(duration);
      test();
    }
    std::cout << details::Color::Purple << "?" << details::Color::DarkWhite
              << " " << details::Color::DarkWhite << name << " "
              << details::Color::DarkBlue << "(" << duration << "ms) -- pending"
              << std::endl;
	details::mocha.duration_pending += duration;
	details::mocha.pending++;

  } catch (details::TestSuccess &) {
    std::cout << details::Color::DarkGreen << "o" << details::Color::DarkWhite
              << " " << details::Color::White << name << " "
              << details::Color::DarkBlue << "(" << duration << "ms)"
              << std::endl;
	details::mocha.duration_passed += duration;
	details::mocha.passed++;
  } catch (details::TestFailure &) {
    std::cout << details::Color::DarkRed << "x" << details::Color::DarkWhite
              << " " << details::Color::White << name << " "
              << details::Color::DarkBlue << "(" << duration << "ms)"
              << std::endl;
	details::mocha.duration_failed += duration;
	details::mocha.failed++;
  }
  catch (...) {
	  std::cout << details::Color::Red << "!" << details::Color::DarkWhite
		  << " " << details::Color::White << name << " "
		  << details::Color::DarkBlue << "(" << duration << "ms) -- Uncatched exception"
		  << std::endl;
	  details::mocha.duration_failed += duration;
	  details::mocha.failed++;  
  }
  if (ctx.afterEach) {
    ctx.afterEach();
  }
}
class Assert {

public:
  template <class T, class U> static void equals(T &&a, U &&b) {
    isTrue(a == b);
  }
  template <class T, class Func> static void throws(Func &&f) {
    bool hasThrown = false;
    try {
      f();
    } catch (T &) {
      hasThrown = true;
    } catch (...) {
    }
    isTrue(hasThrown);
  }
  static void isTrue(bool b) {
    if (b)
      throw details::TestSuccess();
    else
      throw details::TestFailure();
  }
  static void isFalse(bool b) { isTrue(!b); }
};
}