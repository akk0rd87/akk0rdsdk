#include "gpg_manager.h"
#include "basewrapper.h"

#ifdef __ANDROID__
#include "gpg/gpg.h"
#include "core/android/android_wrapper.h"

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
                        [](gpg::TurnBasedMultiplayerManager::PlayerSelectUIResponse const &
                        response) {
                            //////
                            logDebug("PlayerSelectUIResponse");
                            if (response.status == gpg::UIStatus::VALID) {
                                gpg::TurnBasedMatchConfig config = gpg::TurnBasedMatchConfig::Builder()
                                        .SetMinimumAutomatchingPlayers(
                                                response.minimum_automatching_players)
                                        .SetMaximumAutomatchingPlayers(
                                                response.maximum_automatching_players)
                                        .AddAllPlayersToInvite(response.player_ids).Create();

                                GPG_ManagerContext.game_services_->TurnBasedMultiplayer().CreateTurnBasedMatch(
                                        config,
                                        [](gpg::TurnBasedMultiplayerManager::TurnBasedMatchResponse const &matchResponse) {
                                            if (matchResponse.status == gpg::MultiplayerStatus::VALID) {
                                                //PlayGame(matchResponse.match);
                                                logDebug("Ready to play 1");
                                            } else
                                                logDebug("matchResponse.status != gpg::MultiplayerStatus::VALID");
                                        });
                            }
                            //////
                        });
            }
            else
            {
                gpg::TurnBasedMatchConfig config = gpg::TurnBasedMatchConfig::Builder()
                        .SetMinimumAutomatchingPlayers(MinPlayers)
                        .SetMaximumAutomatchingPlayers(MaxPlayers).Create();

                GPG_ManagerContext.game_services_->TurnBasedMultiplayer().CreateTurnBasedMatch(
                        config,
                        [](gpg::TurnBasedMultiplayerManager::TurnBasedMatchResponse const &matchResponse) {
                            if (matchResponse.status == gpg::MultiplayerStatus::VALID) {
                                //PlayGame(matchResponse.match);
                                logDebug("Ready to play 2");
                            } else
                                logDebug("matchResponse.status != gpg::MultiplayerStatus::VALID");
                        });
            }
        }
#endif
}