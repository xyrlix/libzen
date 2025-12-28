#include <zen/threading/pool/thread_pool.h>
#include <zen/fmt.h>
#include <chrono>

int main() {
    // Create a thread pool with 4 threads
    zen::thread_pool pool(4);
    
    zen::print("Thread pool created with {} threads", pool.thread_count());
    
    // Submit tasks to the thread pool
    std::vector<std::future<int>> futures;
    
    for (int i = 0; i < 8; ++i) {
        futures.emplace_back(pool.submit([i]() {
            zen::print("Task {} running", i);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            return i * i;
        }));
    }
    
    // Wait for all tasks to complete and collect results
    zen::print("Collecting results...");
    
    int sum = 0;
    for (auto& future : futures) {
        sum += future.get();
    }
    
    zen::print("Sum of squares: {}", sum);
    
    return 0;
}