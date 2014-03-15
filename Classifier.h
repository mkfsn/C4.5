#ifndef __MKFSN_CLASSIFIER_H__
#define __MKFSN_CLASSIFIER_H__


#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <fstream>
#include <map>
#include <cstdlib>
#include <cassert>
#include <cmath>
#include <memory>


using std::istream;
using std::ostream;
using std::string;
using std::map;
using std::vector;
using std::stringstream;
using std::pair;
using std::ifstream;
using std::cout;
using std::cerr;
using std::endl;
using std::unique_ptr;


/**************************************************************
 * [0]@attribute marital_status {S,M}
 *   : S{0}, M{1}, other(2)
 * [1]@attribute num_children_at_home numeric
 *   : 0{0}, 1{1}, 2{2}, ... , 9{9}, 10-{10}, other(11)
 * [2]@attribute member_card {Basic,Normal,Silver,Gold}
 *   : Basic{0}, Normal{1}, Silver{2}, Gold{3}
 * [3]@attribute age numeric
 *   : 0-9{0}, 10-19{1}, ... , 90-99{9}, 100-{10}, other(11)
 * [4]@attribute year_income numeric
 *   : 0-19999{0}, 20000-39999{1}, ... , 200000-{10}, other(11)
 **************************************************************/
struct C45_node {
    int attribute;
    string value;
    vector<unique_ptr<C45_node>> decision;
    C45_node();
};


struct Element {
    /* Data member */
    string marital_status;
    string num_children_at_home;
    string member_card;
    string age;
    string year_income;
    string other;

    /* Member function */
    Element();
    string& operator[] ( const int index ); 
    string operator[] ( const int index ) const; 
};


class C45 {
    /* Data member */
    vector<Element> database;    
    string train_file, test_file;
    ifstream f_train, f_test;
    unique_ptr<C45_node> tree;
    map<string, double> weight;
    map<double, string> weight_r;

    int match_correct, match_incorrect;
    double correct_ratio;

    /* Member function */
    Element parse_entry ( string str );
    int insert ( string str );
    vector<double> calculate_info ( vector<Element> dataset, bool* flag );
    unique_ptr<C45_node> make_node( vector<Element> dataset, bool* flag );
    int attribute_size ( int item );
    string traverse ( Element el, unique_ptr<C45_node>& tree );

public:
    C45(string _train, string _test);
    ~C45();

    void training(void);
    void testing(void);

    friend ostream& operator<< ( ostream& out, const C45 &c );
};



#endif
