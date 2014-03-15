#include "Classifier.h"
#include <fstream>
#include <string>

using std::ifstream;
using std::cout;
using std::cerr;
using std::endl;
using std::string;

void print_usage(void);

int main(int argc, char* argv[])
{
    if ( argc < 3 ) {
        print_usage();
        return 1;
    }

    C45 c45(argv[1], argv[2]);

    c45.training();
    c45.testing();

    cout << c45;

    return 0;
}

void print_usage(void)
{
    cerr << "Usage:" << endl
        << "\t ./main training_file test_file" << endl
        << endl
        << "Example:" << endl
        << "\t ./main training.arff test.arff" << endl
        << endl;
}
