// Date:   Mon May 12 19:46:17 2025
// Mail:   lunar_ubuntu@qq.com
// Author: https://github.com/xiaoqixian

#include "evo/slab"
#include <algorithm>
#include <gtest/gtest.h>
#include <random>
#include <unordered_map>
#include <vector>

TEST(SlabTest, SlabTest) {
  using T = unsigned long;
  evo::slab<T> slab;
  std::unordered_map<T, T> um;
  
  std::random_device rd;
  std::mt19937 gen(rd());

  std::vector<T> nums;
  constexpr size_t size = 100'000;
  nums.reserve(size);
  for (int i = 0; i < size; i++) {
    nums.emplace_back(i);
  }

  std::shuffle(nums.begin(), nums.end(), gen);

  for (auto num: nums) {
    auto token = slab.emplace(num);
    um.emplace(num, token);
  }

  std::shuffle(nums.begin(), nums.begin() + size / 2, gen);

  // erase half of the elements
  for (int i = 0; i < size / 2; i++) {
    auto token = um[nums[i]];
    ASSERT_EQ(slab[token], nums[i]);
    um.erase(nums[i]);
    slab.erase(token);
  }

  // make sure the other half still exist
  for (int i = size/2; i < size; i++) {
    auto token = um[nums[i]];
    ASSERT_EQ(slab[token], nums[i]);
  }

  std::shuffle(nums.begin(), nums.begin() + size / 2, gen);

  // shuffle the erased half and put them back
  for (int i = 0; i < size / 2; i++) {
    auto token = slab.emplace(nums[i]);
    um.emplace(nums[i], token);
  }

  // make sure the other half remain the same
  for (int i = size/2; i < size; i++) {
    auto token = um[nums[i]];
    ASSERT_EQ(slab[token], nums[i]);
  }
}

