// For header, better always include, gSKI need this compile argument too
//#ifdef SEMANTIC_MEMORY

#ifndef _SMEM_
#define _SMEM_



#include <map>
#include <set>
#include <vector>
//#include <ext/hash_map>
#include <string>
#include <iostream>

#ifdef _MSC_VER
#pragma warning (disable : 4503)
#include <hash_map>
#else
#include <ext/hash_map>
namespace stdext= ::__gnu_cxx;
#endif

using std::map;
using std::set;
using std::vector;
using std::string;
using std::cout;
using std::ostream;
using std::endl;
using std::pair;
using stdext::hash_map;
//using stdext::hash_compare;
using std::less;
using std::allocator;

#ifndef _MSC_VER
#ifndef HASH_MAP_DEF
#define HASH_MAP_DEF
//this was lifted from a reply to a porting question found by googling...
namespace __gnu_cxx
{
  template <> struct hash<string>
  {
    size_t operator () (const string& x ) const
    {
      return hash<char const*>()(x.c_str());
    }
  };
}
#endif
#endif


unsigned long StringToUnsignedLong(string str);

// These need to be consistent
#define VARIABLE_SYMBOL_TYPE 0
#define IDENTIFIER_SYMBOL_TYPE 1
#define SYM_CONSTANT_SYMBOL_TYPE 2
#define INT_CONSTANT_SYMBOL_TYPE 3
#define FLOAT_CONSTANT_SYMBOL_TYPE 4
#define NUM_SYMBOL_TYPES 5
#define NUM_PRODUCTION_TYPES 4


// long term memory element
bool is_identifier_type(int value_type);

template<class T>
set<T> set_intersect(const set<T>&, const set<T>&);


string remove_quote(string str);

class LME
{
public:
	string id;
	string attr;
	string value;
	int value_type;
	vector<int> boost_history;
	LME(string input_id, string input_attr, string input_value, 
		int input_value_type, const vector<int>& history = vector<int>()){
		id = input_id;
		attr = input_attr;
		value = remove_quote(input_value);
		value_type = input_value_type;
		boost_history = history;
	}
	// this determines the uniqueness of a LME
	bool operator < (const LME lme) const;
	//... all required data structure
	~LME(){
		
		//cout << id<<","<<attr<<","<<value<<","<<value_type<<"being destroied\n";
	}
};

// CueTriplet is the input to match with long term memory
// It might be very close to LME strucure, but not necessarily the same
class CueTriplet
{
public:
	string id;
	string attr;
	string value;
	int value_type;
	CueTriplet(string input_id, string input_attr, string input_value, int input_value_type){
		id = input_id;
		attr = input_attr;
		value = remove_quote(input_value);
		value_type = input_value_type;
	}
	// this determines the uniqueness of a LME
	bool operator < (const CueTriplet t) const;
	//... all required data structure
};


typedef stdext::hash_map<string, int> HASH_S_LP;
typedef stdext::hash_map<string, HASH_S_LP> HASH_S_HASH_S_LP;
typedef stdext::hash_map<string, HASH_S_HASH_S_LP> HASH_S_HASH_S_HASH_S_LP;


class SemanticMemory
{
	public:
		bool debug_output;
		SemanticMemory(){debug_output = false;};
		void insert_LME (string id, string attr, string value, 
			int value_type, const vector<int>& history = vector<int>());

		// This one should be the external insert function called.
		// Need to merge identical chunks recursively
		void merge_LMEs(vector<LME>& lmes, long current_cycle);

		string merge_id(string& id, HASH_S_HASH_S_HASH_S_LP& id_atrr_value_hash, 
			hash_map<string, string>& merging_hash,
			vector<LME>& all_new_lmes,
			set<string>& merging_path, long& current_cycle);
		
		bool find_identical_chunk (string& chunk_id, HASH_S_HASH_S_HASH_S_LP& id_attr_value_hash,
			string& new_chunk_id, vector<LME>& all_new_lmes);

		bool test_id(const string id);
		
		// this matching function assumes single level cue structure, and exact match
		// this is intended to be the simplified version for testing purpose only
		set<string> match_retrieve_single_level(const set<CueTriplet>&);
		
		set<CueTriplet> match_retrieve_single_level_2006_1_22(const set<CueTriplet>&, string&);
		set<CueTriplet> match_retrieve_single_level_2006_1_31(const set<CueTriplet>&, string&);
		bool match_retrieve_single_level_2006_3_15(const set<CueTriplet>&, string&, set<CueTriplet>& result, float& confidence, float& experience);
		
		// I'll need a partial match function (on exemplars) to compare with clustering approach
		// The threshold can be varied so that it can achieve hierachical clustering by exemplars.
		// However, exemplars won't have much useful meta-information available, which has to be achieved by reducing the dimension of represetnation - prototype
		bool partial_match(const set<CueTriplet>&, string&, set<CueTriplet>& result, float& threshold, float& confidence, float& experience);
		
		// Given the id, expands other attributes according to the given cue (optional)
		// If the attribute is in the cue, then expand that attribute regardless of activation
		//		set<LME> expand_id(string id, set<CueTriplet>& cue = set<CueTriplet>());

		void dump(ostream& out);
		void dump(vector<LME*>& out);
		void print();
		int clear();

		int get_chunk_count() {return memory_id_attr_hash.size();};
		int get_lme_count(){return LME_Array.size();};

		HASH_S_HASH_S_HASH_S_LP get_id_attr_hash(){return memory_id_attr_hash;};
		LME* getLME_ptr(int index){return this->LME_Array[index];};
		
		int reset_history(); // after init agent, the history need better restarted, although the knowledge is still there
								// This is reasonable.
								// On the other hand, decidion cycle number is restarted as well.

		~SemanticMemory();
//	private:
		// given attribute and value, find the id with the matching attr-value pair
		set<string> match_attr_value(const string attr, const string value, int value_type);
	private:		
		// given attribute and value, find the indexes of LME with the matching attr-value pair
		// value must be either constant or long-term identifier and shouldn't be temporary identifier
		set<int> exact_match_attr_value(const string attr, const string value, int value_type);

		// given id and attribute, retrieve the index for corresponding LMEs
		set<int> match_id_attr(const string id, const string attr);
		
		int insert_LME_hash (HASH_S_HASH_S_HASH_S_LP& hash, string key1, string key2, string key3, int, int value_type);
		
		
		
		// primary structure,
		//key is id, value is a hash of attribute to LMEs
		//HASH_S_HASH_S_SET memory_id_attr_hash;
		HASH_S_HASH_S_HASH_S_LP memory_id_attr_hash;
		// utility structure, 
		//key is attribute, value is a hash of value to LMEs
		//HASH_S_HASH_S_SET memory_attr_value_hash;
		HASH_S_HASH_S_HASH_S_LP memory_attr_value_hash;
		
		vector<LME*> LME_Array;
		
	
};

template <class T>
ostream& operator << (ostream &out, const set<T>& s);

ostream& operator << (ostream &out, const LME& lme);

ostream& operator << (ostream &out, const set<LME>& s);

ostream& operator << (ostream &out, const CueTriplet& c);

ostream& operator << (ostream &out, const set<CueTriplet>& c);


class TwoDimArray{
public:
private:
	vector< vector<int> > array;
	stdext::hash_map<string, int> dim1_to_index;
	stdext::hash_map<string, int> dim2_to_index;

};

#endif /* _SMEM_ */

//#endif SEMANTIC_MEMORY
