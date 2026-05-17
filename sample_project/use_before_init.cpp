#include <iostream>

int useBeforeInit()
{
    int count;
    std::cout << count << "\n";
    return count;
}
