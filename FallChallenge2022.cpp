#include <ios>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <bits/stdc++.h>

using namespace std;

enum Coord {
	X,
	Y
};

enum side {
    LEFT = -1,
    RIGHT = 1
};

enum Owner {
	NEUTRAL = -1,
	OPP,
	ME
};

enum settings {
    UNSET = -1,
    FALSE,
    TRUE
};

# define safetyON true
# define safetyOFF false

typedef pair<int,int> Pair;
typedef pair<int, pair<int, int>> pPair;
typedef pair<int, ostringstream> aPair;

typedef struct s_cell {
    int     x;
    int     y;
    int     scrap_amount;
    int     owner;
    int     units;
    int     unitsSpawned;
    bool    recycler;
    bool    can_build;
    bool    can_spawn;
    bool    in_range_of_recycler;
    bool    has_moved;
    int     spawnValue;
    int     spawnPriority;
    int     targetPriority;
    bool    alreadyTargeted;
    int     reinforce;
    int     threat;
    int     stand;
    Pair    cavalry;
    bool    unitsAvailable;
    int     menace;
}   t_cell;

typedef struct s_node {
    int     parent_x;
    int     parent_y;
    int     f;
    int     g;
    int     h;
}   t_node;

class Brain {
	private:

	public:
		Brain( int w, int h );
		~Brain( void );

		void			endTurn( void );
		vector<t_cell>	sonar( t_cell & src, bool safety );
		vector<t_cell>	closeRange( t_cell & cell, t_cell * src, bool safety);
        vector<t_cell>  closeRangeSouth( t_cell & cell, t_cell * src, bool safety);
        vector<t_cell>  closeRangeNorth( t_cell & cell, t_cell * src, bool safety);


		static bool		sonarForLoop( int x, int delta );
        
        bool            isValid( int x, int y);
        bool            isBlocked( t_cell & cell );
        bool            isDanger( t_cell & cell, t_cell & src, bool safety);
        bool            isBlockedorDanger( t_cell & cell, t_cell & src , bool safety);
        int             myInRangeRecycler( t_cell & cell );
        bool            isTarget(t_cell cell, bool axis, int deltaX, int deltaY, int move, t_cell src);
        bool            isDeadEnd( t_cell & cell, t_cell & src);
        bool            isOppWay(t_cell & cell);

        void            scanSafety( t_cell & cell );
        void            updateTargetStatus( t_cell & cell, bool status);
        void            updateRecyclerStatus( t_cell & cell, bool status );
        void            updateSpawnPriority( t_cell & cell, int priority );

        Pair            AstarSearch(t_cell cell);
        bool            hasAWay(t_cell cell, int owner);
        Pair            searchAllies(t_cell cell);

        bool            isNextOPP(t_cell & cell);

        int             checkEnemySquad( t_cell cell, bool reinforce );
        int             checkCellSpawnPriority( t_cell cell );
        vector<t_cell>  checkPriority( void );
        int             scanDominion( t_cell & cell);
        int             checkRessources( t_cell & cell );

        bool            CavalryCall( t_cell & cell );
        int             countCavalry( t_cell & cell);
        bool            Battlecry( t_cell & cell );
        bool            isNeeded( t_cell & cell );
        vector<t_cell>  targetEnemy( t_cell & cell);
		
        t_cell          *getCell( int x, int y );
        
		int				width;
		int 			height;
		int				turn;
		int				dX;
		int				dY;

		vector<t_cell>  cells;
    	vector<t_cell>  my_cells;
    	vector<t_cell>  opp_cells;
    	vector<t_cell>  neutral_cells;
    	vector<t_cell>  my_units;
    	vector<t_cell>  opp_units;
    	vector<t_cell>  my_recyclers;
    	vector<t_cell>  opp_recyclers;
    	vector<t_cell>  target_cells;
		
		t_cell			my_highest;
		t_cell			my_lowest;
        int             my_left;
        int             my_right;
		t_cell			opp_highest;
		t_cell			opp_lowest;
        int             opp_left;
        int             opp_right;
        int             units_up;
        int             units_down;
        int             opp_matter;

		bool			crossed;
		bool			north;
		bool			south;
        bool            oppBehind;
        bool            noWay;


};

std::ostream	&operator<<( std::ostream & ostream, t_cell const & src ){
	ostream << src.x << " " << src.y;
	return (ostream);
}

bool    isStraight(int dX, int dY) {
    if (dX == 0 or dY == 0)
        return true;
    else
        return false;
}

bool sortTarget(t_cell i, t_cell j) { return (i.menace > j.menace); }

bool sortReinforce(t_cell i, t_cell j) { return (i.reinforce > j.reinforce); }

bool sortLEFT(t_cell i, t_cell j) { return (i.x < j.x); }

bool sortRIGHT(t_cell i, t_cell j) { return (i.x > j.x); }

bool sortUnits(t_cell i, t_cell j) { return (i.units > j.units); }

bool sortPriority(t_cell i, t_cell j) { return (i.spawnPriority > j.spawnPriority); }

Brain::Brain( int w, int h ) : width(w), height(h), turn(0), crossed(false), north(false), south(false), dX(0), dY(0), my_left(w), opp_left(w), my_right(-1), opp_right(-1), oppBehind(false), noWay(false), units_up(0), units_down(0) {
	this->cells.clear();
	
	this->my_cells.clear();
	this->opp_cells.clear();
	this->neutral_cells.clear();
	
	this->my_units.clear();
	this->opp_units.clear();

	this->my_recyclers.clear();
	this->opp_recyclers.clear();

	this->target_cells.clear();
}

Brain::~Brain( void ) {

}

void	Brain::endTurn( void ) {
	this->cells.clear();
	
	this->my_cells.clear();
	this->opp_cells.clear();
	this->neutral_cells.clear();
	
	this->my_units.clear();
	this->opp_units.clear();

	this->my_recyclers.clear();
	this->opp_recyclers.clear();

	this->target_cells.clear();

    this->units_down = 0;
    this->units_up = 0;
    this->opp_left = width;
    this->opp_right = -1;
    this->my_left = width;
    this->my_right = -1;
}

void    Brain::updateRecyclerStatus( t_cell & cell, bool status ) {
    int x = cell.x;
    int y = cell.y;
    
    cells[(y * width) + x].recycler = status;

    for (vector<t_cell>::iterator it = this->my_cells.begin(); it != this->my_cells.end(); it++) {
        if (it->x == x and it->y == y) {
            it->recycler = status;
            break;
        }
    }
}

void    Brain::updateTargetStatus( t_cell & cell, bool status ) {
    int x = cell.x;
    int y = cell.y;

    cells[(y * width) + x].alreadyTargeted = status;
}

void    Brain::updateSpawnPriority( t_cell & cell, int Priority ) {
    int x = cell.x;
    int y = cell.y;

    cells[(y * width) + x].spawnPriority = -1;
}

vector<t_cell>	Brain::sonar( t_cell & src, bool safety) {
	vector<t_cell>	target;
    vector<t_cell>  empty;
    bool            verbose = src.x == 11 and src.y == 4;
    empty.clear();
	bool            closeOPPsouth = false;
    bool            closeOPPnorth = false;
    bool            under = src.y > this->opp_lowest.y + 1;
    bool            onTop = src.y < this->opp_highest.y - 1;
	
    if (dX == LEFT) {
        if (src.y < height / 2)
            closeOPPsouth = src.x > this->opp_right;
        else
            closeOPPnorth = src.x > this->opp_right; 
    }
    else {
        if (src.y < height / 2) {
            closeOPPsouth = src.x < this->opp_left;
        }
        else
            closeOPPnorth = src.x < this->opp_left; 
    }

    if (closeOPPsouth or (!under and !south and src.y == this->my_lowest.y)) {
        target = closeRangeSouth(src, NULL, safety);
    }
    else if (closeOPPnorth or (!onTop and !north and src.y == this->my_highest.y)) {
        target = closeRangeNorth(src, NULL, safety);
    }
    else {
        target = closeRange(src, NULL, safety);
    }

	if (target.size() == 1)
		return (target);
	else if (target.size() > 0) {
		vector<t_cell>  longRange;
        for (vector<t_cell>::iterator it = target.begin(); it != target.end(); it++) {
			t_cell Current = *it;

			longRange = closeRange(Current, &src, safetyOFF);

			if (longRange.size() == 1) {
                target.clear();
                target.emplace_back(Current);
				return (target);
            }
			longRange.clear();
		}
	}
	return (empty);
}

bool Brain::sonarForLoop(int x, int delta) {
    if (delta == -1)
        return (x >= -1);
    else
        return (x <= 1);
}

t_cell  *Brain::getCell( int x, int y ) { return (&this->cells[(y * this->width) + x]);}

bool    Brain::isValid (int x, int y ) {
    return ((x >= 0) && (x < this->width) && (y >= 0) && (y < this->height));
}

bool    Brain::isBlocked(t_cell & cell) {
    if (cell.recycler == 1 or cell.scrap_amount == 0)
        return (true);
    else
        return (false);
}

bool    Brain::isDeadEnd(t_cell &cell, t_cell &src) {

    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            bool axis = isStraight(j, i);
            if ((i == 0 and j == 0) or !axis)
                continue;
            int nY = cell.y + i;
            int nX = cell.x + j;
            if (!isValid(nX, nY)) {
                continue;
            }
            if (nX == src.x and nY == src.y)
                continue;
            t_cell Current = cells[(nY * width) + nX];
            if (Current.scrap_amount > 0 and !Current.recycler and !(Current.in_range_of_recycler and Current.scrap_amount <= 2) and Current.owner != ME)
                return (false);
        }
    }
    if (cell.owner == OPP and !cell.recycler and cell.units < src.units)
        return (false);
    return (true);
}

int    Brain::countCavalry( t_cell & cell) {
    int reinforcement = 0;
    bool verbose = cell.x == 6 and cell.y == 3;

    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            bool axis = isStraight(j, i);
            if ((i == 0 and j == 0) or !axis)
                continue;
            int nY = cell.y + i;
            int nX = cell.x + j;
            if (!isValid(nX, nY))
                continue;
            
            bool behindEnemyLine = false;    
            if (!this->oppBehind) {
                if (this->dX == LEFT) {
                    behindEnemyLine = this->opp_left <= cell.x;
                }
                else {
                    behindEnemyLine = this->opp_right >= cell.x;
                }
            }

            bool behindMe = false;
            if ( !behindEnemyLine) {
                if (dX == RIGHT and j == 1)
                    behindMe = true;
                else if (dX == LEFT and j == -1)
                    behindMe = true;
            }
            
            
            t_cell Current = cells[(nY * width) + nX];

            if (Current.owner == ME and Current.unitsAvailable == true and !behindMe)
                reinforcement += Current.units;
        }
    }

    return (reinforcement);
}

bool    Brain::isNeeded( t_cell & cell ) {

    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            bool axis = isStraight(j, i);
            if ((i == 0 and j == 0) or !axis)
                continue;
            int nY = cell.y + i;
            int nX = cell.x + j;
            if (!isValid(nX, nY))
                continue;
            
            t_cell Current = cells[(nY * width) + nX];
            
            if (Current.owner != ME or Current.recycler)
                continue;
            
            int threat = this->checkEnemySquad(Current, false);

            if (Current.owner == ME and Current.units > 0 and Current.unitsAvailable) {
                if (threat > 0 and this->CavalryCall(Current)) {
                    return (true);
                }
            }
        }
    }
    return (false);
}

bool    Brain::CavalryCall( t_cell & cell) {
    cell.units = this->cells[(cell.y * width) + cell.x].units;
    cell.unitsSpawned = this->cells[(cell.y * width) + cell.x].unitsSpawned;
    int     defense = cell.units + cell.unitsSpawned;
    int     threat = this->checkEnemySquad(cell, false);
    int     cavalry = this->countCavalry(cell);
    bool    callCavalry = threat <= (defense + cavalry) and !(threat < defense);
    int     newUnits = cell.units;
    int     loop = true;
    
    if (!callCavalry) {
        cells[(cell.y * width) + cell.x].stand = FALSE;
        return (false);
    }

    cells[(cell.y * width) + cell.x].stand = TRUE;
    cells[(cell.y * width) + cell.x].unitsAvailable = false;
    
    for (int i = -1; i <= 1 and loop; i++) {
        for (int j = -1; j <= 1 and loop; j++) {
            bool axis = isStraight(j, i);
            if ((i == 0 and j == 0) or !axis)
                continue;
            int nY = cell.y + i;
            int nX = cell.x + j;
            if (!isValid(nX, nY))
                continue;
            
            bool behindEnemyLine = false;    
            if (!this->oppBehind) {
                if (this->dX == LEFT) {
                    behindEnemyLine = this->opp_left <= cell.x;
                }
                else {
                    behindEnemyLine = this->opp_right >= cell.x;
                }
            }

            bool behindMe = false;
            if ( !behindEnemyLine) {
                if (dX == RIGHT and j == 1)
                    behindMe = true;
                else if (dX == LEFT and j == -1)
                    behindMe = true;
            }
            t_cell *Current = &cells[(nY * width) + nX];

            if (Current->owner == ME and Current->units > 0 and Current->unitsAvailable and !behindMe) {
                Current->cavalry.first = cell.x;
                Current->cavalry.second = cell.y;
                Current->unitsAvailable = false;
                newUnits += Current->units;
                if ((newUnits + cell.unitsSpawned) >= threat) {
                    cells[(cell.y * width) + cell.x].units = newUnits;
                    loop = false;
                }
            }
        }
    }
    return (true);
}

bool    Brain::Battlecry( t_cell & cell) {
    cell.units = this->cells[(cell.y * width) + cell.x].units;
    int     defense = cell.units + this->checkEnemySquad(cell, false);
    int     cavalry = this->countCavalry(cell);
    bool    callCavalry =  defense < cavalry and !cell.recycler;
    int     troops = 0;
    int     squad = 0;
    int     loop = true;
    vector<t_cell>  recruits;

    if (!callCavalry) {
        cells[(cell.y * width) + cell.x].stand = FALSE;
        return (false);
    }

    cells[(cell.y * width) + cell.x].stand = TRUE;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1 and loop; j++) {
            bool    behindEnemyLine = false;
            bool axis = isStraight(j, i);
            if ((i == 0 and j == 0) or !axis)
                continue;
            int nY = cell.y + i;
            int nX = cell.x + j;
            if (!isValid(nX, nY))
                continue;

            bool behindMe = false;

            t_cell *Current = &cells[(nY * width) + nX];

            bool verbose = Current->x == 8 and Current->y == 4;

            if (!this->oppBehind) {
                if (this->dX == LEFT) {
                    behindEnemyLine = this->opp_right <= Current->x;
                }
                else {
                    behindEnemyLine = this->opp_left >= Current->x;
                }
            }

            if ( !behindEnemyLine ) {
                if (dX == RIGHT and j == 1)
                    behindMe = true;
                else if (dX == LEFT and j == -1)
                    behindMe = true;
            }

            if (Current->owner == ME and Current->units > 0 and Current->unitsAvailable and !behindEnemyLine and !behindMe) {
                recruits.emplace_back(*Current);
            }
        }
    }

    sort(recruits.begin(), recruits.end(), sortUnits);

    for (vector<t_cell>::iterator calling = recruits.begin(); calling != recruits.end() and loop; calling++) {
        calling->cavalry.first = cell.x;
        calling->cavalry.second = cell.y;
        calling->unitsAvailable = false;
        troops += calling->units;
        if (troops > defense) {
            loop = false;
        }
    }

    if (troops == 0)
        return (false);
    return (true);
}

vector<t_cell>	Brain::closeRange( t_cell & cell, t_cell *src, bool safety) {
	vector<t_cell> target;
	vector<t_cell> path;
    bool verbose = cell.x == 16 and cell.y == 3;
    bool lastLine = false;
    int deltaY = this->dY;
    bool    frontLine = false;
    
    if (!this->oppBehind) {
        if (this->dX == LEFT)
            lastLine = cell.x <= this->my_left;
        else
            lastLine = cell.x >= this->my_right;
    }
    if (this->dX == LEFT)
        frontLine = cell.x == this->my_right;
    else
        frontLine = cell.x == this->my_left;

    if (frontLine) {
        if (this->units_up < this->units_down)
            deltaY = 1;
        else if (this->units_up > this->units_down)
            deltaY = -1;
    }

	for (int x = -1 * dX; sonarForLoop(x, dX); x += dX) {
        for (int y = -1 * deltaY; sonarForLoop(y, deltaY); y += deltaY) {
            bool axis = isStraight(x, y);
            if ((x == 0 and y == 0) or !axis)
                continue;
            if (src and x == src->x and y == src->y)
                continue;
            int nY = cell.y + y;
            int nX = cell.x + x;
            if (!isValid(nX, nY))
                continue;
            
            bool _verbose = nX == -1 and nY == 7;
            if (lastLine) {
                if (this->dX == LEFT and x == -1)
                    continue;
                else if ( this->dX == RIGHT and x == 1)
                    continue;
            }

            t_cell Current = this->cells[(nY * width) + nX];
            
            if (Current.owner != ME and !isBlockedorDanger(Current, cell, safety) and !Current.alreadyTargeted) {   
                if (!this->oppBehind and isDeadEnd(Current, cell))
                    continue;
                else {
                    Current.menace = this->checkEnemySquad(Current, false);
                    if (Current.owner == OPP)
                        Current.menace += this->opp_matter / 10;
                    target.emplace_back(Current);
                }
            }
            else if (!isBlockedorDanger(Current, cell, safety) and !src) {
                path.emplace_back(Current);
            }
        }
    }
    if (target.size() == 1)
        return (target);
    else if (target.size() > 1) {
        bool behindEnemyLine = false;    
        if (this->dX == LEFT) {
            behindEnemyLine = this->opp_left <= cell.x;
        }
        else {
            behindEnemyLine = this->opp_right >= cell.x;
        }
        //behindEnemyLine = true;
        path.clear();
        if (behindEnemyLine) {
            vector<t_cell> TargetSorted = target;
            sort(TargetSorted.begin(), TargetSorted.end(), sortTarget);
            for (vector<t_cell>::iterator find = TargetSorted.begin(); find != TargetSorted.end(); find++) {
                if (find->menace < cell.units) {
                    t_cell final = *find;
                    path.emplace_back(final);
                    break;
                }
            }
        }
        if (path.size() == 0) {
            t_cell final = *target.begin();
            path.emplace_back(final);
        }
    }
    return (path);
}

vector<t_cell>	Brain::closeRangeSouth( t_cell & cell, t_cell *src, bool safety) {
	vector<t_cell> target;
	vector<t_cell> path;
    bool lastLine = false;

    if (!this->oppBehind) {
        if (this->dX == LEFT)
            lastLine = cell.x <= this->my_left;
        else
            lastLine = cell.x >= this->my_right;
    }

	for (int y = 1; y >= -1; y--) {
        for (int x = -1 * dX; sonarForLoop(x, dX); x += dX) {
            bool axis = isStraight(x, y);
            if ((x == 0 and y == 0) or !axis)
                continue;
            if (src and x == src->x and y == src->y)
                continue;
            int nY = cell.y + y;
            int nX = cell.x + x;
            if (!isValid(nX, nY))
                continue;
            
            t_cell Current = this->cells[(nY * width) + nX];
            
            if (lastLine) {
                if (this->dX == LEFT and x == -1)
                    continue;
                else if ( this->dX == RIGHT and x == 1)
                    continue;
            }


            if (Current.owner != ME and !isBlockedorDanger(Current, cell, safety) and !Current.alreadyTargeted) {   
                if (!this->oppBehind and isDeadEnd(Current, cell))
                    continue;
                else {
                    target.emplace_back(Current);
                    return (target);
                }
            }
            else if (!isBlockedorDanger(Current, cell, safety) and !src) {
                path.emplace_back(Current);
            }
        }
    }
    if (target.size() == 1)
        return (target);
    else if (target.size() > 1) {
        bool behindEnemyLine = false;    
        if (this->dX == LEFT) {
            behindEnemyLine = this->opp_left <= cell.x;
        }
        else {
            behindEnemyLine = this->opp_right >= cell.x;
        }
        //behindEnemyLine = true;
        path.clear();
        if (behindEnemyLine) {
            vector<t_cell> TargetSorted = target;
            sort(TargetSorted.begin(), TargetSorted.end(), sortTarget);
            for (vector<t_cell>::iterator find = TargetSorted.begin(); find != TargetSorted.end(); find++) {
                if (find->menace < cell.units) {
                    t_cell final = *find;
                    path.emplace_back(final);
                    break;
                }
            }
        }
        if (path.size() == 0) {
            t_cell final = *target.begin();
            path.emplace_back(final);
        }
    }
    return (path);
}


vector<t_cell>	Brain::closeRangeNorth( t_cell & cell, t_cell *src, bool safety) {
	vector<t_cell> target;
	vector<t_cell> path;
    bool lastLine = false;

    if (!this->oppBehind) {
        if (this->dX == LEFT)
            lastLine = cell.x <= this->my_left;
        else
            lastLine = cell.x >= this->my_right;
    }

	for (int y = -1; y <= 1; y++) {
        for (int x = -1 * dX; sonarForLoop(x, dX); x += dX) {
            bool axis = isStraight(x, y);
            if ((x == 0 and y == 0) or !axis)
                continue;
            if (src and x == src->x and y == src->y)
                continue;
            int nY = cell.y + y;
            int nX = cell.x + x;
            if (!isValid(nX, nY))
                continue;
            
            t_cell Current = this->cells[(nY * width) + nX];

            if (lastLine) {
                if (this->dX == LEFT and x == -1)
                    continue;
                else if ( this->dX == RIGHT and x == 1)
                    continue;
            }
            
            if (Current.owner != ME and !isBlockedorDanger(Current, cell, safety) and !Current.alreadyTargeted) {   
                if (!this->oppBehind and isDeadEnd(Current, cell))
                    continue;
                else {
                    target.emplace_back(Current);
                    return (target);
                }
            }
            else if (!isBlockedorDanger(Current, cell, safety) and !src) {
                path.emplace_back(Current);
            }
        }
    }
   if (target.size() == 1)
        return (target);
    else if (target.size() > 1) {
        bool behindEnemyLine = false;    
        if (this->dX == LEFT) {
            behindEnemyLine = this->opp_left <= cell.x;
        }
        else {
            behindEnemyLine = this->opp_right >= cell.x;
        }
        //behindEnemyLine = true;
        path.clear();
        if (behindEnemyLine) {
            vector<t_cell> TargetSorted = target;
            sort(TargetSorted.begin(), TargetSorted.end(), sortTarget);
            for (vector<t_cell>::iterator find = TargetSorted.begin(); find != TargetSorted.end(); find++) {
                if (find->menace < cell.units) {
                    t_cell final = *find;
                    path.emplace_back(final);
                    break;
                }
            }
        }
        if (path.size() == 0) {
            t_cell final = *target.begin();
            path.emplace_back(final);
        }
    }
    return (path);
}

void    Brain::scanSafety( t_cell &cell ) {
    
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            bool axis = isStraight(j, i);
            if ((i == 0 and j == 0) or !axis)
                continue;
            int nY = cell.y + i;
            int nX = cell.x + j;
            if (!isValid(nX, nY)) {
                continue;
            }
            t_cell Current = cells[(nY * width) + nX];
            if (Current.owner == OPP and Current.units > 0) {
                if (cell.threat == -1)
                    cell.threat = 0;
                cell.threat += Current.units;
            }
        }
    }
}

int    Brain::scanDominion( t_cell &cell ) {
    
    int my_control = 1;
    int opp_control = 0;

    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            bool axis = isStraight(j, i);
            if ((i == 0 and j == 0) or !axis)
                continue;
            int nY = cell.y + i;
            int nX = cell.x + j;
            if (!isValid(nX, nY)) {
                continue;
            }
            t_cell Current = cells[(nY * width) + nX];
            if (Current.owner == OPP and Current.scrap_amount <= cell.scrap_amount) {
                opp_control++;
            }
            else if (Current.owner == ME) {
                my_control++;
            }
        }
    }

    if (opp_control > my_control)
        return (OPP);
    else if (my_control > opp_control)
        return (ME);
    else
        return (NEUTRAL);
}

bool    Brain::isDanger(t_cell &cell, t_cell &src, bool safety) {
    if (cell.threat == -1) {
        this->scanSafety(cell);
    }
    
    if (cell.in_range_of_recycler and cell.scrap_amount == 1) {
        return (true);
    }
    else if (safety == safetyON and cell.threat > src.units)
        return (true);
    else {
        return (false);
    }
}

bool    Brain::isBlockedorDanger( t_cell &cell, t_cell &src, bool safety ) {
    if (this->isBlocked(cell) or this->isDanger(cell, src, safety))
        return (true);
    else
        return (false);
}




bool    Brain::isTarget(t_cell cell, bool axis, int deltaX, int deltaY, int move, t_cell src) {
    bool verbose = src.x == -1 and src.y == 3;


    if (!this->oppBehind) {
        if (this->dX == LEFT) {
           if (cell.x < this->my_left and !src.in_range_of_recycler)
            return (false);
        }
        else
            if (cell.x >= this->my_right and !src.in_range_of_recycler)
                return (false);
    }

    if (!axis) {
        t_cell next[2];
        next[0] = cells[((cell.y + deltaY) * width) + cell.x];
        next[1] = cells[((cell.y) * width) + cell.x + deltaX];
        if (isBlockedorDanger(next[0], src, safetyON) and isBlockedorDanger(next[1], src, safetyON))
            return (false);
    }
    if (cell.in_range_of_recycler and cell.scrap_amount == 1)
        return (false);
    if (!this->oppBehind and this->isDeadEnd(cell, src))
        return (false);
    /*if (!crossed and cell.x == max[X] / 2 and isUnblocked(cell)) {
        if (cell.alreadyTargeted == true or expand) {
            return (false);
        }
        else {
            return (true);
        }
    }
    else if (goNorth and cell.y == highestOPP->y and cell.x == highestOPP->x) {
            return (true);
    }
    else if (goSouth and cell.y == lowestOPP->y and cell.x == lowestOPP->x) {
            return (true);
    }*/
    //else if (crossed and !goSouth and !goNorth and cell.owner != ME and cell.scrap_amount > 0 and !cell.recycler) {
    if (cell.owner != ME and cell.scrap_amount > 0 and !cell.recycler) {
        if (cell.in_range_of_recycler and cell.owner != OPP)
            return (false);
        else if (cell.alreadyTargeted == true) {
            return (false);
        }
        else {
            return (true);
        }
    }
    else {
        return (false);
    }
}

int Brain::myInRangeRecycler( t_cell &cell ) {
    int recyclers = 0;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            bool axis = isStraight(j, i);
            if ((i == 0 and j == 0) or !axis)
                continue;
            int nY = cell.y + i;
            int nX = cell.x + j;
            if (!this->isValid(nX, nY)) {
                continue;
            }
            t_cell Current = cells[(nY * width) + nX];
            if (Current.recycler)
                recyclers++;
        }
    }
    return (recyclers);
}

bool  Brain::hasAWay(t_cell cell, int owner) {
    bool    closedList[this->height][this->width];
    memset(closedList, false, sizeof(closedList));

    t_node  nodeDetails[this->height][this->width];

    int x;
    int y;

    for (y = 0; y < this->height; y++) {
        for (x = 0; x < this->width; x++) {
            nodeDetails[y][x].f = INT_MAX;
            nodeDetails[y][x].g = INT_MAX;
            nodeDetails[y][x].h = INT_MAX;
            nodeDetails[y][x].parent_x = -1;
            nodeDetails[y][x].parent_y = -1;
        }
    }

    x = cell.x;
    y = cell.y;

    nodeDetails[y][x].f = 0;
    nodeDetails[y][x].g = 0;
    nodeDetails[y][x].h = 0;
    nodeDetails[y][x].parent_x = x;
    nodeDetails[y][x].parent_y = y;

    set<pPair> openList;

    openList.insert(make_pair(0, make_pair(y, x)));

    bool foundTarget = false;
    pair<int,t_cell> result;
    int Priority = 0;
    int loop = 0;
    while (!openList.empty()) {
        pPair p = *openList.begin();
        openList.erase(openList.begin());

        y = p.second.first;
        x = p.second.second;
        closedList[y][x] = true;

        int gNew;
        int hNew;
        int fNew;

        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                bool axis = isStraight(j, i);
                if ((i == 0 and j == 0) or !axis)
                    continue;
                int nY = y + i;
                int nX = x + j;
                int movements = axis ? 1 : 2;
                if (!isValid(nX, nY)) {
                    continue;
                }
                t_cell *Current = &this->cells[(nY * width) + nX];
                t_cell *target = NULL;
                int newMovements = nodeDetails[y][x].g + movements;
                int newPriority = Current->targetPriority;
                bool    validCELL = Current->owner == owner and !Current->recycler; 
                if (validCELL) {
                    nodeDetails[nY][nX].parent_x = x;
                    nodeDetails[nY][nX].parent_y = y;
                    if (foundTarget == false) {
                        foundTarget = true;
                        result.first = newMovements;
                    }
                    else {
                        if (newMovements < result.first or (newMovements == result.first and newPriority > Priority)) {
                            result.first = newMovements;
                            int row = y;
                            int col = x;
                            while (!(nodeDetails[row][col].parent_x == cell.x and nodeDetails[row][col].parent_y == cell.y)) {
                                int tmp_row = nodeDetails[row][col].parent_y;
                                int tmp_col = nodeDetails[row][col].parent_x;
                                row = tmp_row;
                                col = tmp_col;
                            }
                            target = &cells[(row * width) + col];
                            result.second = *target;
                            Priority = newPriority;
                        }
                    }
                }
                else if (closedList[nY][nX] == false && !isBlocked(*Current) && !isDanger(*Current, cell, safetyON)) {
                    gNew = newMovements;
                    hNew = 0;
                    fNew = gNew + hNew;
                    if (nodeDetails[nY][nX].f == INT_MAX or nodeDetails[nY][nX].f >= fNew) {
                        Pair coord = make_pair(nY, nX);
                        pPair toInsert = make_pair(fNew, coord);
                        openList.insert(toInsert);
                        nodeDetails[nY][nX].f = fNew;
                        nodeDetails[nY][nX].g = gNew;
                        nodeDetails[nY][nX].h = hNew;
                        nodeDetails[nY][nX].parent_x = x;
                        nodeDetails[nY][nX].parent_y = y;
                    }
                }
            }
        }
        if (foundTarget == true) {
            return (true);
        }
        loop++;
    }
    return (false);
}


bool    Brain::isNextOPP(t_cell & cell) {

    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            bool axis = isStraight(j, i);
            if ((i == 0 and j == 0) or !axis)
                continue;
            int nY = cell.y + i;
            int nX = cell.x + j;
            if (!isValid(nX, nY)) {
                continue;
            }

            t_cell Current = cells[(nY * width) + nX];
            if (Current.scrap_amount > 0 and !Current.recycler and Current.owner == OPP)
                return (true);
        }
    }
    return (false);
}


Pair  Brain::searchAllies(t_cell cell) {
    bool    closedList[this->height][this->width];
    memset(closedList, false, sizeof(closedList));

    t_node  nodeDetails[this->height][this->width];

    int x;
    int y;

    for (y = 0; y < this->height; y++) {
        for (x = 0; x < this->width; x++) {
            nodeDetails[y][x].f = INT_MAX;
            nodeDetails[y][x].g = INT_MAX;
            nodeDetails[y][x].h = INT_MAX;
            nodeDetails[y][x].parent_x = -1;
            nodeDetails[y][x].parent_y = -1;
        }
    }

    x = cell.x;
    y = cell.y;

    nodeDetails[y][x].f = 0;
    nodeDetails[y][x].g = 0;
    nodeDetails[y][x].h = 0;
    nodeDetails[y][x].parent_x = x;
    nodeDetails[y][x].parent_y = y;

    set<pPair> openList;

    openList.insert(make_pair(0, make_pair(y, x)));

    bool foundTarget = false;
    pair<int,t_cell> result;
    int Priority = 0;
    int loop = 0;
    while (!openList.empty()) {
        pPair p = *openList.begin();
        openList.erase(openList.begin());

        y = p.second.first;
        x = p.second.second;
        closedList[y][x] = true;

        int gNew;
        int hNew;
        int fNew;

        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                bool axis = isStraight(j, i);
                if ((i == 0 and j == 0) or !axis)
                    continue;
                int nY = y + i;
                int nX = x + j;
                int movements = axis ? 1 : 2;
                if (!isValid(nX, nY)) {
                    continue;
                }
                t_cell *Current = &this->cells[(nY * width) + nX];
                t_cell *target = NULL;
                int newMovements = nodeDetails[y][x].g + movements;
                int newPriority = Current->targetPriority;
                bool    validCELL = Current->owner == ME and !Current->recycler and this->isNextOPP(*Current); 
                if (validCELL) {
                    nodeDetails[nY][nX].parent_x = x;
                    nodeDetails[nY][nX].parent_y = y;
                    if (foundTarget == false) {
                        foundTarget = true;
                        result.first = newMovements;
                        result.second = *Current;
                    }
                    else {
                        if (newMovements < result.first or (newMovements == result.first and newPriority > Priority)) {
                            result.first = newMovements;
                            result.second = *Current;
                            Priority = newPriority;
                        }
                    }
                }
                else if (closedList[nY][nX] == false && !isBlocked(*Current) && !isDanger(*Current, cell, safetyON)) {
                    gNew = newMovements;
                    hNew = 0;
                    fNew = gNew + hNew;
                    if (nodeDetails[nY][nX].f == INT_MAX or nodeDetails[nY][nX].f >= fNew) {
                        Pair coord = make_pair(nY, nX);
                        pPair toInsert = make_pair(fNew, coord);
                        openList.insert(toInsert);
                        nodeDetails[nY][nX].f = fNew;
                        nodeDetails[nY][nX].g = gNew;
                        nodeDetails[nY][nX].h = hNew;
                        nodeDetails[nY][nX].parent_x = x;
                        nodeDetails[nY][nX].parent_y = y;
                    }
                }
            }
        }
        if (foundTarget == true) {
            Pair resultCoord = make_pair(result.second.x, result.second.y);
            return (resultCoord);
        }
        loop++;
    }
    Pair noTarget = make_pair(-1, -1);
    return (noTarget);
}


Pair  Brain::AstarSearch(t_cell cell) {
    bool    closedList[this->height][this->width];
    memset(closedList, false, sizeof(closedList));

    t_node  nodeDetails[this->height][this->width];

    int x;
    int y;

    for (y = 0; y < this->height; y++) {
        for (x = 0; x < this->width; x++) {
            nodeDetails[y][x].f = INT_MAX;
            nodeDetails[y][x].g = INT_MAX;
            nodeDetails[y][x].h = INT_MAX;
            nodeDetails[y][x].parent_x = -1;
            nodeDetails[y][x].parent_y = -1;
        }
    }

    x = cell.x;
    y = cell.y;

    nodeDetails[y][x].f = 0;
    nodeDetails[y][x].g = 0;
    nodeDetails[y][x].h = 0;
    nodeDetails[y][x].parent_x = x;
    nodeDetails[y][x].parent_y = y;

    set<pPair> openList;

    openList.insert(make_pair(0, make_pair(y, x)));

    bool foundTarget = false;
    pair<int,t_cell> result;
    int Priority = 0;
    int loop = 0;
    while (!openList.empty()) {
        pPair p = *openList.begin();
        openList.erase(openList.begin());

        y = p.second.first;
        x = p.second.second;
        closedList[y][x] = true;

        int gNew;
        int hNew;
        int fNew;

        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                bool axis = isStraight(j, i);
                if ((i == 0 and j == 0) or !axis)
                    continue;
                int nY = y + i;
                int nX = x + j;
                int movements = axis ? 1 : 2;
                if (!isValid(nX, nY)) {
                    continue;
                }
                t_cell *Current = &this->cells[(nY * width) + nX];
                t_cell *target = NULL;
                int newMovements = nodeDetails[y][x].g + movements;
                int newPriority = Current->targetPriority;
                if (isTarget(*Current, axis, j, i, newMovements, cell)) {
                    nodeDetails[nY][nX].parent_x = x;
                    nodeDetails[nY][nX].parent_y = y;
                    if (foundTarget == false) {
                        foundTarget = true;
                        result.first = newMovements;

                        int row = y;
                        int col = x;
                        while (!(nodeDetails[row][col].parent_x == cell.x and nodeDetails[row][col].parent_y == cell.y)) {
                            int tmp_row = nodeDetails[row][col].parent_y;
                            int tmp_col = nodeDetails[row][col].parent_x;
                            row = tmp_row;
                            col = tmp_col;
                        }
                        target = &cells[(row * width) + col];
                        result.second = *target;
                        Priority = newPriority;
                    }
                    else {
                        if (newMovements < result.first or (newMovements == result.first and newPriority > Priority)) {
                            result.first = newMovements;
                            int row = y;
                            int col = x;
                            while (!(nodeDetails[row][col].parent_x == cell.x and nodeDetails[row][col].parent_y == cell.y)) {
                                int tmp_row = nodeDetails[row][col].parent_y;
                                int tmp_col = nodeDetails[row][col].parent_x;
                                row = tmp_row;
                                col = tmp_col;
                            }
                            target = &cells[(row * width) + col];
                            result.second = *target;
                            Priority = newPriority;
                        }
                    }
                }
                else if (closedList[nY][nX] == false && !isBlocked(*Current) && !isDanger(*Current, cell, safetyON)) {
                    gNew = newMovements;
                    /*if (!crossed and !goSouth and !goNorth)
                        hNew = abs(nX - (max[X] / 2));
                    else if (goSouth)
                        hNew = getHvalue(lowestOPP->x, lowestOPP->y, *Current);
                    else if (goNorth)
                        hNew = getHvalue(highestOPP->x, highestOPP->y, *Current);
                    else
                        hNew = 0;*/ //getHvalue(nX, nY, cell); //getHvalue(Cells, *Current, max); //0;
                    hNew = 0;
                    fNew = gNew + hNew;
                    if (nodeDetails[nY][nX].f == INT_MAX or nodeDetails[nY][nX].f >= fNew) {
                        Pair coord = make_pair(nY, nX);
                        pPair toInsert = make_pair(fNew, coord);
                        openList.insert(toInsert);
                        nodeDetails[nY][nX].f = fNew;
                        nodeDetails[nY][nX].g = gNew;
                        nodeDetails[nY][nX].h = hNew;
                        nodeDetails[nY][nX].parent_x = x;
                        nodeDetails[nY][nX].parent_y = y;
                    }
                }
            }
        }
        if (foundTarget == true) {
            Pair resultCoord = make_pair(result.second.x, result.second.y);
            return (resultCoord);
        }
        loop++;
    }
    Pair noTarget = make_pair(-1, -1);
    return (noTarget);
}

int findRessources(vector<t_cell> Cells, t_cell cell, int deltaX, int deltaY, int minR) {
    for (vector<t_cell>::iterator it = Cells.begin(); it != Cells.end(); it ++) {
        t_cell Current = *it;
        bool my_units = Current.owner == ME and Current.units > 0;
        if (Current.x == cell.x + deltaX and Current.y == cell.y + deltaY) {
            if (Current.scrap_amount >= minR and !my_units)
                return (1);
            else if (Current.scrap_amount < 2 or my_units)
                return (-1);
            else
                return (0);
        }
    }
    return (-1);
}

int findObstacles(vector<t_cell> Cells, t_cell cell, int deltaX, int deltaY) {
    for (vector<t_cell>::iterator it = Cells.begin(); it != Cells.end(); it ++) {
        t_cell Current = *it;
        if (Current.x == cell.x + deltaX and Current.y == cell.y + deltaY) {
            if (Current.scrap_amount == 0 or Current.recycler)
                return (1);
            else
                return (0);
        }
    }
    return (1);
}

int findPriority(vector<t_cell> Cells, t_cell cell, int deltaX, int deltaY) {
    int Priority = 0;
    for (vector<t_cell>::iterator it = Cells.begin(); it != Cells.end(); it ++) {
        t_cell Current = *it;
        if (Current.x == cell.x + deltaX and Current.y == cell.y + deltaY) {
            if (Current.scrap_amount > 0 and Current.owner != ME)
                Priority++;
            if (Current.owner == OPP) {
                Priority++;
                Priority += Current.units;
            }
        }
    }
    return (Priority);
}

int Brain::checkCellSpawnPriority( t_cell cell ) {
    int Priority = 0;
    bool    verbose = false;
    
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            bool axis = isStraight(j, i);
            if ((i == 0 and j == 0) or !axis)
                continue;
            int nY = cell.y + i;
            int nX = cell.x + j;
            if (!isValid(nX, nY)) {
                //Priority--;
                continue;
            }
            t_cell Current = cells[(nY * width) + nX];
            Priority += Current.spawnValue;
        }
    }

    return (Priority);
}

vector<t_cell>  Brain::checkPriority( void ) {
    vector<t_cell> toSpawn;

    toSpawn.clear();

    for (vector<t_cell>::iterator it = my_cells.begin(); it != my_cells.end(); it++) {
        t_cell Current = *it;
        bool    verbose = Current.x == -1 and Current.y == 5;
        
        if (Current.recycler) {
            continue;
        }
        
        t_cell *checkPrevious = this->getCell(Current.x, Current.y);

        if (checkPrevious->unitsSpawned != 0)
            continue;

        int currentPriority;

        currentPriority = checkCellSpawnPriority(Current);
        int inRange = myInRangeRecycler(Current);
        bool Way = this->hasAWay(Current, ME);
        if (inRange) {
            if (!Way and Current.scrap_amount > inRange)
                currentPriority = 100;
            else
                currentPriority = -4;
        }

        if (currentPriority > 0) {
            Current.spawnPriority = currentPriority;
            toSpawn.emplace_back(Current);
        }
    }
   
   if (toSpawn.size() > 0) {
        if (this->dX == LEFT)
            sort(toSpawn.begin(), toSpawn.end(), sortRIGHT);
        else
            sort(toSpawn.begin(), toSpawn.end(), sortLEFT);

        sort(toSpawn.begin(), toSpawn.end(), sortPriority);
   }

    return (toSpawn);
}

int Brain::checkEnemySquad( t_cell cell, bool reinforce ) {
    int     Squad = 0;

    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            bool axis = isStraight(j, i);
            if ((i == 0 and j == 0) or !axis)
                continue;
            int nY = cell.y + i;
            int nX = cell.x + j;
            if (!isValid(nX, nY)) {
                continue;
            }
            t_cell Current = cells[(nY * width) + nX];
            if (Current.owner != OPP)
                continue;
            Squad += Current.units;
            if (reinforce and Current.units == 1)
                Squad += checkEnemySquad(Current, false);
        }
    }
    return (Squad);
}

vector<t_cell> Brain::targetEnemy( t_cell & cell ) {
    vector<t_cell>  eSquads;

    eSquads.clear();

    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            bool axis = isStraight(j, i);
            if ((i == 0 and j == 0) or !axis)
                continue;
            int nY = cell.y + i;
            int nX = cell.x + j;
            if (!isValid(nX, nY)) {
                continue;
            }
            t_cell Current = cells[(nY * width) + nX];
            if (Current.owner != OPP)
                continue;
            eSquads.emplace_back(Current);
        }
    }

    if (eSquads.size() > 1)
        sort(eSquads.begin(), eSquads.end(), sortUnits);

    return (eSquads);
}

/*bool checkRessources(vector<t_cell> cells, t_cell cell, int turn) {
    int ressources = 0;
    static int minR = 5;
    static int minM = 3;

    ressources += findRessources(cells, cell, 1, 0, minR);
    ressources += findRessources(cells, cell, -1, 0, minR);
    ressources += findRessources(cells, cell, 0, 1, minR);
    ressources += findRessources(cells, cell, 0, -1, minR);
    if ( ressources >= minM) {
        return true;
    }
    else {

        return false;
    }
}*/

int Brain::checkRessources( t_cell & cell ) {
    int ressources = 0;

    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            bool axis = isStraight(j, i);
            if (!axis)
                continue;
            int nY = cell.y + i;
            int nX = cell.x + j;
            if (!isValid(nX, nY)) {
                continue;
            }
            t_cell Current = cells[(nY * width) + nX];
            if (Current.scrap_amount > cell.scrap_amount)
                ressources += cell.scrap_amount;
            else
                ressources += Current.scrap_amount;
            if (Current.owner == OPP and Current.recycler)
                ressources += 20;
            if (Current.owner == ME)
                ressources -= Current.units;
        }
    }
    return (ressources);
}

/*bool    validSite(vector<t_cell> cells, t_cell cell, int height, int width) {
    int     stop = 0;
    int     ressources = 0;
    bool    ret = false;

    if (cell.x == width / 3) {
        ret = true;
    }
    else if (cell.x == (width /3 ) * 2) {
        ret = true;
    }
    else if (height > 7 and cell.y == height / 2 ) {
        ret = true;
    }
    else {
        ret = false;
    }
    if (ret) {
        stop += findObstacles(cells, cell, 1, 0);
        stop += findObstacles(cells, cell, -1, 0);
        stop += findObstacles(cells, cell, 0, 1);
        stop += findObstacles(cells, cell, 0, -1);
        ressources += findRessources(cells, cell, 1, 0);
        ressources += findRessources(cells, cell, -1, 0);
        ressources += findRessources(cells, cell, 0, 1);
        ressources += findRessources(cells, cell, 0, -1);
    }
    if (stop > 2 or ressources < 3) {
        ret = false;
    }
    return (ret);
}*/

t_cell  findSpawnSite(vector<t_cell> target_cells, int height, int width) {
    t_cell target = *target_cells.begin();

    for (vector<t_cell>::iterator it = target_cells.begin(); it != target_cells.end(); it++) {
        t_cell Current = *it;
        if (Current.units > 0) {
            return (Current);
        }
    }

    for (vector<t_cell>::iterator it = target_cells.begin(); it != target_cells.end() ; it++) {
        t_cell Current = *it;
        bool    danger = (Current.in_range_of_recycler and Current.scrap_amount == 1);
        if (!Current.recycler and !danger) {
            int tmpDist = abs(Current.x - (width / 2)) + abs(Current.y - (height / 2));
            int dist = abs(target.x - (width / 2)) + abs(target.y - (height / 2));
            if (tmpDist <= dist)
                target = Current;
            if (tmpDist == 1)
                return (target);
        }
    }
    return (target);
}

int main()
{
    int width;
    int height;
    cin >> width >> height; cin.ignore();

    Brain   hive(width, height);

    int MaxBuilding = 0;
    int bigGrid = (width * height) >= (9 * 18);
    int check = 0;
    // game loop
    while (1) {
        int             data_my_units = 0;
        int             data_opp_units = 0;
        int             data_my_recyclers = 0;
        int             data_opp_recyclers = 0;
        bool            has_build = false;
        

        int my_matter;
        int opp_matter;
        cin >> my_matter >> opp_matter; cin.ignore();
        
        hive.opp_matter = opp_matter;

        if (hive.turn and hive.turn < 13) {
            MaxBuilding = (hive.turn / 3) + 1;
        }
        if (!bigGrid) {
            if (hive.turn > 15 and hive.turn < 20) {
                MaxBuilding = 19 - hive.turn;
            }
        }
        
        hive.cells.reserve(height * width);
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int scrap_amount;
                int owner; // 1 = me, 0 = foe, -1 = neutral
                int units;
                int recycler;
                int can_build;
                int can_spawn;
                int in_range_of_recycler;
                cin >> scrap_amount >> owner >> units >> recycler >> can_build >> can_spawn >> in_range_of_recycler; cin.ignore();

                t_cell cell = {x, y, scrap_amount, owner, units, 0, recycler == 1, can_build == 1, can_spawn == 1, in_range_of_recycler == 1, false, 0, 0, 0, false, 0, -1, UNSET, make_pair(-1, -1), true, 0};
                if (cell.owner == ME) {
                    if (!hive.north and cell.y == 0)
                        hive.north = true;
                    if (!hive.south and cell.y == height - 1)
                        hive.south = true;
                    if (cell.units > 0) {
                        hive.my_units.emplace_back(cell);

                        if (cell.x < hive.my_left)
                            hive.my_left = cell.x;
                        if (cell.x > hive.my_right)
                            hive.my_right = cell.x;
                        if (cell.y < height / 2)
                            hive.units_up += cell.units;
                        else
                            hive.units_down += cell.units;
                    }
                    else if (cell.recycler) {
                        cell.spawnValue = 0;
                        hive.my_recyclers.emplace_back(cell);
                    }
                    else if (hive.dX == 0 and hive.dY == 0) {
                        if (x < width / 2)
                            hive.dX = -1; // LEFT
                        else
                            hive.dX = 1; // RIGHT
                        if (y < height / 2)
                            hive.dY = -1;
                        else
                            hive.dY = 1;
                    }
                    hive.my_cells.emplace_back(cell);
                }
                else if (cell.owner == OPP) {
                    cell.spawnValue = 5;
                    cell.targetPriority = 5;
                    if (cell.units > 0) { 
                        cell.spawnValue += cell.units;
                        hive.opp_units.emplace_back(cell);
                        if (cell.x < hive.opp_left)
                            hive.opp_left = cell.x;
                        if (cell.x > hive.opp_right)
                            hive.opp_right = cell.x;
                    }
                    else if (cell.recycler) {
                        cell.spawnValue = 0;
                        hive.opp_recyclers.emplace_back(cell);
                    }
                    if (!cell.recycler) {
                        hive.target_cells.emplace_back(cell);
                    }
                    hive.opp_cells.emplace_back(cell);
                }
                else {
                    cell.spawnValue = 1;
                    cell.targetPriority = 1;
                    if (cell.scrap_amount > 0)
                        hive.target_cells.emplace_back(cell);
                    else
                        cell.spawnValue = 0;
                    hive.neutral_cells.emplace_back(cell);
                }
                hive.cells.emplace_back(cell);
            }
        }

        hive.my_highest = *hive.my_units.begin();
        hive.my_lowest = *hive.my_units.rbegin();
        hive.opp_highest = *hive.opp_units.begin();
        hive.opp_lowest = *hive.opp_units.rbegin();

        if (!hive.oppBehind)  {
            if (hive.dX == LEFT) {
                if (hive.opp_left <= hive.my_left ) {
                    hive.oppBehind = true;
                }
            }
            else {
                if (hive.opp_right >= hive.my_right) {
                    hive.oppBehind = true;
                }
            }
            bool WayFound = false;
            for (vector<t_cell>::iterator it = hive.my_units.begin(); it != hive.my_units.end(); it++) {
                if (hive.hasAWay(*it, OPP)) {
                    WayFound = true;
                    break;
                }
            }
            if (!WayFound) {
                hive.oppBehind = true;
                hive.noWay = true;
            }
        }
        if (hive.oppBehind and !hive.noWay) {
            bool WayFound = false;
            for (vector<t_cell>::iterator it = hive.my_units.begin(); it != hive.my_units.end(); it++) {
                if (hive.hasAWay(*it, OPP)) {
                    WayFound = true;
                    break;
                }
            }
            if (!WayFound) {
                hive.noWay = true;
            }
        }


        
        vector<string> actions;
        vector<t_cell> reinforce;
        for (vector<t_cell>::iterator it = hive.my_cells.begin(); it != hive.my_cells.end(); it++) {
            t_cell cell = *it;
            int eSquad = hive.checkEnemySquad(cell, true);
            if (eSquad > 0 and !cell.recycler) {
                cell.reinforce = eSquad;
                reinforce.emplace_back(cell);
            }
        }     
        vector<t_cell> secondcavalry;
        secondcavalry.clear();

        //sort(reinforce.begin(), reinforce.end(), sortReinforce);

        if (hive.dX == LEFT)
            sort(reinforce.begin(), reinforce.end(), sortLEFT);
        else
            sort(reinforce.begin(), reinforce.end(), sortRIGHT);
        
        for (vector<t_cell>::iterator it = reinforce.begin(); it != reinforce.end(); it++) {
   
            t_cell cell = *it;
            bool build = cell.reinforce > 1 or hive.my_recyclers.empty();
            bool    verbose = cell.x == 7 and cell.y == 5;
            bool    reinforced = false;
            int     closeEnemy = hive.checkEnemySquad(cell, false);
            bool    willDisappear = cell.scrap_amount <= hive.myInRangeRecycler(cell);
            
            if (willDisappear) {
                continue;
            }

            if (reinforce.size() == 1 and (((cell.units + (my_matter / 10)) > closeEnemy) and cell.can_spawn and !cell.recycler)) {
                int closeEnemy = hive.checkEnemySquad(cell, false);
                int menNeeded = my_matter / 10;
                if ( (my_matter >= menNeeded * 10) and cell.can_spawn and !cell.recycler) {
                    ostringstream action;
                    action << "SPAWN " << menNeeded  << " " << cell.x << " " << cell.y;
                    actions.emplace_back(action.str());
                    my_matter -= (menNeeded * 10);
                    hive.cells[(cell.y * width) + cell.x].unitsSpawned += menNeeded;
                    reinforced = true;
                }
            }
            else if (cell.reinforce == 1 and cell.units < 1 and my_matter >= 20 and cell.can_spawn and !cell.recycler) {
                ostringstream action;
                action << "SPAWN " << 2 << " " << cell.x << " " << cell.y;
                actions.emplace_back(action.str());
                my_matter -= 20;
                hive.cells[(cell.y * width) + cell.x].unitsSpawned += 2;
                reinforced = true;
            }
            else if (build and my_matter >= 10 and cell.can_build) {
                ostringstream action;
                action << "BUILD " << cell.x << " " << cell.y;
                hive.updateRecyclerStatus(cell, true);
                actions.emplace_back(action.str());
                my_matter -= 10;
                reinforced = true;
            }
            
            if (!reinforced)
                secondcavalry.emplace_back(cell);
        }

        if ( secondcavalry.size() > 0) {
            for (vector<t_cell>::iterator it = secondcavalry.begin(); it != secondcavalry.end(); it++) {
                t_cell cell = *it;
                int     closeEnemy = hive.checkEnemySquad(cell, false);
                int menNeeded = 1 + closeEnemy - cell.units;
                if (menNeeded < 1)
                    continue;
                if ( (my_matter >= menNeeded * 10) and cell.can_spawn and !cell.recycler) {
                    ostringstream action;
                    action << "SPAWN " << menNeeded  << " " << cell.x << " " << cell.y;
                    actions.emplace_back(action.str());
                    my_matter -= (menNeeded * 10);
                    hive.cells[(cell.y * width) + cell.x].unitsSpawned += menNeeded;
                }
            }
        }

        reinforce.clear();

        if (my_matter >= 10 and hive.my_recyclers.size() < hive.opp_recyclers.size() and hive.my_recyclers.size() < 2 and !hive.noWay) {
            for (vector<t_cell>::iterator it = hive.my_cells.begin(); it != hive.my_cells.end(); it++) {
                t_cell cell = *it;
                if (!cell.can_build)
                    continue;
                int Ressources = hive.checkRessources(cell);
                if (Ressources > 0 ) {
                    cell.reinforce = Ressources;
                    reinforce.emplace_back(cell);
                }
            }
        }

        /*if (hive.dX == LEFT)
            sort(reinforce.begin(), reinforce.end(), sortRIGHT);
        else
            sort(reinforce.begin(), reinforce.end(), sortLEFT);*/

        sort(reinforce.begin(), reinforce.end(), sortReinforce);
        
        for (vector<t_cell>::iterator it = reinforce.begin(); it != reinforce.end(); it++) {
   
            t_cell  cell = *it;
            bool    build = my_matter >= 10 and cell.can_build and hive.my_recyclers.size() <= hive.opp_recyclers.size() and hive.my_recyclers.size() < 3;
            bool    verbose = false;

            if (build and my_matter >= 10) {
                ostringstream action;
                action << "BUILD " << cell.x << " " << cell.y;
                hive.updateRecyclerStatus(cell, true);
                hive.my_recyclers.emplace_back(*it);
                actions.emplace_back(action.str());
                my_matter -= 10;
                break;
            }
        }
        
        reinforce.clear();

        /*if (my_matter >= 10 and hive.my_recyclers.size() < hive.opp_recyclers.size() and hive.my_recyclers.size() > 3) {
    
            for (vector<t_cell>::iterator it = hive.my_cells.begin(); it != hive.my_cells.end() and !has_build; it++) {
                t_cell Current = *it;
                if (Current.can_build) {
                    bool validSite = checkRessources(hive.cells, Current, hive.turn);
                    if (validSite) {
                        ostringstream action;
                        action << "BUILD " << Current;
                        hive.updateRecyclerStatus(Current, true);
                        actions.emplace_back(action.str());
                        my_matter -= 10;
                        break;
                    }
                }
            }
        }*/

        if (my_matter >= 10) {
            vector<t_cell> toSpawn = hive.checkPriority();
            int unitSpawned = 0;
            if (toSpawn.size() > 0) {    
                for (vector<t_cell>::iterator target = toSpawn.begin(); target != toSpawn.end() and my_matter >= 10; target++) {
                    
                    if (target == (toSpawn.end() - 1)) {
                        unitSpawned = my_matter / 10;
                    }
                    else {
                        int threat = hive.checkEnemySquad(*target, false);
                        if ( threat and threat > (target->units + target->unitsSpawned)) {
                            if ((my_matter / 10) + target->units > threat + 1 )
                                unitSpawned = (threat + 1) - target->units;
                            else {
                            unitSpawned = 0;
                            }
                        }
                        else if (threat) {
                            unitSpawned = 0;
                        }
                        else {
                            unitSpawned = 1; 
                        }
                    }

                    if (unitSpawned > 0) {
                    ostringstream action;
                    action << "SPAWN " << unitSpawned << " " << *target;
                    actions.emplace_back(action.str());
                    my_matter -= unitSpawned * 10;
                    hive.cells[(target->y * width) + target->x].unitsSpawned += unitSpawned;
                    } 
                }
            }
        }

        for (vector<t_cell>::iterator it = hive.my_units.begin(); it != hive.my_units.end(); it++) {   

            for (vector<t_cell>::iterator isTargeted = hive.cells.begin(); isTargeted != hive.cells.end(); isTargeted++) {
                isTargeted->alreadyTargeted = false;
            }

        
            t_cell cell = *it;
            bool verbose = cell.x == 11 and cell.y == 4;

            int stand = hive.cells[(cell.y * hive.width) + cell.x].stand;
            int cX = hive.cells[(cell.y * hive.width) + cell.x].cavalry.first;
            int cY = hive.cells[(cell.y * hive.width) + cell.x].cavalry.second;
            cell.units = hive.cells[(cell.y * hive.width) + cell.x].units;
            cell.unitsSpawned = hive.cells[(cell.y * hive.width) + cell.x].unitsSpawned;
            int defense = cell.units + cell.unitsSpawned;

            if (stand == TRUE) {
                continue; 
            }

            if (cX != -1 and cY != -1) {
                ostringstream action;
                action << "MOVE " << cell.units << " " << cell << " " << cX << " " << cY;
                actions.emplace_back(action.str());
                hive.cells[(cell.y * hive.width) + cell.x].units = 0;
                continue;
            }

            int threat = hive.checkEnemySquad(cell, false);

            if (threat > 0 and threat > defense and cell.stand == UNSET) {
                if (hive.CavalryCall(cell))
                    continue;
            }
            vector<t_cell> eSquads;

            eSquads.clear();
            bool    squadSaved = false;
            if (threat == defense and cell.stand == UNSET) {
                eSquads = hive.targetEnemy(cell);
                squadSaved = true;
                bool hasEnemyTarget = false;
                for (vector<t_cell>::iterator squads = eSquads.begin(); squads != eSquads.end(); squads++) {
                    if (hive.Battlecry(*squads)) {
                        cX = hive.cells[(cell.y * hive.width) + cell.x].cavalry.first;
                        cY = hive.cells[(cell.y * hive.width) + cell.x].cavalry.second;
                        hasEnemyTarget = true;
                    }
                }
                if (!hasEnemyTarget) {
                    bool backToDoor = false;
                    if (hive.dX == LEFT) {
                        t_cell *check = hive.getCell(cell.x - 1, cell.y);
                        backToDoor = hive.isBlocked(*check) or check->owner != ME;
                    }  
                    else {
                        t_cell *check = hive.getCell(cell.x + 1, cell.y);
                        backToDoor = hive.isBlocked(*check) or check->owner != ME;
                    } 
                    
                    if (!backToDoor) {
                        t_cell *update = hive.getCell(cell.x, cell.y);
                        update->unitsAvailable = false;
                        update->stand = TRUE;
                        continue;
                    }
                }
            }

            if (hive.isNeeded(cell)) {
                cX = hive.cells[(cell.y * hive.width) + cell.x].cavalry.first;
                cY = hive.cells[(cell.y * hive.width) + cell.x].cavalry.second;
            }
            else {
                if (!squadSaved)
                    eSquads = hive.targetEnemy(cell);
                for (vector<t_cell>::iterator squads = eSquads.begin(); squads != eSquads.end(); squads++) {
                    if (hive.Battlecry(*squads)) {
                        cX = hive.cells[(cell.y * hive.width) + cell.x].cavalry.first;
                        cY = hive.cells[(cell.y * hive.width) + cell.x].cavalry.second;
                    }
                }
            }

            if (cX != -1 and cY != -1) {
                ostringstream action;
                action << "MOVE " << cell.units << " " << cell << " " << cX << " " << cY;
                actions.emplace_back(action.str());
                hive.cells[(cell.y * hive.width) + cell.x].units = 0;
                continue;
            }

            cell.units = hive.cells[(cell.y * hive.width) + cell.x].units;

            if (cell.x == width / 2)
                hive.crossed = true;
            if (cell.y == 0)
                hive.north = true;
            else if (cell.y == height - 1)
                hive.south = true;
            bool loop = true;
            int units = cell.units;

            

            while (loop) {
                int atTurn = units;
                t_cell  targetCell;
                bool    AStarFound = true;
                vector<t_cell>target = hive.sonar(cell, safetyON);
                if (target.size() == 0) {
                    Pair targetCoord = hive.AstarSearch(cell);
                    int tX = targetCoord.first;
                    int tY = targetCoord.second;
                    if (tX == -1 and tY == -1) {
                        loop = false;
                        AStarFound = false;
                    }
                    if (AStarFound) {
                        targetCell = hive.cells[(tY * hive.width) + tX];
                }
                else {
                    targetCell = *target.begin();
                }

                if (target.size() == 0 and !AStarFound) {
                    Pair allies = hive.searchAllies(cell);
                    if (allies.first == -1 and allies.second == -1)
                        continue;
                    ostringstream action;
                    action << "MOVE " << cell.units << " " << cell << " " << targetCell;
                    actions.emplace_back(action.str());
                }
                
                hive.updateTargetStatus(targetCell, true);
                bool danger = cell.in_range_of_recycler and cell.scrap_amount == 1;
                int squad = 0;
                bool useThreat = false;
                int incomming = hive.checkEnemySquad(targetCell, false);
                
                if (units > 1 and !danger and hive.turn < 50 and !threat and incomming == 0) {
                    squad = units / 2;
                }
                else if (units > 1 and !danger and !threat and incomming == 0) {
                    squad = (rand() % units) + 1;
                }
                else if (useThreat and threat <= units and !cell.in_range_of_recycler) {
                    squad = units - threat;
                    loop = false;
                }
                else
                    squad = units;
                if (targetCell.x == cell.x and targetCell.y == cell.y) {
                    squad = 0;
                    loop = false;
                }
                if (squad > 0) {
                    ostringstream action;
                    action << "MOVE " << squad << " " << cell << " " << targetCell;
                    actions.emplace_back(action.str());
                }
                units -= squad;
                if (units == 0)
                    loop = false;
            }
        }
        
         if (actions.empty()) {
             cout << "WAIT" << endl;
        } else {
            for (vector<string>::iterator it = actions.begin(); it != actions.end(); ++it) {
                cout << *it << ";";
            }
            cout << endl;
        }
        hive.endTurn();
        hive.turn++;
    }
}