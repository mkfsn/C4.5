#include "Classifier.h"


namespace {
    unsigned int get_decision(int attr, string value);
}


C45_node::C45_node() :attribute(-1), value("") {}


Element::Element()
    :marital_status("O")
     , num_children_at_home("-1")
     , member_card("Basic")
     , age("-1")
     , year_income("-1")
     , other("other") {}


string& Element:: operator[] ( const int index )
{
    assert( index >= 0 && index <= 4 );
    switch (index) {
        case 0:
            return marital_status;
        case 1:
            return num_children_at_home;
        case 2:
            return member_card;
        case 3:
            return age;
        case 4:
            return year_income;
    }
    return other;
}

string Element:: operator[] ( const int index ) const
{
    assert( index >= 0 && index <= 4 );
    switch (index) {
        case 0:
            return marital_status;
        case 1:
            return num_children_at_home;
        case 2:
            return member_card;
        case 3:
            return age;
        case 4:
            return year_income;
    }
    return other;
}


Element C45:: parse_entry ( string str )
{
    string token, delim(",");
    size_t pos = 0;
    Element entry;

    while ( (pos = str.find(delim)) != string::npos ) {
        token = str.substr(0, pos);

        stringstream ss;
        int item;
        string value;
        ss << token;
        ss >> item;
        ss >> value;

        entry[item] = value;
        str.erase(0, pos + 1);
    }
    stringstream ss;
    int item;
    string value;

    ss << str;
    ss >> item;
    ss >> value;

    entry[item] = value;
    return entry;
}


int C45:: insert ( string str )
{
    Element entry = parse_entry(str);
    weight[ entry[2] ]++;
    database.push_back( entry );
    return database.size();
}


vector<double> C45::calculate_info ( vector<Element> dataset, bool* flag )
{
    struct Local {
        static double func_info ( vector<int> arr ) {
            int sum = 0;
            for ( size_t i = 0 ; i < arr.size() ; i++  )
                sum += arr[i];
            double result(0);
            for ( size_t i = 0 ; i < arr.size() ; i++ ) {
                if ( arr[i] != 0 ) {
                    double val = arr[i]*1.0/sum;
                    result -= ( val * log(val) );
                }
            }
            return result;
        }
    };

    vector<vector<vector<int>>> count;
    for ( int i = 0 ; i < 5 ; i++ ) {
        vector<vector<int>> tmp;
        for ( int j = 0 ; j < attribute_size(i) ; j++ ) {
            vector<int> t{0, 0, 0, 0};
            tmp.push_back ( t );
        }
        count.push_back( tmp );
    }

    size_t size = dataset.size();

    for ( unsigned int i = 0 ; i < size ; i++ ) {
        int decision = get_decision(2, dataset[i][2]);
        for ( int j = 0 ; j < 5 ; j++ ) {
            if ( !flag[j] )
                continue;
            if ( j == 2 ) {
                count[j][0][decision]++;
            } else {
                int idx = get_decision(j, dataset[i][j]);
                count[j][idx][decision]++;
            }
        }
    }

    vector<double> info {0, 0, 0, 0, 0};
    for ( int i = 0 ; i < 5 ; i++ ) {
        if ( !flag[i] )
            continue;
        if ( i == 2 ) {
            vector<int> cnt;
            for ( int j = 0 ; j < 4 ; j++  ) {
                cnt.push_back( count[2][0][j] );
            }
            info[2] = Local::func_info( cnt );
        } else {
            size_t len = attribute_size(i);
            for ( unsigned int j = 0 ; j < len ; j++ ) {
                vector<int> cnt;
                int sum = 0;
                for ( int k = 0 ; k < 4 ; k++ ) {
                    sum += count[i][j][k];
                    cnt.push_back( count[i][j][k] );
                }
                info[i] += sum*1.0 / size * Local::func_info(cnt); 
            }
        }
    }
    return info;
}


C45_node* C45::make_node( vector<Element> dataset, bool* flag )
{
    if ( dataset.size() < 1 ) {
        C45_node* node = new C45_node();
        node->attribute = 2;
        map<double, string>::reverse_iterator m = weight_r.rbegin();
        node->value = m->second;
        return node;
    }
    bool check = flag[0] || flag[1] || flag[3] || flag[4];
    if ( !check ) {
        map<string, double> candidate;
        map<double, string> candidate_r;
        for ( size_t i = 0 ; i < dataset.size() ; i++ ) {
            map<string, double>::iterator it = candidate.find( dataset[i][2] );
            if ( it == candidate.end() ) { // not found
                double init = weight[ dataset[i][2] ];
                candidate.insert( pair<string, double>{ dataset[i][2], init } );
            } else {
                candidate[ dataset[i][2] ]++;
            }
        }
        map<string, double>::iterator m1;
        for ( m1 = candidate.begin() ; m1 != candidate.end() ; ++m1 ) {
            candidate_r[ m1->second ] = m1->first;
        }
        map<double, string>::reverse_iterator m2 = candidate_r.rbegin();

        C45_node* node = new C45_node();
        node->attribute = 2;
        node->value = m2->second;
        return node;
    }

    C45_node* node = new C45_node();

    // Compute information-theoretic criteria
    vector<double> info = calculate_info ( dataset, flag ),
        gain { 0, 0, 0, 0, 0 };

    double gain_best(-10);
    for ( int i = 0 ; i < 5 ; i++ ) {
        if ( i == 2 || !flag[i] )
            continue;
        gain[i] = info[2] - info[i];
        if ( gain[i] > gain_best ) {
            gain_best = gain[i];
            node->attribute = i;
        }
    }

    // Get best decision
    bool new_flag[5];
    for ( int i = 0 ; i < 5 ; i++ )
        new_flag[i] = flag[i];
    new_flag[node->attribute] = false;
    node->decision.resize( attribute_size(node->attribute) );
     
    // For each decision in node
    for ( int i = 0 ; i < attribute_size(node->attribute) ; i++ ) {
        vector<Element> new_dataset;
        for ( unsigned int j = 0 ; j < dataset.size() ; j++ ) {
            int idx = node->attribute,
                decision = get_decision( idx, dataset[j][idx] );
            if ( decision == i )
                new_dataset.push_back( dataset[j] );
        }
        node->decision[i] = make_node( new_dataset, new_flag );
    }
    return node;
}


int C45::attribute_size ( int item )
{
    int size[] = {3, 12, 4, 12, 12};
    return size[item];
}


string C45:: traverse ( Element el, C45_node* tree )
{
    if ( tree->attribute == 2 )
        return tree->value;

    int attr = tree->attribute;
    int deci = get_decision( attr, el[attr] );
    return traverse( el, tree->decision[deci] );
}


C45:: C45(string _train = "", string _test = "")
    :train_file(_train), test_file(_test)
     , f_train(train_file.c_str(), ifstream::in)
     , f_test(test_file.c_str(), ifstream::in)
     , weight{ {"Basic", 0}, {"Normal", 0}, {"Silver", 0}, {"Gold", 0} }
{

    #ifdef DEBUG
    cerr << "Reading and parsing training file ..." << endl;
    #endif

    if ( !f_train ) {
        cerr.flush();
        cerr << "Train file '" << train_file << "' Not Found." << endl
            << "Terminated." << endl;
        cerr << endl;
        exit(1);
    }

    string line("");
    while ( f_train.ignore(1) && getline(f_train, line) ) {
        line.replace( line.end()-1, line.end(), "" );
        insert( line );
    }

    #ifdef DEBUG
    cerr << "Reading and parsing training file complete." << endl;
    cerr << endl;
    #endif

    #ifdef DEBUG
    vector<Element>::const_iterator v_it;
    for ( v_it = database.begin() ; v_it != database.end() ; ++v_it ) {
        for ( int i=0 ; i<5 ; i++ ) {
            cerr << i << "->" << (*v_it)[i] << ", ";
        }
        cerr << endl;
    }
    #endif

    if ( !f_test ) {
        cerr.flush();
        cerr << "Test file '" << test_file << "' Not Found." << endl
            << "Terminated." << endl;
        cerr << endl;
        exit(1);
    }

    map<string, double>::iterator m1;
    for ( m1 = weight.begin() ; m1 != weight.end() ; ++m1 ) {
        weight_r[ m1->second/database.size() ] = m1->first;
    }

    #ifdef DEBUG
    map<double, string>::reverse_iterator m2;
    for ( m2 = weight_r.rbegin() ; m2 != weight_r.rend() ; ++m2 ) {
        cerr << '[' << m2->second << "(" << m2->first << ")] ";
    }
    cerr << endl << endl;
    #endif
}


C45::~C45()
{
    // Clean the tree
}


void C45::training (void)
{
    bool flag[] = { true, true, true, true, true };

    #ifdef DEBUG
    cerr << "Building Tree ... " << endl;
    #endif

    tree = make_node( database, flag );

    #ifdef DEBUG
    cerr << "Building Tree complete" << endl;
    cerr << endl;
    #endif
}


void C45::testing (void)
{
    match_correct = 0;
    match_incorrect = 0;
    #ifdef DEBUG
    cerr << "Start testing ... " << endl;
    #endif

    string line("");
    while ( f_test.ignore(1) && getline(f_test, line) ) {
        line.replace( line.end()-1, line.end(), "" );

        #ifdef DEBUG
        cerr << line << endl;
        #endif

        Element el = parse_entry ( line );
        string result = traverse ( el, tree );
        if ( el[2] == result )
            match_correct++;
        else
            match_incorrect++;
        
        #ifdef DEBUG
        cerr << "Result = " << result << endl;
        #endif
    }
    correct_ratio = match_correct*1.0/( match_correct+match_incorrect );

    #ifdef DEBUG
    cerr << "Mission complete. " << endl;
    #endif
}


ostream& operator<< ( ostream& out, const C45& c )
{
    return out << c.correct_ratio * 100;
}


namespace {
    unsigned int get_decision(int attr, string value)
    {
        switch(attr)
        {
            case 0: {
                const char* a[] = {"S", "M"};
                unsigned int i(0), len(sizeof(a)/sizeof(const char*));
                for ( ; i < len ; i++ ) {
                    if ( value == a[i]  ) {
                        return i;
                    }
                }
                return i;
            }
            case 1: {
                int num_children( atoi(value.c_str()) );
                if (num_children > 10)
                    return 10;
                else if ( num_children >= 0 )
                    return num_children;
                else
                    return 11;
            }
            case 2: {
                const char* a[] = {"Basic", "Normal", "Silver", "Gold"};
                unsigned int i(0), len(sizeof(a)/sizeof(const char*));
                for ( ; i < len ; i++ ) {
                    if ( value == a[i]  ) {
                        return i;
                    }
                }
            }
            case 3: {
                int age( atoi(value.c_str()) ),
                    group ( age/10 );
                if ( age < 0 )
                    return 11;
                return group > 10 ? 10 : group;
            }
            case 4: {
                int income ( atoi(value.c_str()) ),
                    group ( income/20000 );
                if ( income < 0 )
                    return 11;
                return group > 10 ? 10 : group;
            }
        }
        return 11;
    }
}

