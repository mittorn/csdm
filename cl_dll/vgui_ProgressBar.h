#ifndef _PRGBAR_H
#define _PRGBAR_H
using namespace vgui;

class CProgressBar : public Panel
{
public:
    CProgressBar();
    virtual void paint();

	void NewPrgBar( float flTimeLong );
	void KillPrgBar( void );
private:
	float TimeStart;
	float TimeEnd;
	float TimeLong;
};

#endif