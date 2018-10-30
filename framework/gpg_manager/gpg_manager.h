#ifndef __AKK0RD_GOOGLE_PLAY_GAME_MANAGER_H__
#define __AKK0RD_GOOGLE_PLAY_GAME_MANAGER_H__

class GPG_Manager
{
public:
    static bool Init(bool autoLogin);

    static void StartSelection(int MinPlayers, int MaxPlayers, bool UI);

    enum struct GPG_State : unsigned char { notInited, Initialized, NotAuthorized, Authorized };


};

#endif // __AKK0RD_GOOGLE_PLAY_GAME_MANAGER_H__