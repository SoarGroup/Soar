// DON'T put include guards around this
// it must be uniquely included for every cpp file that uses it,
// since what msg is defined as changes

// CLASS_TOKEN and ENABLE_DEBUG must be defined before this file is included


#define msg cout << Sorts::frame << " " << CLASS_TOKEN << "(" << this << "): "

// the compiler should take out the if statement, since ENABLE_DEBUG is a
// constant.
#define dbg if(DEBUG_OUTPUT) cout << Sorts::frame << " " << CLASS_TOKEN << "(" << this << ")[d]: "
