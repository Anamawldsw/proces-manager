#include <iostream>
#include <pybind11/pybind11.h>

using namespace std;
namespace py = pybind11;


int main(){
    cout << "Elo elo 3 2 0!" << endl;
    cout << "TEST";
    return 0;
}

PYBIND11_MODULE(test, m){
    m.def("test", &main, "funkcja test");
}