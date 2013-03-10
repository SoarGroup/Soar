#ifndef CLI_PROXY_H
#define CLI_PROXY_H

#include <vector>
#include <map>
#include <ostream>

class proxied;

class cliproxy {
public:
	cliproxy();
	virtual ~cliproxy();
	virtual void use(const std::vector<std::string> &args, std::ostream &os);
	
	void add(const std::string &name, cliproxy *child);
	void add(const std::string &name, proxied *child);
	void del(const std::string &name);
	void rename(const std::string &from, const std::string &to);
	cliproxy *find(const std::string &path);
	
private:
	void add(const std::string &name, cliproxy *child, bool own);
	cliproxy *findp(int i, const std::vector<std::string> &path);
	
	struct child {
		bool own;
		cliproxy *p;
		
		child() : own(false), p(NULL) {}
		~child() { if (own) { delete p; } }
	};
	
	std::map<std::string, child> children;
};

class int_proxy : public cliproxy {
public:
	int_proxy(int *p);
	void use(const std::vector<std::string> &args, std::ostream &os);
	
private:
	int *p;
};

class bool_proxy : public cliproxy {
public:
	bool_proxy(bool *p);
	void use(const std::vector<std::string> &args, std::ostream &os);
	
private:
	bool *p;
};

template<typename T>
class memfunc_proxy : public cliproxy {
public:
	typedef void (T::*farg_ptr)(const std::vector<std::string> &args, std::ostream &os);
	typedef void (T::*fout_ptr)(std::ostream &os);
	typedef void (T::*fargc_ptr)(const std::vector<std::string> &args, std::ostream &os) const;
	typedef void (T::*foutc_ptr)(std::ostream &os) const;
	
	memfunc_proxy(T *p, farg_ptr f)        : p(p), pc(NULL), farg(f),    fargc(NULL), fout(NULL), foutc(NULL) {}
	memfunc_proxy(T *p, fout_ptr f)        : p(p), pc(NULL), farg(NULL), fargc(NULL), fout(f),    foutc(NULL) {}
	memfunc_proxy(const T *p, fargc_ptr f) : p(NULL), pc(p), farg(NULL), fargc(f),    fout(NULL), foutc(NULL) {}
	memfunc_proxy(const T *p, foutc_ptr f) : p(NULL), pc(p), farg(NULL), fargc(NULL), fout(NULL), foutc(f)    {}
	
	void use(const std::vector<std::string> &args, std::ostream &os) {
		if (farg) {
			(p->*farg)(args, os);
		} else if (fout) {
			(p->*fout)(os);
		} else if (fargc) {
			(pc->*fargc)(args, os);
		} else if (foutc) {
			(pc->*foutc)(os);
		}
	}

private:
	T *p;
	const T *pc;
	farg_ptr  farg;
	fout_ptr  fout;
	fargc_ptr fargc;
	foutc_ptr foutc;
};

class proxied {
public:
	cliproxy *get_proxy() { return &proxy; }
	void proxy_add(const std::string &name, cliproxy *p)    { proxy.add(name, p); }
	void proxy_add(const std::string &name, proxied *child) { proxy.add(name, child); }
	void proxy_del(const std::string &name)                 { proxy.del(name); }
private:
	cliproxy proxy;
};

#endif
