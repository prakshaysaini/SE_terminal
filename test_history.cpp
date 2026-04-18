#include <iostream>
#include <vector>

int main() {
    std::vector<int> history = {1, 2, 3};
    int historyIndex = 3;
    if (historyIndex == history.size()) {
        std::cout << "EQUAL" << std::endl;
    } else {
        std::cout << "NOT EQUAL" << std::endl;
    }
    return 0;
}
