#pragma once


#include <cassert>
#include <exception>
#include <iostream>
#include <iomanip>
#include <functional>
#include <chrono>
#include <stack>
#include <array>
#include <string>
#include <memory>
#include <algorithm>
#include <future>
#include <cstring>
#include <map>
#ifdef WIN32
#include <Windows.h>
#else
#include <fstream>
#endif

namespace mochapp {
const char *version = "0.1.0";

namespace details {
#ifdef _MSC_VER
	// this class allows to display checkmark on windows console
	class Unicode : public std::streambuf {
		const int indent{ 2 };
		std::ostream &out;
		std::streambuf *buf;
		int remain = 0;
		int overflow(int ch) override {
			auto  hOutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
			DWORD t;
			static std::vector<char> b;

			if (b.empty()) {
				if (ch >> 5 == 0x06) {
					b.emplace_back((char)ch);
					remain = 1;
				}
				else
					if (ch >> 4 == 0x0E) {
						b.emplace_back((char)ch);
						remain = 2;
					}
					else if (ch >> 3 == 0x1E) {
						b.emplace_back((char)ch);
						remain = 3;
					}
					else {
						buf->sputc(ch);
					}
			}
			else {
				b.emplace_back((char)ch);
				remain--;
			}
			if (remain == 0) {
				WriteConsole(hOutHandle, b.data(), b.size(), &t, NULL);
				b.clear();
				return 1;
			}
			return 0;
		}
		Unicode() = delete;

	public:
		explicit Unicode(std::ostream &out) : out(out), buf(out.rdbuf()) {
			SetConsoleOutputCP(CP_UTF8);
			SetConsoleCP(CP_UTF8);
			out.rdbuf(this);
		}
		~Unicode() { out.rdbuf(buf); }
	};
	static Unicode _unicodeSupport(std::cout);
#endif

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
#else
  static std::map<Color, const char *> unix_colors = {
      {Color::Black, "\033[30m"},      {Color::DarkBlue, "\033[34m"},
      {Color::DarkGreen, "\033[32m"},  {Color::DarkCyan, "\033[36m"},
      {Color::DarkRed, "\033[31m"},    {Color::DarkPurple, "\033[35m"},
      {Color::DarkYellow, "\033[33m"}, {Color::DarkWhite, "\033[37m"},
      {Color::Blue, "\033[94m"},       {Color::Green, "\033[92m"},
      {Color::Cyan, "\033[96m"},       {Color::Red, "\033[91m"},
      {Color::Purple, "\033[95m"},     {Color::Yellow, "\033[93m"},
      {Color::White, "\033[97m"},
  };
  out << unix_colors[c];
#endif
  return out;
}

enum Status { Success, Failed, Pending, StatusCount };

// Helper ostream auto indent
class IndentScope : public std::streambuf {
  const int indent{2};
  std::ostream &out;
  std::streambuf *buf;
  bool startOfLine{true};
  int overflow(int ch) override {
    if (startOfLine && !((ch == '\n') || (ch == '\r'))) {
      for (int i = 0; i < indent; ++i)
        buf->sputc(' ');
    }
    startOfLine = (ch == '\n') || (ch == '\r');
    return buf->sputc(ch);
  }
  IndentScope() = delete;

public:
  explicit IndentScope(std::ostream &out) : out(out), buf(out.rdbuf()) {
    out.rdbuf(this);
  }
  ~IndentScope() { out.rdbuf(buf); }
};

struct IReporter {
  std::array<int, 3> count{};
  std::array<int, 3> durations{};

  virtual ~IReporter() {}
  virtual void beginDescribe(const std::string &name) {}
  virtual void endDescribe(const std::string &name) {}
  virtual void beginIt(const std::string &name) {}
  virtual void endIt(const std::string &name, Status status, int duration) {}
  virtual void _endIt(const std::string &name, Status status, int duration) {
    durations[status] += duration;
    count[status]++;
    endIt(name, status, duration);
  }
  virtual void beginReport(int argc, char **argv) {}
  virtual void endReport() {}
};

struct DefaultReporter : public IReporter {
  std::atomic<bool> testRunning{};
  std::future<void> progressTask;
  std::stack<std::shared_ptr<IndentScope>> indent;

#ifdef _MSC_VER
  std::array<const char *, StatusCount> symbols{{ u8"✔", u8"✘", u8"…" }};
#else
  std::array<const char *, StatusCount> symbols{{"\u2714", "\u2718", "\u2757"}};
#endif
  std::array<Color, StatusCount> colors{
      {Color::DarkGreen, Color::Red, Color::Purple}};

  void beginDescribe(const std::string &name) override {
    std::cout << details::Color::DarkWhite << u8" ⁃ " << name << std::endl;
    indent.emplace(std::make_shared<IndentScope>(std::cout));
  };
  void endDescribe(const std::string &name) override { indent.pop(); };

#ifdef _MSC_VER
  void beginIt(const std::string &name) override {
    testRunning = true;
    progressTask = std::async(std::launch::async, [this, name]() {
      auto i = 0;
      std::string d = R"(\|/)";
      d += (char)196;
      while (testRunning) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (testRunning) {
          std::cout << details::Color::Gray << d[i++ % d.size()] << ' ' << name
                    << " ...\r";
        }
      }
    });
  }
#endif
  void endIt(const std::string &name, Status status, int duration) override {
#ifdef _MSC_VER
    testRunning = false;
    progressTask.wait();
#endif
    std::cout << details::Color::White << u8" ⁃ " << colors[status]
              << symbols[status] << details::Color::DarkWhite << " "
              << details::Color::White << name << " "
              << details::Color::DarkBlue << "(" << duration << "ms)"
              << std::endl;
  }
  void beginReport(int argc, char **argv) {}
  void endReport() override {
    std::cout << details::Color::DarkGreen << count[Success]
              << " tests complete" << details::Color::White << " ("
              << durations[Success] << "ms)" << std::endl;
    std::cout << details::Color::DarkRed << count[Failed] << " tests failed"
              << details::Color::White << " (" << durations[Failed] << "ms)"
              << std::endl;
    std::cout << details::Color::DarkBlue << count[Pending] << " tests pending"
              << details::Color::White << " (" << durations[Pending] << "ms)"
              << std::endl;
    std::cout << "Finished" << std::endl;
  }
};

std::vector<std::pair<const char *, std::function<IReporter *()>>>
    knownReporters;

template <class T> void installReporter(const char *name) {
  knownReporters.emplace_back(std::make_pair(name, []() { return new T; }));
}
struct Mocha {
  std::unique_ptr<IReporter> reporter;

  Mocha() {}
  ~Mocha() {
    if (reporter)
      reporter->endReport();
  }
  bool initialized{false};
  void initialize() {
    if (!initialized) {
      initialized = true;

#ifndef _MSC_VER
      // On unix __argv is not accessible
      // Ugly hack to retreive argc and argv from /proc/self/cmdline
      std::ifstream ifs;
      ifs.open("/proc/self/cmdline", std::ifstream::in);
      char c = ifs.get();
      static std::vector<std::string> ___argv(1);
      while (ifs.good()) {
        if (c == 0) {
          ___argv.emplace_back();
        } else {
          ___argv.back() += c;
        }
        c = ifs.get();
      }
      ___argv.resize(___argv.size() - 1);

      ifs.close();
      static std::vector<char *> ___argvp(___argv.size());
      for (int i = 0; i < ___argv.size(); ++i) {
        ___argvp[i] = const_cast<char *>(___argv[i].c_str());
      }

      static int __argc = ___argvp.size();
      static char **__argv = ___argvp.data();

#endif
      auto argc = __argc;
      const auto first = __argv;
      const auto last = std::next(first, argc);
      auto find_arg = [&](const std::initializer_list<const char *> &options) {
        for (const auto &option : options) {
          auto it = std::find_if(first, last, [option](const char *arg) {
            return !strcmp(option, arg);
          });
          if (std::distance(it, last) >= 2) {
            return std::next(it);
          }
        }
        return last;
      };

      auto has_arg = [&](const std::initializer_list<const char *> &options) {
        for (const auto &option : options) {
          auto it = std::find_if(first, last, [option](const char *arg) {
            return !strcmp(option, arg);
          });
          if (it != last) {
            return true;
          }
        }
        return false;
      };

      installReporter<DefaultReporter>("default");

      auto help = [&]() {
        std::cout << R"(
Usage : cmd [options]

Options:

  -h, --help                              output usage information
  -V, --version                           output the version number
  -R, --reporter <name>                   specify the reporter to use
  --reporters                             display available reporters
)";
        std::exit(0);
      };

      if (has_arg({"--help", "-h"})) {
        help();
      }
      if (has_arg({"--version", "-v"})) {
        std::cout << "version : " << version << std::endl;
		std::exit(0);
      }
      if (has_arg({"--reporters"})) {
        std::cout << "Available Reporters : " << std::endl;
        for (const auto &reporter : knownReporters) {
          std::cout << u8" ⁃ " << reporter.first << std::endl;
        }
        std::exit(0);
      }

      auto reporter_arg = find_arg({"--reporter", "-R"});
      if (reporter_arg != last) {
        const auto &reporters = knownReporters;
        auto it = std::find_if(
            reporters.begin(), reporters.end(),
            [reporter_arg](
                const std::pair<const char *, std::function<IReporter *()>>
                    &kvp) { return !strcmp(*reporter_arg, kvp.first); });
        if (it != reporters.end()) {
          reporter.reset(it->second());
        } else {
          std::cerr << "Reporter not installed \n";
          std::exit(1);
        }
      } else {
        reporter.reset(new DefaultReporter);
      }
      reporter->beginReport(__argc, __argv);
    }
  }

  struct Context {
    std::function<void()> before;
    std::function<void()> after;
    std::function<void()> beforeEach;
    std::function<void()> afterEach;
  };
  std::stack<Context> contexts;

  void pushContext() { contexts.emplace(); }
  void popContext() { contexts.pop(); }
  Context &context() { return contexts.top(); }
};
static Mocha &mocha() {
  static Mocha instance;
  instance.initialize();
  return instance;
};

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

//
class TestFailure {};
class TestSuccess {};
class TestPending {};
}

// Register a new reporter 
// T must implements details::IReporter
template <class T> void installReporter(const char *name) {
	details::installReporter<T>(name);
}

void before(std::function<void()> &&f) {
  details::mocha().contexts.top().before = std::move(f);
}
void after(std::function<void()> &&f) {
  details::mocha().contexts.top().after = std::move(f);
}
void beforeEach(std::function<void()> &&f) {
  details::mocha().contexts.top().beforeEach = std::move(f);
}
void afterEach(std::function<void()> &&f) {
  details::mocha().contexts.top().afterEach = std::move(f);
}
//void retries(unsigned int times) {
//  auto &ctx = details::mocha().contexts.top();
//}
class _describe {
  void execute(const std::string &name, std::function<void()> body) {
    struct Ctx {
      Ctx() { details::mocha().contexts.emplace(); }
      ~Ctx() {
        auto &after = details::mocha().contexts.top().after;
        if (after)
          after();
        details::mocha().contexts.pop();
      }
    } _ctx;

    details::mocha().reporter->beginDescribe(name);
    body();
    details::mocha().reporter->endDescribe(name);
  }

public:
  void operator()(const std::string &name, std::function<void()> body) {
    execute(name, body);
  }
  void only(const std::string &name, std::function<void()> body) {
    execute(name, body);
  }
  void skip(const std::string &name, std::function<void()> body) {}
};
static _describe describe;

class _it {

  void execute(const std::string &name, std::function<void()> test) {
    auto &ctx = details::mocha().contexts.top();
    if (ctx.before) {
      ctx.before();
      ctx.before = nullptr;
    }
    if (ctx.beforeEach) {
      ctx.beforeEach();
    }
    int duration{0};
    details::Status status{details::Pending};
    details::mocha().reporter->beginIt(name);
    try {
      {
        details::StopWatch _(duration);
        test();
      }
    } catch (details::TestSuccess &) {
      status = details::Success;
    } catch (details::TestFailure &) {
      status = details::Failed;
    } catch (...) {
      status = details::Failed;
    }
    details::mocha().reporter->_endIt(name, status, duration);
    if (ctx.afterEach) {
      ctx.afterEach();
    }
  }

public:
  void operator()(const std::string &name) {
    (*this)(name, [] { throw details::Pending; });
  }
  void operator()(const std::string &name, std::function<void()> test) {
    execute(name, test);
  }
  void only(const std::string &name, std::function<void()> test) {
    execute(name, test);
  }
  void skip(const std::string &name, std::function<void()> test) {}
};
static _it it;

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