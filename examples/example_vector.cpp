#include <zen/containers/vector.h>
#include <zen/fmt.h>

int main() {
    // Create a vector and add elements
    zen::vector<int> vec;
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);
    vec.push_back(4);
    vec.push_back(5);
    
    // Print vector contents
    zen::print("Vector elements: {}", vec);
    
    // Access elements
    zen::print("First element: {}", vec[0]);
    zen::print("Last element: {}", vec.back());
    
    // Iterate through vector
    zen::print("Iterating through vector:");
    for (const auto& elem : vec) {
        zen::print("  {}", elem);
    }
    
    // Modify elements
    vec[2] = 42;
    zen::print("Modified vector: {}", vec);
    
    // Remove elements
    vec.pop_back();
    vec.erase(vec.begin() + 1);
    zen::print("After removal: {}", vec);
    
    return 0;
}