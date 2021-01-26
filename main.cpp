#include <iostream>
#include <vector>

#include <source_dir_locator.hpp>
#include <parser.hpp>
#include <unpacker.hpp>

class Solution {
public:
    int test(std::vector<int> nums, int n) {
        return nums.size() + n;
    }
};

int main() {
#if (defined(_WIN32) || defined(_WIN64))
    auto testcases = PROJECT_SOURCE_DIR "\\" "testcases.txt";
#else
    auto testcases = PROJECT_SOURCE_DIR "/" "testcases.txt";
#endif

    auto params = parse_params(testcases);
    auto results = Unpacker<std::vector<int>, int>().invoke(params, &Solution::test, Solution());

    for (auto v : results) {
        std::cout << v << ' ';
    }
    std::cout << std::endl;

    return EXIT_SUCCESS;
}
