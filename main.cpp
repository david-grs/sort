#include <string>
#include <chrono>
#include <random>
#include <vector>
#include <array>
#include <algorithm>

#include <iostream>

#define assert_throw(x) if (!(x)) throw std::runtime_error("died line " + std::to_string(__LINE__));

static const std::size_t ContainerSize = std::size_t(1000000);
static const char alpha[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

template <typename IntRandF>
std::string genString(std::size_t len, IntRandF intrand)
{
	std::string str;
	str.reserve(len);

	for (std::size_t i = 0; i < len; ++i)
		str.push_back(alpha[intrand() % (sizeof(alpha) - 1)]);

	return str;
}

struct A
{
	template <typename IntRandF>
	explicit A(IntRandF intrand)
	{
		x = intrand();

		for (std::size_t i = 0; i < d.size(); ++i)
			d[i] = static_cast<double>(intrand());

		for (std::size_t i = 0; i < s.size(); ++i)
			s[i] = genString(20, intrand);
	}

	~A() =default;

	A(const A&) =delete;
	A& operator=(const A&) =delete;

	A(A&&) =default;
	A& operator=(A&&) =default;

	bool operator<(const A& rhs) const { return x < rhs.x; }

	std::size_t x;
	std::array<double, 10> d;
	std::array<std::string, 20> s;
};

template <typename Callable>
void run_benchmark(const std::string& desc, Callable&& callable)
{
	auto start = std::chrono::steady_clock::now();
	callable();
	auto end = std::chrono::steady_clock::now();

	double total_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	if (total_time < 1.0)
	{
		total_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
		std::cout << desc << ": total_time=" << total_time << "us" << std::endl;
	}
	else
	{
		std::cout << desc << ": total_time=" << total_time << "ms" << std::endl;
	}
}

std::vector<A> build()
{
	std::vector<A> va;

	std::random_device rd;
	auto seed = rd();

	std::mt19937 gen(seed);
	std::uniform_int_distribution<std::size_t> rng;

	va.reserve(ContainerSize);

	for (std::size_t i = 0; i < ContainerSize; ++i)
		va.emplace_back([&]() { return rng(gen); });

	return va;
}

std::vector<A*> build_ptr()
{
	std::vector<A*> va;

	std::random_device rd;
	auto seed = rd();

	std::mt19937 gen(seed);
	std::uniform_int_distribution<std::size_t> rng;

	va.reserve(ContainerSize);

	for (std::size_t i = 0; i < ContainerSize; ++i)
		va.emplace_back(new A([&]() { return rng(gen); }));

	return va;
}

int foo()
{
	std::vector<int> va = {1000, 2, 10, 9, 4, 3, 20, 40, 0, 5, 100};
	std::vector<size_t> vi(va.size());
	std::iota(vi.begin(), vi.end(), 0);

	std::partial_sort(vi.begin(), vi.begin() + vi.size() / 2, vi.end(),
					  [&va](size_t i1, size_t i2)
	{
		return va[i1] < va[i2];
	});

	for (std::size_t i = 0; i < vi.size(); ++i)
	{
		std::cout << vi[i] << " => " << va[vi[i]] << std::endl;
	}

	return 0;
}

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		std::cerr << "usage: " << argv[0] << " <sort|partial_sort|quickselsort>" << std::endl;
		return 1;
	}

	//return foo();

	static std::ptrdiff_t K = 1000;

	const std::string argv0(argv[1]);
	if (argv0 == "sort")
	{
		std::vector<A> va = build();
		run_benchmark(argv0, [&]()
		{
			std::sort(va.begin(), va.end());
		});
	}
	else if (argv0 == "partial_sort")
	{
		std::vector<A> va = build();
		run_benchmark(argv0, [&]()
		{
			std::partial_sort(va.begin(), va.begin() + K, va.end());
		});

		for (std::size_t i = 0; i < 100; ++i)
		{
			std::cout << va[i].x << " ";
			assert_throw(va[i].x < va[i + 1].x);
		}
		std::cout << std::endl;
	}
	else if (argv0 == "quickselsort")
	{
		std::vector<A> va = build();
		run_benchmark(argv0, [&]()
		{
			std::nth_element(va.begin(), va.begin() + K, va.end());
			std::sort(va.begin(), va.begin() + K);
		});

		for (std::size_t i = 0; i < 100; ++i)
			std::cout << va[i].x << " ";
		std::cout << std::endl;
	}
	else if (argv0 == "sort_indexes")
	{
		std::vector<A> va = build();
		std::vector<std::size_t> vi;

		run_benchmark(argv0, [&]()
		{
			vi = std::vector<size_t>(va.size());
			std::iota(vi.begin(), vi.end(), 0);

			std::sort(vi.begin(), vi.end(),
					  [&va](size_t i1, size_t i2)
			{
				return va[i1] < va[i2];
			});
		});

		for (std::size_t i = 0; i < 100; ++i)
			std::cout << va[vi[i]].x << " ";
		std::cout << std::endl;
	}
	else if (argv0 == "partial_sort_indexes")
	{
		std::vector<A> va = build();
		std::vector<std::size_t> vi;
		run_benchmark(argv0, [&]()
		{
			vi = std::vector<size_t>(va.size());
			std::iota(vi.begin(), vi.end(), 0);

			std::partial_sort(vi.begin(), vi.begin() + K, vi.end(),
							  [&va](size_t i1, size_t i2)
			{
				return va[i1] < va[i2];
			});
		});

		for (std::size_t i = 0; i < 100; ++i)
		{
			std::cout << va[vi[i]].x << " ";
			assert_throw(va[vi[i]].x < va[vi[i + 1]].x);
		}

		std::cout << std::endl;
	}
	else if (argv0 == "partial_sort_ptr")
	{
		std::vector<A*> va = build_ptr();

		run_benchmark(argv0, [&]()
		{
			std::partial_sort(va.begin(), va.begin() + K, va.end(), [](const A* lhs, const A* rhs) { return *lhs < *rhs; });
		});

		for (std::size_t i = 0; i < 100; ++i)
		{
			std::cout << va[i]->x << " ";
			assert_throw(va[i]->x < va[i + 1]->x);
		}
		std::cout << std::endl;
	}

	return 0;
}
