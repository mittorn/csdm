#define STATE_NONACTIVE -1
#define STATE_IDLE 0
#define STATE_CAPTURE 1
#define STATE_DEFENDING 2

class CHeadQuartersRules : public CHalfLifeTeamplay
{
public:
	CHeadQuartersRules();
	virtual void Think( void );
	virtual BOOL IsHeadQuarters( void );
private:
	void HeadQThink( void );
	void SpawnNextBase( void );
	int State;
	bool DefBaseCap;
	int LastCurrentTeam;
	int CurrentTeam;
	float LFT;
	float StartTime;
	float EndTime;
	float StartTime2;
	float EndTime2;
	CBaseEntity *CurrentBase;
	float SpawnTime;
	CBaseEntity *LastBase;
};