#include "gpg_manager.h"
#include "basewrapper.h"

#ifdef __ANDROID__
#include "gpg/gpg.h"
#include "core/android/android_wrapper.h"
#include <thread>
#include <chrono>

void GPGonActivityResultCallback(JNIEnv *env, jobject thiz, jobject activity, jint request_code, jint result_code, jobject data)
{
    logDebug("GPGonActivityResultCallback");
    gpg::AndroidSupport::OnActivityResult(env, activity, request_code, result_code, data);
};

extern "C" JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    gpg::AndroidInitialization::JNI_OnLoad(vm);
    return JNI_VERSION_1_4;
}
#endif

struct GPG_ManagerContextStruct
{
public:
    bool Inited = false;
#ifdef __ANDROID__
    std::unique_ptr<gpg::GameServices> game_services_;
#endif

    ~GPG_ManagerContextStruct()
    {
        Inited = false;
    }
};

static GPG_ManagerContextStruct GPG_ManagerContext;

void private_MyTurn(const gpg::TurnBasedMatch& Match)
{
    std::vector<uint8_t> match_data;
    match_data.push_back(100);
    match_data.push_back(200);
    match_data.push_back(100);

    gpg::ParticipantResults results = Match.ParticipantResults();
    gpg::MultiplayerParticipant nextParticipant = Match.SuggestedNextParticipant();

    if (!nextParticipant.Valid()) {//Error case
        logDebug("dismiss");
        GPG_ManagerContext.game_services_->TurnBasedMultiplayer().DismissMatch(Match);
        return;
    }

    logDebug("nextParticipant name = %s", nextParticipant.DisplayName().c_str());

    std::this_thread::sleep_for(std::chrono::milliseconds(3000));

    GPG_ManagerContext.game_services_->TurnBasedMultiplayer().TakeMyTurn(Match,
                                                                         match_data,
                                                                         results, nextParticipant,
                                                                         [](const gpg::TurnBasedMultiplayerManager::TurnBasedMatchResponse &
                                                                         response) {
                                                                             logDebug("Took turn");

                                                                             if(gpg::IsSuccess(response.status))
                                                                                 logDebug("success");
                                                                             else
                                                                                 logError("error");
                                                                         });
}

void private_CreateTurnBasedMatch(const gpg::TurnBasedMatchConfig& config)
{
    GPG_ManagerContext.game_services_->TurnBasedMultiplayer().CreateTurnBasedMatch(
            config,
            [](const gpg::TurnBasedMultiplayerManager::TurnBasedMatchResponse &matchResponse) {
                if (matchResponse.status == gpg::MultiplayerStatus::VALID) {
                    //PlayGame(matchResponse.match);
                    logDebug("Ready to play 2");

                    if(matchResponse.match.HasData())
                        logDebug("Has Data");
                    else
                        logDebug("Has no");

                    switch(matchResponse.match.Status())
                    {
                        case gpg::MatchStatus::CANCELED: logDebug("CANCELED"); break;
                        case gpg::MatchStatus::COMPLETED: logDebug("COMPLETED"); break;
                        case gpg::MatchStatus::EXPIRED: logDebug("EXPIRED"); break;
                        case gpg::MatchStatus::INVITED: logDebug("INVITED"); break;
                        case gpg::MatchStatus::MY_TURN: logDebug("MY_TURN"); private_MyTurn(matchResponse.match); break;
                        case gpg::MatchStatus::PENDING_COMPLETION: logDebug("PENDING_COMPLETION"); break;
                        case gpg::MatchStatus::THEIR_TURN: logDebug("THEIR_TURN"); break;
                        default: logDebug("Other state"); break;
                    }

                } else
                    logDebug("matchResponse.status != gpg::MultiplayerStatus::VALID");
            });
}


bool GPG_Manager::Init(bool autoLogin)
{
    if(!GPG_ManagerContext.Inited)
    {
#ifdef __ANDROID__
        AndroidWrapper::SetOnActivityResultCallback(GPGonActivityResultCallback);

        gpg::AndroidPlatformConfiguration platform_configuration;
        {
            jobject act = reinterpret_cast<jobject>(SDL_AndroidGetActivity());
            platform_configuration.SetActivity(act);

            if (platform_configuration.Valid())
                logDebug("Valid");
            else
                logDebug("not Valid");
        }

// Creates a games_services object that has lambda callbacks.
        GPG_ManagerContext.game_services_ =
                gpg::GameServices::Builder()
                        .SetDefaultOnLog(gpg::LogLevel::VERBOSE)
                        .SetOnAuthActionStarted([](gpg::AuthOperation op) {
                            logDebug("0");

                            if(op == gpg::AuthOperation::SIGN_IN)
                                logDebug("0 SIGN_IN");
                            else
                                logDebug("0 SIGN_OUT");
                        })
                        .SetOnAuthActionFinished([autoLogin]
                                                 (gpg::AuthOperation op,
                                                  gpg::AuthStatus status
                                                 )
                         {
                            logDebug("1");
                            if (op == gpg::AuthOperation::SIGN_IN) {
                                logDebug("1 SIGN_IN");
                            } else if (op == gpg::AuthOperation::SIGN_OUT) {
                                logDebug("1 SIGN_OUT");
                            } else {
                                logDebug("else ");
                            }

                            logDebug("status %d", int(status));

                            switch(status)
                            {
                                case gpg::AuthStatus::ERROR_APP_MISCONFIGURED:
                                    logError("ERROR_APP_MISCONFIGURED");
                                    break;
                                case gpg::AuthStatus::ERROR_GAME_NOT_FOUND:
                                    logError("ERROR_GAME_NOT_FOUND");
                                    break;
                                case gpg::AuthStatus::ERROR_INTERNAL:
                                    logError("ERROR_INTERNAL");
                                    break;
                                case gpg::AuthStatus::ERROR_INTERRUPTED:
                                    logError("ERROR_INTERRUPTED");
                                    break;
                                case gpg::AuthStatus::ERROR_NETWORK_OPERATION_FAILED:
                                    logError("ERROR_NETWORK_OPERATION_FAILED");
                                    break;
                                case gpg::AuthStatus::ERROR_NOT_AUTHORIZED:
                                    logWarning("ERROR_NOT_AUTHORIZED");
                                    if(autoLogin)
                                    {
                                        GPG_ManagerContext.game_services_->StartAuthorizationUI();
                                    }
                                    break;
                                case gpg::AuthStatus::ERROR_NO_DATA:
                                    logError("ERROR_NO_DATA");
                                    break;
                                case gpg::AuthStatus::ERROR_TIMEOUT:
                                    logError("ERROR_TIMEOUT");
                                    break;
                                case gpg::AuthStatus::ERROR_VERSION_UPDATE_REQUIRED:
                                    logError("ERROR_VERSION_UPDATE_REQUIRED");
                                    break;
                                case gpg::AuthStatus::VALID:
                                    logDebug("VALID");
                                    break;
                                default:
                                    logError("Another status");
                                    break;
                            }
                        })
                        /*
                        .SetOnMultiplayerInvitationEvent(
                                [](gpg::MultiplayerEvent event, std::string match_id,
                                       gpg::MultiplayerInvitation invitation) {
                                    logDebug("MultiplayerInvitationEvent callback");

                                    switch(event) {
                                        case gpg::MultiplayerEvent::REMOVED: logDebug("REMOVED"); break;
                                        case gpg::MultiplayerEvent::UPDATED: logDebug("UPDATED"); break;
                                        case gpg::MultiplayerEvent::UPDATED_FROM_APP_LAUNCH: logDebug("UPDATED_FROM_APP_LAUNCH"); break;
                                    }

                                    GPG_Manager::ShowMatchBoxUI();
                                })
                                */
                        .SetOnTurnBasedMatchEvent([] (const gpg::MultiplayerEvent& event, const std::string& str, const gpg::TurnBasedMatch& Match)
                                                  {
                                                      logDebug("SetOnTurnBasedMatchEvent callback");

                                                      if(event == gpg::MultiplayerEvent::UPDATED)
                                                      {
                                                          logDebug("Updated");
                                                          if(Match.Valid())
                                                          {
                                                              logDebug("Valid");

                                                              if(Match.Status() == gpg::MatchStatus::MY_TURN)
                                                              {
                                                                  private_MyTurn(Match);
                                                              }
                                                          }
                                                      }
                                                  }
                        )
                        .SetOnLog([](gpg::LogLevel logLevel, const std::string & msg)
                                  {
                                      // https://developers.google.com/games/services/cpp/api/namespace/gpg#namespacegpg_1a4301d118877862d8a7d23745a56c430f
                                      switch(logLevel)
                                      {
                                          case gpg::LogLevel::VERBOSE:
                                          case gpg::LogLevel::INFO:
                                              logDebug("%s", msg.c_str());
                                            break;

                                          case gpg::LogLevel::WARNING:
                                              logWarning("%s", msg.c_str());
                                              break;

                                          case gpg::LogLevel::ERROR:
                                          default:
                                              logError("%s", msg.c_str());
                                              break;
                                      }

                                  })
                        .Create(platform_configuration);


        if (!GPG_ManagerContext.game_services_)
        {
            logError("GPG: failed to create GameServices!");
        }
        else
        {
            GPG_ManagerContext.Inited = true;
        }
#endif
    }
    return true;
}

void GPG_Manager::StartSelection(int MinPlayers, int MaxPlayers, bool UI)
{
#ifdef __ANDROID__
    if (GPG_ManagerContext.game_services_)
        if(GPG_ManagerContext.game_services_->IsAuthorized())
        {
            if(UI)
            {
                GPG_ManagerContext.game_services_->TurnBasedMultiplayer().ShowPlayerSelectUI(
                        MinPlayers, MaxPlayers, true,
                        [](const gpg::TurnBasedMultiplayerManager::PlayerSelectUIResponse &
                        response) {
                            //////
                            logDebug("PlayerSelectUIResponse");
                            if (gpg::IsSuccess(response.status)) {
                                gpg::TurnBasedMatchConfig config = gpg::TurnBasedMatchConfig::Builder()
                                        .SetMinimumAutomatchingPlayers(
                                                response.minimum_automatching_players)
                                        .SetMaximumAutomatchingPlayers(
                                                response.maximum_automatching_players)
                                        .AddAllPlayersToInvite(response.player_ids).Create();

                                private_CreateTurnBasedMatch(config);
                            }
                        });
            }
            else
            {
                gpg::TurnBasedMatchConfig config = gpg::TurnBasedMatchConfig::Builder()
                        .SetMinimumAutomatchingPlayers(MinPlayers)
                        .SetMaximumAutomatchingPlayers(MaxPlayers).Create();

                private_CreateTurnBasedMatch(config);
            }
        }
#endif
}

void GPG_Manager::ShowMatchBoxUI()
{
#ifdef __ANDROID__
    if (GPG_ManagerContext.game_services_)
        if(GPG_ManagerContext.game_services_->IsAuthorized())
        {
            //showMatchInboxUI
            GPG_ManagerContext.game_services_->TurnBasedMultiplayer().ShowMatchInboxUI([](const gpg::TurnBasedMultiplayerManager::MatchInboxUIResponse& response)
                                                                                       {
                                                                                           logDebug("MatchInboxUIResponse");

                                                                                           if (gpg::IsSuccess(response.status)) {
                                                                                               switch (response.match.Status()) {
                                                                                                   case gpg::MatchStatus::THEIR_TURN:
                                                                                                       logDebug("THEIR_TURN");
                                                                                                       //Manage match with dismiss, leave and cancel options
                                                                                                       //ManageGame(response.match, true, true, false);
                                                                                                       break;
                                                                                                   case gpg::MatchStatus::MY_TURN:
                                                                                                       logDebug("MY_TURN");
                                                                                                       private_MyTurn(response.match);
                                                                                                       //Play selected game
                                                                                                       //PlayGame(response.match);
                                                                                                       break;
                                                                                                   case gpg::MatchStatus::COMPLETED:
                                                                                                       logDebug("COMPLETED");
                                                                                                       //Manage match with dismiss, rematch options
                                                                                                       //ManageGame(response.match, false, false, true);
                                                                                                       break;
                                                                                                   case gpg::MatchStatus::EXPIRED:
                                                                                                       logDebug("EXPIRED");
                                                                                                       break;
                                                                                                   default:
                                                                                                       logDebug("DEFAULT");
                                                                                                       //Manage match with dismiss option
                                                                                                       //ManageGame(response.match, false, false, false);
                                                                                                       break;
                                                                                               }
                                                                                           } else
                                                                                           {
                                                                                               logError("Invalid response status");
                                                                                           }
                                                                                       }

                    );
        }
#endif
};