#include <iostream>
#include <armadillo>

using namespace std;
using namespace arma;

int main() {
	mat m(3, 3);
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 3; ++j) {
			m(i, j) = i * 4 + j;
		}
	}
	cout << "old" << endl << "---------" << endl << m << endl;
	m.reshape(4, 3);
	cout << "new" << endl << "---------" << endl << m << endl;
	return 0;
}
