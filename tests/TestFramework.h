#pragma once

#include <cmath>
#include <exception>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace TestFramework {

struct TestCase {
	std::string name;
	std::function<void()> run;
};

/// <summary>
/// 各翻訳単位から登録されたテストを保持する。関数内staticにして初期化順序問題を避ける。
/// </summary>
inline std::vector<TestCase> &Registry() {
	static std::vector<TestCase> tests;
	return tests;
}

class Registrar final {
  public:
	Registrar(std::string name, std::function<void()> test) {
		Registry().push_back({std::move(name), std::move(test)});
	}
};

inline void Require(bool condition, std::string_view message) {
	if (!condition) {
		throw std::runtime_error(std::string(message));
	}
}

template <typename Left, typename Right>
void RequireEqual(const Left &actual, const Right &expected, std::string_view message) {
	if (!(actual == expected)) {
		throw std::runtime_error(std::string(message));
	}
}

inline void RequireNear(float actual, float expected, float tolerance, std::string_view message) {
	if (!std::isfinite(actual) || std::abs(actual - expected) > tolerance) {
		throw std::runtime_error(std::string(message));
	}
}

template <typename Exception, typename Callback>
void RequireThrows(Callback &&callback, std::string_view message) {
	try {
		std::forward<Callback>(callback)();
	} catch (const Exception &) {
		return;
	} catch (...) {
		throw std::runtime_error(std::string(message) + " (different exception type)");
	}
	throw std::runtime_error(std::string(message) + " (no exception)");
}

inline int RunAll() {
	int failed = 0;
	for (const TestCase &test : Registry()) {
		try {
			test.run();
			std::cout << "[PASS] " << test.name << '\n';
		} catch (const std::exception &exception) {
			++failed;
			std::cerr << "[FAIL] " << test.name << ": " << exception.what() << '\n';
		} catch (...) {
			++failed;
			std::cerr << "[FAIL] " << test.name << ": unknown exception\n";
		}
	}
	std::cout << "\n" << Registry().size() - failed << "/" << Registry().size() << " tests passed.\n";
	return failed == 0 ? 0 : 1;
}

} // namespace TestFramework

#define TEST_CONCAT_INNER(left, right) left##right
#define TEST_CONCAT(left, right) TEST_CONCAT_INNER(left, right)
#define TEST_CASE(name)                                                                                                 \
	static void TEST_CONCAT(TestFunction_, __LINE__)();                                                                   \
	static TestFramework::Registrar TEST_CONCAT(TestRegistrar_, __LINE__)(name, TEST_CONCAT(TestFunction_, __LINE__));    \
	static void TEST_CONCAT(TestFunction_, __LINE__)()

