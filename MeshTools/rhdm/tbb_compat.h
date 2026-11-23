/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2021 University of Utah, The Trustees of Columbia University in
the City of New York, and others.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

// Minimal stand-in for the subset of Intel TBB used by the robust hex-dominant
// meshing code (blocked_range, parallel_for, parallel_reduce, spin_mutex).
// parallel_for is implemented with std::thread; reductions run serially so
// they are deterministic. Setting tbb::max_threads() = 1 makes everything serial.
#pragma once
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <algorithm>
#include <exception>

namespace tbb {

// number of worker threads used by parallel_for (0 = hardware concurrency)
inline int& max_threads() { static int n = 0; return n; }

template <typename T> class blocked_range
{
public:
	blocked_range(T b, T e, size_t grain = 1) : m_begin(b), m_end(e), m_grain(grain == 0 ? 1 : grain) {}
	T begin() const { return m_begin; }
	T end() const { return m_end; }
	size_t size() const { return (size_t)(m_end - m_begin); }
	size_t grainsize() const { return m_grain; }
	bool empty() const { return !(m_begin < m_end); }
private:
	T m_begin, m_end;
	size_t m_grain;
};

template <typename T, typename Func>
void parallel_for(const blocked_range<T>& range, const Func& f)
{
	if (range.empty()) return;
	int nt = max_threads();
	if (nt <= 0) nt = (int)std::max(1u, std::thread::hardware_concurrency());
	size_t N = range.size();
	size_t grain = range.grainsize();
	size_t chunks = (N + grain - 1) / grain;
	if ((nt == 1) || (chunks <= 1)) { f(range); return; }

	std::atomic<size_t> next(0);
	std::exception_ptr err = nullptr;
	std::mutex errLock;
	auto worker = [&]() {
		try {
			while (true)
			{
				size_t c = next++;
				if (c >= chunks) break;
				T b = range.begin() + (T)(c * grain);
				T e = (c + 1 == chunks ? range.end() : (T)(range.begin() + (T)((c + 1) * grain)));
				f(blocked_range<T>(b, e, grain));
			}
		}
		catch (...) {
			std::lock_guard<std::mutex> lock(errLock);
			if (!err) err = std::current_exception();
			next = chunks;
		}
	};
	int nworkers = (int)std::min<size_t>((size_t)nt, chunks);
	std::vector<std::thread> threads;
	for (int i = 1; i < nworkers; ++i) threads.emplace_back(worker);
	worker();
	for (auto& t : threads) t.join();
	if (err) std::rethrow_exception(err);
}

template <typename T, typename Value, typename Map, typename Reduce>
Value parallel_reduce(const blocked_range<T>& range, const Value& identity, const Map& map, const Reduce& reduce)
{
	if (range.empty()) return identity;
	// serial, chunked in the same way as TBB's deterministic reduction
	Value result = identity;
	size_t grain = range.grainsize();
	for (T b = range.begin(); b < range.end(); )
	{
		T e = (T)std::min<size_t>((size_t)range.end(), (size_t)b + grain);
		result = reduce(result, map(blocked_range<T>(b, e, grain), identity));
		b = e;
	}
	return result;
}

template <typename T, typename Value, typename Map, typename Reduce>
Value parallel_deterministic_reduce(const blocked_range<T>& range, const Value& identity, const Map& map, const Reduce& reduce)
{
	return parallel_reduce(range, identity, map, reduce);
}

class spin_mutex
{
public:
	void lock() { while (m_flag.test_and_set(std::memory_order_acquire)) std::this_thread::yield(); }
	void unlock() { m_flag.clear(std::memory_order_release); }

	class scoped_lock
	{
	public:
		scoped_lock(spin_mutex& m) : m_m(m) { m_m.lock(); }
		~scoped_lock() { m_m.unlock(); }
	private:
		spin_mutex& m_m;
	};

private:
	std::atomic_flag m_flag = ATOMIC_FLAG_INIT;
};

} // namespace tbb
