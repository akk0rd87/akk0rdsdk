#include "gpg_manager.h"
#include "basewrapper.h"

#ifdef __ANDROID__
#include "gpg/gpg.h"
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

bool GPG_Manager::Init()
{
    if(!GPG_ManagerContext.Inited)
    {
#ifdef __ANDROID__
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
                        .SetOnAuthActionFinished([](gpg::AuthOperation op,
                                                        gpg::AuthStatus status) {
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
                                    logError("ERROR_NOT_AUTHORIZED");
                                    GPG_ManagerContext.game_services_->StartAuthorizationUI();
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
                                    logError("VALID");
                                    break;

                                default:
                                    logError("Another status");
                                    break;
                            }
                        })
                        .SetOnLog([](gpg::LogLevel logLevel, const std::string & msg)
                                  {
                                      //__android_log_print(ANDROID_LOG_ERROR, "SDL", "%s", msg.c_str());
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