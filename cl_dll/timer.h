class CHudTimer: public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	int Draw(float fTime);
	int MsgFunc_StartTimer(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_KillTimer(const char *pszName, int iSize, void *pbuf);
	int m_HUD_timer;
	bool ShowTimer;
	float Time;
	float StartTime;
};