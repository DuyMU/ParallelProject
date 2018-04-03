/*
 * =====================================================================================
 *
 *       Filename:  Data_Mining-Association_Rule-Apriori.cpp
 *
 *    Description:  Data Mining - Association Rule
 *                  Apriori C++ Implementation
 *
 *       Compiler:  g++
 *       Platform:  OS X 10.7
 *
 *           Name:  KuoE0 (kuoe0.tw@gmail.com)
 *         School:  Computer Science and Information Engineering
 *                  National Cheng Kung University, Taiwan
 *
 * =====================================================================================
 */

#include <iostream>
#include <ctime>
#include <fstream>
#include <map>
#include <cstring>
#include <set>
#include <string>
using namespace std;

typedef set< int > ItemSet;
typedef set< ItemSet > SuperItemSet;

typedef ItemSet::iterator ItemSetIter;
typedef SuperItemSet::iterator SuperItemSetIter;

SuperItemSet makeL1( const char *input_filename, map< ItemSet, int > &SetSupport, const double min_sup );
SuperItemSet scanDB( const char *input_filename, const SuperItemSet &L, map< ItemSet, int > &SetSupport, const double min_sup );
double calSupport( const char *input_filename, const ItemSet &itemset, map< ItemSet, int > &SetSupport );
bool isSupport( const string &str, ItemSet itemset );
SuperItemSet generateCk( const SuperItemSet &L );
bool hasInfrequent( const ItemSet &itemset, const SuperItemSet &L );
SuperItemSet genSubset( const ItemSet &itemset );
void showRule( ofstream &outfile, const SuperItemSet &L, const map< ItemSet, int > &SetSupport, const double min_conf );
void partition( ofstream &outfile, const ItemSet &itemset, const map< ItemSet, int > &SetSupport, ItemSet &P1, ItemSet &P2, ItemSetIter iter, const double min_conf );

int main( int argc, char *argv[] ) {

	if ( argc < 5 ) {
		cout << "Find association rule:" << endl;
		cout << "Usage: ./Apriori [data set] [output file] [min support] [min confidence]" << endl;
		return 0;
	}
	
	const double min_sup = atof( argv[ 3 ] );
	const double min_conf = atof( argv[ 4 ] );
	const char *input_filename = argv[ 1 ];
	const char *output_filename = argv[ 2 ];
	
	ofstream outfile( output_filename );
	map< ItemSet, int > SetSupport;

	clock_t st = clock();

	// generate L1
	SuperItemSet L = makeL1( input_filename, SetSupport, min_sup );
	while ( L.size() ) {
		L = scanDB( input_filename, generateCk( L ), SetSupport, min_sup );
		showRule( outfile, L, SetSupport, min_conf );
	}

	outfile.close();
	printf( "Execution time: %.2lf sec.\n", double( clock() ) / st );
	return 0;
}
// generate L1 from database
SuperItemSet makeL1( const char *input_filename, map< ItemSet, int > &SetSupport, const double min_sup ){
	string str;
	ifstream in( input_filename );
	SuperItemSet L;
	while ( !in.eof() ) {
		getline( in, str );
		char cstr[ str.length() + 1 ];
		strcpy( cstr, str.c_str() );
		for ( char *token = strtok( cstr, " " ); token; token = strtok( 0, " " ) ) {
			ItemSet temp;
			temp.insert( atoi( token ) );
			L.insert( temp );
		}
	}
	in.close();
	return scanDB( input_filename, L, SetSupport, min_sup );
}
SuperItemSet scanDB( const char *input_filename, const SuperItemSet &L, map< ItemSet, int > &SetSupport, const double min_sup ) {
	SuperItemSet ret;
	for ( SuperItemSetIter iter = L.begin(); iter != L.end(); ++iter )
		// check it is a frequet itemset
		if ( calSupport( input_filename, *iter, SetSupport ) >= min_sup )
			ret.insert( *iter );
	return ret;
}
// calculate the support
double calSupport( const char *input_filename, const ItemSet &itemset, map< ItemSet, int > &SetSupport ) {

	ifstream infile( input_filename );
	int cnt, total;
	string str;

	for ( total = 0, cnt = 0; !infile.eof(); ++total ) {
		getline( infile, str );
		if ( isSupport( str, itemset ) )
			++cnt;
	}
	return double( cnt ) / total;
}
// check this transaction is a support for this itemset
bool isSupport( const string &str, ItemSet itemset ) {
	char cstr[ str.length() + 1 ];
	strcpy( cstr, str.c_str() );
	// get elements in sting by split in into tokens
	for ( char *token = strtok( cstr, ", " ); token; token = strtok( 0, ", " ) ) {
		int x = atoi( token );
		// if this element in our set, delete in from our set
		if ( itemset.find( x ) != itemset.end() )
			itemset.erase( x );
	}
	// if this set is empty, means that this set is a candicate
	return itemset.empty();
}
// generate Ck from Lk-1
SuperItemSet generateCk( const SuperItemSet &L ) {
	SuperItemSet ret;
	for ( SuperItemSetIter iter = L.begin(); iter != L.end(); ++iter ) {
		SuperItemSetIter t = iter;
		for ( SuperItemSetIter iter2 = ++t; iter2 != L.end(); ++iter2 ) {
			ItemSet s1( (*iter).begin(), --(*iter).end() ), s2( (*iter2).begin(), --(*iter2).end() );
			int n1 = *( --(*iter).end() ), n2 = *( --(*iter2).end() );
			if ( s1 == s2 && n1 != n2 ) {
				ItemSet temp = *iter;
				temp.insert( n2 );
				if ( !hasInfrequent( temp, L ) )
					ret.insert( temp );
			}
		}
	}
	return ret;
}
// is there any subset in this itemset is not a frequet itemset
bool hasInfrequent( const ItemSet &itemset, const SuperItemSet &L ) {
	SuperItemSet temp = genSubset( itemset );
	for ( SuperItemSetIter iter = temp.begin(); iter != temp.end(); ++iter ) {
		if ( L.find( *iter ) == L.end() )
			return 1;
	}
	return 0;
}
// generate all subset
SuperItemSet genSubset( const ItemSet &itemset ) {
	SuperItemSet ret;
	for ( ItemSetIter iter = itemset.begin(); iter != itemset.end(); ++iter ) {
		ItemSet temp = itemset;
		temp.erase( *iter );
		ret.insert( temp );
	}
	return ret;
}

// show all rule
void showRule( ofstream &outfile, const SuperItemSet &L, const map< ItemSet, int > &SetSupport, const double min_conf ) {
	for ( SuperItemSetIter iter = L.begin(); iter != L.end(); ++iter ) {
		int s = (*iter).size();
		// enumerate all partition
		ItemSet P1, P2;
		partition( outfile, *iter, SetSupport, P1, P2, (*iter).begin(), min_conf );
	}
}

// enumerate to find the rule
void partition( ofstream &outfile, const ItemSet &itemset, const map< ItemSet, int > &SetSupport, ItemSet &P1, ItemSet &P2, ItemSetIter iter,  const double min_conf ) {
	if ( iter == itemset.end() ) {
		if ( !P1.empty() && !P2.empty() ) {
			double p;
			// rule: P1 => P2
			p = double( ( *SetSupport.find( itemset ) ).second ) / ( *SetSupport.find( P1 ) ).second;
			if ( p >= min_conf ) {
				for ( ItemSetIter it = P1.begin(); it != P1.end(); ++it )
					outfile << *it << " ";
				outfile << "=>";
				for ( ItemSetIter it = P2.begin(); it != P2.end(); ++it )
					outfile << " " << *it;
				outfile << '(' << p << ')' << endl;
			}
			// rule: P2 => P1
			p = double( ( *SetSupport.find( itemset ) ).second ) / ( *SetSupport.find( P2 ) ).second;
			if ( p >= min_conf ) {
				for ( ItemSetIter it = P2.begin(); it != P2.end(); ++it )
					outfile << *it << " ";
				outfile << "=>";
				for ( ItemSetIter it = P1.begin(); it != P1.end(); ++it )
					outfile << " " << *it;
				outfile << '(' << p << ')' << endl;
			}
		}
		return;
	}
	P1.insert( *iter );
	partition( outfile, itemset, SetSupport, P1, P2, ++iter, min_conf );
	P1.erase( *(--iter) );
	P2.insert( *iter );
	partition( outfile, itemset, SetSupport, P1, P2, ++iter, min_conf );
	P2.erase( *(--iter) );
}
