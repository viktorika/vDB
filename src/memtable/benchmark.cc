#include "benchmark/benchmark.h"
#include <iostream>
#include <random>
#include <unordered_map>
#include "art.h"

uint32_t kKeySize = 10000;
uint32_t kKeyLength = 20;
std::vector<std::string> keys(kKeySize);

static int GenKeys() {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> distrib(32, 126);
  for (int i = 0; i < kKeySize; i++) {
    keys[i].resize(kKeyLength);
    for (int j = 0; j < kKeyLength; j++) {
      keys[i][j] = distrib(gen);
    }
  }
  return 0;
}

static void RunInsertDeConstructHashMap(benchmark::State& state) {
  for (auto _ : state) {
    std::unordered_map<std::string, std::string> map_;
    for (int i = 0; i < kKeySize; i++) {
      map_[keys[i]] = "123";
    }
  }
}

static void RunInsertDeConstructArt(benchmark::State& state) {
  for (auto _ : state) {
    vDB::Art<std::string> art_;
    for (int i = 0; i < kKeySize; i++) {
      art_.Upsert(keys[i], "123");
    }
  }
}

static void RunFindHashMap(benchmark::State& state) {
  std::unordered_map<std::string, std::string> map_;
  for (int i = 0; i < kKeySize; i++) {
    map_[keys[i]] = "123";
  }
  std::vector<std::string> values(kKeySize);
  for (auto _ : state) {
    for (int i = 0; i < kKeySize; i++) {
      values[i] = map_[keys[i]];
    }
  }
}

static void RunFindArt(benchmark::State& state) {
  vDB::Art<std::string> art_;
  for (int i = 0; i < kKeySize; i++) {
    art_.Upsert(keys[i], "123");
  }
  std::vector<std::string> values(kKeySize);
  for (auto _ : state) {
    for (int i = 0; i < kKeySize; i++) {
      art_.Find(keys[i], &values[i]);
    }
  }
}

static void RunDeleteHashMap(benchmark::State& state) {
  for (auto _ : state) {
    std::unordered_map<std::string, std::string> map_;
    for (int i = 0; i < kKeySize; i++) {
      map_[keys[i]] = "123";
    }
    for (int i = 0; i < kKeySize; i++) {
      map_.erase(keys[i]);
    }
  }
}

static void RunDeleteArt(benchmark::State& state) {
  for (auto _ : state) {
    vDB::Art<std::string> art_;
    for (int i = 0; i < kKeySize; i++) {
      art_.Upsert(keys[i], "123");
    }
    for (int i = 0; i < kKeySize; i++) {
      art_.Delete(keys[i]);
    }
  }
}

BENCHMARK(RunInsertDeConstructHashMap);
BENCHMARK(RunInsertDeConstructArt);
BENCHMARK(RunFindHashMap);
BENCHMARK(RunFindArt);
BENCHMARK(RunDeleteHashMap);
BENCHMARK(RunDeleteArt);

int main(int argc, char** argv) {
  GenKeys();
  ::benchmark ::Initialize(&argc, argv);
  if (::benchmark ::ReportUnrecognizedArguments(argc, argv)) return 1;
  ::benchmark ::RunSpecifiedBenchmarks();
  ::benchmark ::Shutdown();
  return 0;
}