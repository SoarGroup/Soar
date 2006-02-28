//These includes will change
//#include "Global.H"
//#include "Map.H"


typedef struct {
	int x;
	int y;
	} Location;

class AStarNode{
 public:
	AStarNode();
	virtual ~AStarNode();

	void setCoordinates(int, int);

	void setParent(AStarNode *);
	AStarNode *getParent();

	void setMoveCost(int g);
	void setGoalNode(int, int);
	int getNodeValue();

 private:
	Location loc;

	int g;	//Cost to move here
	int h;	//Direct distance (squared likely)

 	AStarNode *parent;
};
