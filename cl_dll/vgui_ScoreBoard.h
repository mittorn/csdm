#ifndef _SCRBRD_H
#define _SCRBRD_H
using namespace vgui;

class CScoreBoard : public Panel
{
public:
    CScoreBoard();
	void paintTeamplay();
    virtual void paint();
};

#endif