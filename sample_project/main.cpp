#include <iostream>

int Bad_function_name(int a){
    int count;
    if (a>0){
        std::cout<<count<<"\n"; // use before init + spacing issues
    }

    int* p = new int;
    return 0; // leak: p not deleted
}

int main() {
    Bad_function_name(1);
    return 0;
}
