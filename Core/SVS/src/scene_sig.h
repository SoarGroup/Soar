#ifndef SCENE_SIG_H
#define SCENE_SIG_H

#include <vector>
#include <string>
#include "serializable.h"

class scene_sig : public serializable {
public:
	class entry : public serializable {
	public:
		entry() : id(-1), type("obj"), start(-1) {}
		
		bool operator==(const entry &e) const {
			/*
			 don't need to compare names or props, the equivalence of those
			 should follow from the equivalence of id and type
			*/
			return id == e.id && type == e.type && start == e.start;
		}
		
		bool similar(const entry &e) const {
			return type == e.type && start == e.start;
		}
		
		void serialize(std::ostream &os) const;
		void unserialize(std::istream &is);

		int id;
		int start;
		std::string type;
		std::string name;
		std::vector<std::string> props;
	};

	scene_sig() {}
	
	entry &operator[](int i)                    { return s.at(i); }
	const entry &operator[](int i) const        { return s.at(i); }
	int size() const                            { return s.size(); } 
	bool empty() const                          { return s.empty(); } 
	void clear()                                { s.clear(); }
	bool operator==(const scene_sig &sig) const { return s == sig.s; }
	bool similar(const scene_sig &sig) const;
	int find_id(int id) const;

	void serialize(std::ostream &os) const;
	void unserialize(std::istream &is);
	void print(std::ostream &os) const;
	int dim() const;
	void add(const entry &e);
	bool get_dim(const std::string &obj, const std::string &prop, int &obj_ind, int &prop_ind) const;
private:
	std::vector<entry> s;
};

#endif
