#ifndef CLI_PROXY_H
#define CLI_PROXY_H

#include <vector>
#include <map>
#include <ostream>

class cliproxy
{
    public:
        virtual ~cliproxy();
        void proxy_use(const std::string& path, const std::vector<std::string>& args, std::ostream& os);
        bool temporary() const
        {
            return false;
        }
        cliproxy& set_help(const std::string& t);
        cliproxy& add_arg(const std::string& arg, const std::string& help);
        
    private:
        void list_children(int level, std::ostream& os);
        void print_help(std::ostream& os) const;
        
        virtual void proxy_get_children(std::map<std::string, cliproxy*>& c) {}
        virtual void proxy_use_sub(const std::vector<std::string>& args, std::ostream& os)
        {
            list_children(0, os);
        }
        
        std::string help_text;
        std::vector<std::string> args_help;
};

class int_proxy : public cliproxy
{
    public:
        int_proxy(int* p, const std::string& description);
        void proxy_use_sub(const std::vector<std::string>& args, std::ostream& os);
        bool temporary() const
        {
            return true;
        }
        
    private:
        int* p;
};

class float_proxy : public cliproxy
{
    public:
        float_proxy(double* p, const std::string& description);
        void proxy_use_sub(const std::vector<std::string>& args, std::ostream& os);
        bool temporary() const
        {
            return true;
        }
        
    private:
        double* p;
};

class bool_proxy : public cliproxy
{
    public:
        bool_proxy(bool* p, const std::string& description);
        void proxy_use_sub(const std::vector<std::string>& args, std::ostream& os);
        bool temporary() const
        {
            return true;
        }
        
    private:
        bool* p;
};

template<typename T>
class memfunc_proxy : public cliproxy
{
    public:
        typedef void (T::*farg_ptr)(const std::vector<std::string>& args, std::ostream& os);
        typedef void (T::*fout_ptr)(std::ostream& os);
        typedef void (T::*fargc_ptr)(const std::vector<std::string>& args, std::ostream& os) const;
        typedef void (T::*foutc_ptr)(std::ostream& os) const;
        
        memfunc_proxy(T* p, farg_ptr f)        : p(p), pc(NULL), farg(f),    fargc(NULL), fout(NULL), foutc(NULL) {}
        memfunc_proxy(T* p, fout_ptr f)        : p(p), pc(NULL), farg(NULL), fargc(NULL), fout(f),    foutc(NULL) {}
        memfunc_proxy(const T* p, fargc_ptr f) : p(NULL), pc(p), farg(NULL), fargc(f),    fout(NULL), foutc(NULL) {}
        memfunc_proxy(const T* p, foutc_ptr f) : p(NULL), pc(p), farg(NULL), fargc(NULL), fout(NULL), foutc(f)    {}
        
        void proxy_use_sub(const std::vector<std::string>& args, std::ostream& os)
        {
            if (farg)
            {
                (p->*farg)(args, os);
            }
            else if (fout)
            {
                (p->*fout)(os);
            }
            else if (fargc)
            {
                (pc->*fargc)(args, os);
            }
            else if (foutc)
            {
                (pc->*foutc)(os);
            }
        }
        
        bool temporary() const
        {
            return true;
        }
        
    private:
        T* p;
        const T* pc;
        farg_ptr  farg;
        fout_ptr  fout;
        fargc_ptr fargc;
        foutc_ptr foutc;
};

class proxy_group : public cliproxy
{
    public:
        proxy_group() {}
        void add(const std::string& name, cliproxy* p);
        bool temporary() const
        {
            return true;
        }
        
    private:
        void proxy_get_children(std::map<std::string, cliproxy*>& c);
        
        std::map<std::string, cliproxy*> children;
};

#endif
