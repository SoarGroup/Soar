#ifndef DTREE_H
#define DTREE_H

#include <vector>
#include <map>
#include <string>
#include <memory>

typedef int category;
typedef std::vector<bool> boolvec;

class classifier_inst
{
    public:
        boolvec attrs;
        category cat;
};

class ID5Tree
{
    public:
        ID5Tree(const std::vector<classifier_inst>& insts, int nattrs);
        void update_tree(int i);
        void update_counts_change(int i, category old);
        void batch_update();
        category classify(const boolvec& attrs) const;
        void output(const std::vector<std::string>& attr_names) const;
        int size() const;
        void get_all_splits(std::vector<int>& splits) const;
        void print_graphviz(std::ostream& os) const;
        void print(const std::string& prefix, std::ostream& os) const;
        bool cli_inspect(int nid, std::ostream& os) const;
        
        void prune();
        
        // Return node that best matches attribute vector
        const ID5Tree* get_matched_node(const boolvec& attrs) const;
        
        void get_instances(std::vector<int>& i) const;
        void get_categories(std::vector<int>& c) const;
        
    private:
        //typedef std::unique_ptr<ID5Tree> ID5ptr;
        typedef std::auto_ptr<ID5Tree> ID5ptr;
        
        ID5Tree(const std::vector<classifier_inst>& insts, const std::vector<int>& attrs);
        void expand();
        void shrink();
        void remove_empty();
        void pull_up(int i);
        void pull_up_repair();
        void update_counts_new(int i);
        void reset_counts();
        void update_counts_from_children();
        int choose_split();
        void update_gains();
        bool cats_all_same() const;
        bool attrs_all_same() const;
        bool expanded() const;
        bool empty() const;
        bool validate_counts();
        void output_rec(const std::string& prefix, const std::vector<std::string>& attr_names) const;
        void batch_update(const std::vector<int>& new_insts);
        category best_cat();
        
        /*
         Instance category counts for each possible value of an attribute
         (true/false) and each possible category.
        */
        struct val_counts
        {
            int ttl_true;
            int ttl_false;
            std::map<category, int> true_counts;
            std::map<category, int> false_counts;
            
            bool operator==(const val_counts& c) const
            {
                return (ttl_true == c.ttl_true &&
                        ttl_false == c.ttl_false &&
                        true_counts == c.true_counts &&
                        false_counts == c.false_counts);
            }
            
            val_counts()
            {
                ttl_true = 0;
                ttl_false = 0;
            }
        };
        
        int id;
        // class counts for every attribute and every value
        std::map<int, val_counts> av_counts;
        std::map<category, int> ttl_counts;
        std::map<int, double> gains;
        const std::vector<classifier_inst>& insts;
        std::vector<int> insts_here;
        std::vector<int> attrs_here;
        category cat;
        int split_attr;
        ID5ptr left;
        ID5ptr right;
};


#endif
