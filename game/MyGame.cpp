#include "stdafx.h"
#include "MyGame.h"

#pragma warning (disable: 4244)

// NOTES FOR FUTURE ME
// Work on the path finding stuff next time, Just completed stub code for PathFind
// Work on Daijkstra's
// Make the nodes only check enabled ones
// Make walls moveable
// Make moved walls enable and disable appropriate nodes


char* CMyGame::m_tileLayout[12] =
{
	// each tile is 64x64 pixels
	"XXXXXXXXXXXXXXXXXXXX",
	"X                  X",
	"X      XX          X",
	"X                  X",
	"X                  X",
	"X                  X",
	"X                  X",
	"X                  X",
	"X           X      X",
	"X           X      X",
	"X                  X",
	"XXXXXXXXXXXXXXXXXXXX",
};

CMyGame::CMyGame(void) :
	m_npc(544, 96, 64, 64, 0),
	m_pumpkin(-100, -100, "Pumpkin22.png", CColor::White(), 0)
{
	m_npc.LoadAnimation("Spider64.png", "walk", CSprite::Sheet(4, 2).Col(0).From(0).To(1));
	m_npc.LoadAnimation("Spider64.png", "idle", CSprite::Sheet(4, 2).Col(2).From(0).To(1));
	m_npc.SetAnimation("idle", 4);

	// create graph structure - nodes 
	// using a loop, to create a node in the middle of every tile
	int tileSize = 64;	// each tile on the map is 64x64 pixels
	float posX;	// x coord of the middle of the tile
	float posY;	// y coord of the middle of the tile
	bool enabled;

	for (int y = 0; y < 12; y++)	// loop through every tile, the tile map is 12 tiles tall
	{
		for (int x = 0; x < 20; x++)	// loop through every tile, the tile map is 20 tiles wide
		{
			posX = x * tileSize + tileSize / 2;	// x * tile size finds the left of that tile, adding half the tile size gives the middle
			posY = y * tileSize + tileSize / 2;	// y * tile size finds the bottom of that tile, adding half the tile size gives the middle

			if (m_tileLayout[y][x] == 'X')	// if there is a wall on that tile, set enabled to false, so the node won't be used for pathfinding
			{
				enabled = false;
			}
			else
			{
				enabled = true;	// if there is no wall there set the node to enabled so that it is used during pathfinding
			}

			// adding node to graph
			m_graph.push_back(NODE{CVector(posX, posY), {}, enabled});
		}
	}

	// create graph structure - connections 
	// using a loop to automatically generate connections
	int index;	// the index for the node we're making connections for
	int r;	// index for the node to the right of our current node we're looking at
	int a;	// index for the node above our current node we're looking at
	float dist;	// distance between nodes, used for the 

	for (int y = 0; y < 12; y++)	// loop through every tile, the tile map is 12 tiles tall
	{
		for (int x = 0; x < 20; x++)	// loop through every tile, the tile map is 20 tiles wide
		{
			// Since the example game we did in class creates a two way connection so the NPC can travel both ways across connections
			// I only need to create connections for 2 sides of each node, since if I did all sides, I would create multiple of the same connection
			// In this case I choose to do connections for the right and above each node
			// find node index
			index = y * 20 + x;	// because my tiles go from left to right, starting in the bottom left, each row going up is 20 more than the row below it

			// right connection
			if (x < 19)	// only connect right if x < 19, because there are 20 tiles horizontally, which would be from 0-19 and there is no node to the right of the very right tiles
			{
				r = index + 1;	// index for the node right of this node
				dist = Distance(m_graph[index].pos, m_graph[r].pos);	// can find the position of each node with their index in the graph we made and the pos variable each node has thanks to the NODE structure

				// creating connections both ways
				m_graph[index].conlist.push_back(CONNECTION{r, dist});	// create connection from the node we're looking at to the node to it's right
				m_graph[r].conlist.push_back(CONNECTION{index, dist});	// node in the opposite direction of the line above (node to the right of this one, connecting to this one)
			}

			// above connection
			if (y < 11)	// only connect right if y < 11, because there are 12 tiles vertically, which would be from 0-11 and there is no node above of the very top tiles
			{
				a = index + 20;	// index for the node above of this node
				dist = Distance(m_graph[index].pos, m_graph[a].pos);	// can find the position of each node with their index in the graph we made and the pos variable each node has thanks to the NODE structure

				// creating connections both ways
				m_graph[index].conlist.push_back(CONNECTION{ a, dist });	// create connection from the node we're looking at to the node above it
				m_graph[a].conlist.push_back(CONNECTION{ index, dist });	// node in the opposite direction of the line above (node to the right of this one, connecting to this one)
			}
		}
	}
}

CMyGame::~CMyGame(void)
{
}

/////////////////////////////////////////////////////
// Per-Frame Callback Funtions (must be implemented!)

void CMyGame::OnUpdate()
{
	Uint32 t = GetTime();

	// NPC: follow the waypoints
	if (!m_waypoints.empty())
	{
		// If NPC not moving, start moving to the first waypoint
		if (m_npc.GetSpeed() < 1)
		{
			m_npc.SetSpeed(500);
			m_npc.SetAnimation("walk");
			m_npc.SetDirection(m_waypoints.front() - m_npc.GetPosition());
			m_npc.SetRotation(m_npc.GetDirection() - 90);
		}

		// Passed the waypoint?
		CVector v = m_waypoints.front() - m_npc.GetPosition();
		if (Dot(m_npc.GetVelocity(), v) < 0)
		{
			// Stop movement
			m_waypoints.pop_front();
			if (m_waypoints.empty())
				m_npc.SetAnimation("idle");
			m_npc.SetVelocity(0, 0);
			m_npc.SetRotation(0);
		}
	}
	
	m_npc.Update(t);
}

void CMyGame::OnDraw(CGraphics* g)
{
	for (NODE n : m_graph)
		for (CONNECTION c : n.conlist)
			g->DrawLine(n.pos, m_graph[c.nEnd].pos, CColor::Black());
	m_nodes.for_each(&CSprite::Draw, g);
	m_tiles.for_each(&CSprite::Draw, g);
	m_pumpkin.Draw(g);
	m_npc.Draw(g);
}

/////////////////////////////////////////////////////
// Game Life Cycle

// one time initialisation
void CMyGame::OnInitialize()
{
	// Create Nodes

	int i = 0;
	for (NODE n : m_graph)
	{
		stringstream s;
		s << i++;
		m_nodes.push_back(new CSpriteOval(n.pos, 12, CColor::White(), CColor::Black(), 0));
		m_nodes.push_back(new CSpriteText(n.pos, "arial.ttf", 14, s.str(), CColor::Black(), 0));
	}

	// Create Tiles
	for (int y = 0; y < 12; y++)
		for (int x = 0; x < 20; x++)
		{
			if (m_tileLayout[y][x] == ' ')
				continue;

			int nTile = 5;
			if (y > 0 && m_tileLayout[y - 1][x] == ' ') nTile -= 3;
			if (y < 11 && m_tileLayout[y + 1][x] == ' ') nTile += 3;
			if (x > 0 && m_tileLayout[y][x - 1] == ' ') nTile--;
			if (x < 20 && m_tileLayout[y][x + 1] == ' ') nTile++;
			if (nTile == 5 && x > 0 && y > 0 && m_tileLayout[y - 1][x - 1] == ' ') nTile = 14;
			if (nTile == 5 && x < 20 && y > 0 && m_tileLayout[y - 1][x + 1] == ' ') nTile = 13;
			if (nTile == 5 && x > 0 && y < 11 && m_tileLayout[y + 1][x - 1] == ' ') nTile = 11;
			if (nTile == 5 && x < 20 && y < 11 && m_tileLayout[y + 1][x + 1] == ' ') nTile = 10;
			
			nTile--;
			m_tiles.push_back(new CSprite(x * 64.f + 32.f, y * 64.f + 32.f, new CGraphics("tiles.png", 3, 5, nTile % 3, nTile / 3), 0));
		}
}

// pathfinding code, INCOMPLETE, CONTINUE THIS NEXT TIME
bool PathFind(vector<NODE>& graph, int nStart, int nGoal, vector<int>& path)
{
	path.push_back(nStart);
	path.push_back(nGoal);
	return true;
}


// called when a new game is requested (e.g. when F2 pressed)
// use this function to prepare a menu or a welcome screen
void CMyGame::OnDisplayMenu()
{
	StartGame();	// exits the menu mode and starts the game mode
}

// called when a new game is started
// as a second phase after a menu or a welcome screen
void CMyGame::OnStartGame()
{
}

// called when a new level started - first call for nLevel = 1
void CMyGame::OnStartLevel(Sint16 nLevel)
{
}

// called when the game is over
void CMyGame::OnGameOver()
{
}

// one time termination code
void CMyGame::OnTerminate()
{
}

/////////////////////////////////////////////////////
// Keyboard Event Handlers

void CMyGame::OnKeyDown(SDLKey sym, SDLMod mod, Uint16 unicode)
{
	if (sym == SDLK_F4 && (mod & (KMOD_LALT | KMOD_RALT)))
		StopGame();
	if (sym == SDLK_SPACE)
		PauseGame();
	if (sym == SDLK_F2)
		NewGame();
}

void CMyGame::OnKeyUp(SDLKey sym, SDLMod mod, Uint16 unicode)
{
}


/////////////////////////////////////////////////////
// Mouse Events Handlers

void CMyGame::OnMouseMove(Uint16 x,Uint16 y,Sint16 relx,Sint16 rely,bool bLeft,bool bRight,bool bMiddle)
{
}

void CMyGame::OnLButtonDown(Uint16 x, Uint16 y)
{
	CVector v(x, y);	// destination

	// check if the move is legal
	if (m_tileLayout[y / 64][x / 64] != ' ')
		return;	// cannot go into wall

	// Place the Pumpkin at the clicked position
	m_pumpkin.SetPosition(v);

	// find the first node: the closest to the NPC
	vector<NODE>::iterator iFirst = min_element(m_graph.begin(), m_graph.end(), [this](NODE& n1, NODE& n2)->bool {
		return Distance(n1.pos, m_npc.GetPos()) < Distance(n2.pos, m_npc.GetPos());
		});
	int nFirst = iFirst - m_graph.begin();

	// find the last node: the closest to the destination
	vector<NODE>::iterator iLast = min_element(m_graph.begin(), m_graph.end(), [v](NODE& n1, NODE& n2)->bool {
		return Distance(n1.pos, v) < Distance(n2.pos, v);
		});
	int nLast = iLast - m_graph.begin();

	// remove the current way points and reset the NPC
	if (!m_waypoints.empty())
	{
		m_waypoints.clear();
		m_npc.SetVelocity(0, 0);
	}

	// call the path finding algorithm to complete the waypoints
	vector<int> path;
	if (PathFind(m_graph, nFirst, nLast, path))
	{
		for (int i : path)
			m_waypoints.push_back(m_graph[i].pos);
		m_waypoints.push_back(v);
	}
}

void CMyGame::OnLButtonUp(Uint16 x,Uint16 y)
{
}

void CMyGame::OnRButtonDown(Uint16 x,Uint16 y)
{
}

void CMyGame::OnRButtonUp(Uint16 x,Uint16 y)
{
}

void CMyGame::OnMButtonDown(Uint16 x,Uint16 y)
{
}

void CMyGame::OnMButtonUp(Uint16 x,Uint16 y)
{
}
