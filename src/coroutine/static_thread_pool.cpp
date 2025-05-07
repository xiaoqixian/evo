// Date:   Mon Apr 21 21:15:16 2025
// Mail:   lunar_ubuntu@qq.com
// Author: https://github.com/xiaoqixian

// #include "evo/coroutine/static_thread_pool"
//
// namespace evo {
//
// static_thread_pool::static_thread_pool(size_t num_threads) {
//   threads_.reserve(num_threads);
//   for (size_t i = 0; i < num_threads; i++) {
//     threads_.emplace_back(runner, this, i);
//   }
// }
//
// void static_thread_pool::resume_handle(handle_t h) {
//   if (h == nullptr) return;
//   {
//     std::unique_lock<std::mutex> lk(queue_latch_);
//     queue_.emplace(h);
//   }
//   cv_.notify_one();
// }
//
// void static_thread_pool::shutdown() {
//   shutdown_requested_ = true;
//   cv_.notify_all();
//   for (auto& th: threads_) {
//     if (th.joinable()) th.join();
//   }
// }
//
// void static_thread_pool::runner(static_thread_pool* pool, size_t) {
//   while (!pool->shutdown_requested_.load(std::memory_order_acquire) ||
//       pool->handle_num_.load(std::memory_order_acquire) > 0) 
//   {
//     std::unique_lock<std::mutex> lk(pool->queue_latch_);
//     // if the Predicate returns true, the condition_variable 
//     // stops waiting.
//     pool->cv_.wait(lk, [&]() {
//       return pool->handle_num_.load(std::memory_order_acquire) > 0 ||
//         pool->shutdown_requested_.load(std::memory_order_acquire);
//     });
//
//     // now the lock is acquired by the thread,
//     // we can safely manipulate the queue.
//     while (!pool->queue_.empty()) {
//       auto entry = pool->queue_.front();
//       pool->queue_.pop();
//
//       pool->handle_num_.fetch_sub(1, std::memory_order_release);
//
//       // unlock to allow other threads use queue.
//       lk.unlock();
//       
//       entry.handle.resume();
//
//       // now we need to acquire lock again before
//       // manipulating the queue.
//       lk.lock();
//     }
//   }
// }

// } // namespace evo
