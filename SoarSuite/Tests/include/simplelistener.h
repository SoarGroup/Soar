#ifndef SIMPLE_LISTENER_H
#define SIMPLE_LISTENER_H

// Listen for remote commands and live
// for "life" 10'ths of a second (e.g. 10 = live for 1 second)
class SimpleListener 
{
public:
	SimpleListener( int life, int port );

	int run(); // returns zero on success, nonzero failure

private:
	int life;
	int port;

	// Choose how to connect (usually use NewThread) but for
	// testing currentThread can be helpful.
	bool useCurrentThread;
};


#endif // SIMPLE_LISTENER_H
