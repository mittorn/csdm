// by csDev team
extern edict_t *EntSelectSpawnPoint( CBaseEntity *pPlayer );

enum
{
	START_ROUND = 0,
	END_ROUND,
	RESPAWN,
};

class CHalfLifeDestroy : public CHalfLifeTeamplay
{
public:
	CHalfLifeDestroy();
	virtual void InitHUD( CBasePlayer *pl );
	virtual void Think( void );
	virtual BOOL IsDestroy( void );
	virtual void PlayerSpawn( CBasePlayer *pPlayer );
	virtual float FlWeaponTryRespawn( CBasePlayerItem *pWeapon );

	virtual void SetNextAction( int Action, float Time );
	virtual void ActionThink();
	virtual void ActStartRound();
	virtual void ActEndRound();
	virtual void ActRespawn();

	virtual void RespawnPlayers();
	virtual void DeleteAllDecals();
	virtual void ResetAllEntity();
	virtual void SetTeamScores( int T, int CT );

private:
	int CTScores;
	int TScores;
	int NextAction;
	float TimeNextAction;
	float RoundTime;
	float StartRoundTime;
};